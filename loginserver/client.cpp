#include "client.h"
#include "login_server.h"
#include "../common/misc_functions.h"
#include "../common/eqemu_logsys.h"
#include "../common/string_util.h"
#include "encryption.h"
#include "account_management.h"

extern LoginServer server;

/**
 * @param c
 * @param v
 */
Client::Client(std::shared_ptr<EQStreamInterface> c, LSClientVersion v)
{
	m_connection       = c;
	m_client_version   = v;
	m_client_status    = cs_not_sent_session_ready;
	m_account_id       = 0;
	m_play_server_id   = 0;
	m_play_sequence_id = 0;
}

bool Client::Process()
{
	EQApplicationPacket *app = m_connection->PopPacket();
	while (app) {
		if (server.options.IsTraceOn()) {
			LogDebug("Application packet received from client (size {0})", app->Size());
		}

		if (server.options.IsDumpInPacketsOn()) {
			DumpPacket(app);
		}

		if (m_client_status == cs_failed_to_login) {
			delete app;
			app = m_connection->PopPacket();
			continue;
		}

		switch (app->GetOpcode()) {
			case OP_SessionReady: {
				if (server.options.IsTraceOn()) {
					LogInfo("Session ready received from client");
				}
				Handle_SessionReady((const char *) app->pBuffer, app->Size());
				break;
			}
			case OP_Login: {
				if (app->Size() < 20) {
					LogError("Login received but it is too small, discarding");
					break;
				}

				if (server.options.IsTraceOn()) {
					LogInfo("Login received from client");
				}

				Handle_Login((const char *) app->pBuffer, app->Size());
				break;
			}
			case OP_ServerListRequest: {
				if (app->Size() < 4) {
					LogError("Server List Request received but it is too small, discarding");
					break;
				}

				if (server.options.IsTraceOn()) {
					LogDebug("Server list request received from client");
				}

				SendServerListPacket(*(uint32_t *) app->pBuffer);
				break;
			}
			case OP_PlayEverquestRequest: {
				if (app->Size() < sizeof(PlayEverquestRequest_Struct)) {
					LogError("Play received but it is too small, discarding");
					break;
				}

				Handle_Play((const char *) app->pBuffer);
				break;
			}
			default: {
				if (LogSys.log_settings[Logs::PacketClientServerUnhandled].is_category_enabled == 1) {
					char dump[64];
					app->build_header_dump(dump);
					LogError("Recieved unhandled application packet from the client: [{}]", dump);
				}
			}
		}

		delete app;
		app = m_connection->PopPacket();
	}

	return true;
}

/**
 * Sends our reply to session ready packet
 *
 * @param data
 * @param size
 */
void Client::Handle_SessionReady(const char *data, unsigned int size)
{
	if (m_client_status != cs_not_sent_session_ready) {
		LogError("Session ready received again after already being received");
		return;
	}

	if (size < sizeof(unsigned int)) {
		LogError("Session ready was too small");
		return;
	}

	m_client_status = cs_waiting_for_login;

	/**
	 * The packets are identical between the two versions
	 */
	auto *outapp = new EQApplicationPacket(OP_ChatMessage, sizeof(LoginHandShakeReply_Struct));
	auto buf = reinterpret_cast<LoginHandShakeReply_Struct*>(outapp->pBuffer);
	buf->base_header.sequence    = 0x02;
	buf->base_reply.success      = true;
	buf->base_reply.error_str_id = 0x65; // 101 "No Error"

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(outapp);
	}

	m_connection->QueuePacket(outapp);
	delete outapp;
}

/**
 * Verifies login and send a reply
 *
 * @param data
 * @param size
 */
void Client::Handle_Login(const char *data, unsigned int size)
{
	if (m_client_status != cs_waiting_for_login) {
		LogError("Login received after already having logged in");
		return;
	}

	// login user/pass are variable length after unencrypted opcode and base message header (size includes opcode)
	constexpr int header_size = sizeof(uint16_t) + sizeof(LoginBaseMessage_Struct);
	int data_size = size - header_size;

	if (size <= header_size) {
		LogError("Login received packet of size: {0}, this would cause a buffer overflow, discarding", size);

		return;
	}

	if (data_size % 8 != 0) {
		LogError("Login received packet of size: {0}, this would cause a block corruption, discarding", size);

		return;
	}

	char *login_packet_buffer = nullptr;

	unsigned int db_account_id = 0;

	std::string db_loginserver = "local";
	if (server.options.CanAutoLinkAccounts()) {
		db_loginserver = "eqemu";
	}

	std::string db_account_password_hash;

	std::string outbuffer;
	outbuffer.resize(data_size);
	if (outbuffer.length() == 0) {
		LogError("Corrupt buffer sent to server, no length");
		return;
	}

	// data starts at base message header (opcode not included)
	auto r = eqcrypt_block(data + sizeof(LoginBaseMessage_Struct), data_size, &outbuffer[0], 0);
	if (r == nullptr) {
		LogError("Failed to decrypt eqcrypt block");
		return;
	}

	std::string cred;

	std::string user(&outbuffer[0]);
	if (user.length() >= outbuffer.length()) {
		LogError("Corrupt buffer sent to server, preventing buffer overflow");
		return;
	}

	// only need to copy the base header for reply options, ignore login info
	memcpy(&m_llrs, data, sizeof(LoginBaseMessage_Struct));

	bool result = false;
	if (outbuffer[0] == 0 && outbuffer[1] == 0) {
		if (server.options.IsTokenLoginAllowed()) {
			cred   = (&outbuffer[2 + user.length()]);
			result = server.db->GetLoginTokenDataFromToken(
				cred,
				m_connection->GetRemoteAddr(),
				db_account_id,
				db_loginserver,
				user
			);
		}
	}
	else {
		if (server.options.IsPasswordLoginAllowed()) {
			cred            = (&outbuffer[1 + user.length()]);
			auto components = SplitString(user, ':');
			if (components.size() == 2) {
				db_loginserver = components[0];
				user           = components[1];
			}

			// health checks
			if (ProcessHealthCheck(user)) {
				DoFailedLogin();
				return;
			}

			LogInfo(
				"Attempting password based login [{0}] login [{1}]",
				user,
				db_loginserver
			);

			ParseAccountString(user, user, db_loginserver);

			if (server.db->GetLoginDataFromAccountInfo(user, db_loginserver, db_account_password_hash, db_account_id)) {
				result = VerifyLoginHash(user, db_loginserver, cred, db_account_password_hash);

				LogDebug("[VerifyLoginHash] Success [{0}]", (result ? "true" : "false"));
			}
			else {
				m_client_status = cs_creating_account;
				AttemptLoginAccountCreation(user, cred, db_loginserver);

				return;
			}
		}
	}

	/**
	 * Login accepted
	 */
	if (result) {
		LogInfo(
			"login [{0}] user [{1}] Login succeeded",
			db_loginserver,
			user
		);

		DoSuccessfulLogin(user, db_account_id, db_loginserver);
	}
	else {
		LogInfo(
			"login [{0}] user [{1}] Login failed",
			db_loginserver,
			user
		);

		DoFailedLogin();
	}
}

/**
 * Sends a packet to the requested server to see if the client is allowed or not
 *
 * @param data
 */
void Client::Handle_Play(const char *data)
{
	if (m_client_status != cs_logged_in) {
		LogError("Client sent a play request when they were not logged in, discarding");
		return;
	}

	const auto *play        = (const PlayEverquestRequest_Struct *) data;
	auto       server_id_in = (unsigned int) play->server_number;
	auto       sequence_in  = (unsigned int) play->base_header.sequence;

	if (server.options.IsTraceOn()) {
		LogInfo(
			"Play received from client [{0}] server number {1} sequence {2}",
			GetAccountName(),
			server_id_in,
			sequence_in
		);
	}

	m_play_server_id   = (unsigned int) play->server_number;
	m_play_sequence_id = sequence_in;
	m_play_server_id   = server_id_in;
	server.server_manager->SendUserToWorldRequest(server_id_in, m_account_id, m_loginserver_name);
}

/**
 * @param seq
 */
void Client::SendServerListPacket(uint32 seq)
{
	auto outapp = server.server_manager->CreateServerListPacket(this, seq);

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(outapp.get());
	}

	m_connection->QueuePacket(outapp.get());
}

void Client::SendPlayResponse(EQApplicationPacket *outapp)
{
	if (server.options.IsTraceOn()) {
		LogDebug("Sending play response for {0}", GetAccountName());
		// server_log->LogPacket(log_network_trace, (const char*)outapp->pBuffer, outapp->size);
	}
	m_connection->QueuePacket(outapp);
}

void Client::GenerateKey()
{
	m_key.clear();
	int count = 0;
	while (count < 10) {
		static const char key_selection[] =
							  {
								  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								  'Y', 'Z', '0', '1', '2', '3', '4', '5',
								  '6', '7', '8', '9'
							  };

		m_key.append((const char *) &key_selection[m_random.Int(0, 35)], 1);
		count++;
	}
}

/**
 * @param user
 * @param pass
 * @param loginserver
 */
void Client::AttemptLoginAccountCreation(
	const std::string &user,
	const std::string &pass,
	const std::string &loginserver
)
{
	LogInfo("[AttemptLoginAccountCreation] user [{}] loginserver [{}]", user, loginserver);

#ifdef LSPX
	if (loginserver == "eqemu") {
		LogInfo("Attempting login account creation via '{0}'", loginserver);

		if (!server.options.CanAutoLinkAccounts()) {
			LogInfo("CanAutoLinkAccounts disabled - sending failed login");
			DoFailedLogin();
			return;
		}


		uint32 account_id = AccountManagement::CheckExternalLoginserverUserCredentials(
			user,
			pass
		);

		if (account_id > 0) {
			LogInfo("[AttemptLoginAccountCreation] Found and creating eqemu account [{}]", account_id);
			CreateEQEmuAccount(user, pass, account_id);
			return;
		}

		DoFailedLogin();
		return;
	}
#endif

	if (server.options.CanAutoCreateAccounts() && loginserver == "local") {
		LogInfo("CanAutoCreateAccounts enabled, attempting to creating account [{0}]", user);
		CreateLocalAccount(user, pass);
		return;
	}

	DoFailedLogin();
}

void Client::DoFailedLogin()
{
	m_stored_user.clear();
	m_stored_pass.clear();

	// unencrypted
	LoginBaseMessage_Struct base_header{};
	base_header.sequence     = m_llrs.sequence; // login (3)
	base_header.encrypt_type = m_llrs.encrypt_type;

	// encrypted
	PlayerLoginReply_Struct login_reply{};
	login_reply.base_reply.success      = false;
	login_reply.base_reply.error_str_id = 105; // Error - The username and/or password were not valid

	char encrypted_buffer[80] = {0};
	auto rc = eqcrypt_block((const char*)&login_reply, sizeof(login_reply), encrypted_buffer, 1);
	if (rc == nullptr) {
		LogDebug("Failed to encrypt eqcrypt block for failed login");
	}

	constexpr int outsize = sizeof(LoginBaseMessage_Struct) + sizeof(encrypted_buffer);
	EQApplicationPacket outapp(OP_LoginAccepted, outsize);
	outapp.WriteData(&base_header, sizeof(base_header));
	outapp.WriteData(&encrypted_buffer, sizeof(encrypted_buffer));

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(&outapp);
	}

	m_connection->QueuePacket(&outapp);
	m_client_status = cs_failed_to_login;
}

/**
 * Verifies a login hash, will also attempt to update a login hash if needed
 *
 * @param account_username
 * @param source_loginserver
 * @param account_password
 * @param password_hash
 * @return
 */
bool Client::VerifyLoginHash(
	const std::string &account_username,
	const std::string &source_loginserver,
	const std::string &account_password,
	const std::string &password_hash
)
{
	auto encryption_mode = server.options.GetEncryptionMode();
	if (eqcrypt_verify_hash(account_username, account_password, password_hash, encryption_mode)) {
		return true;
	}
	else {
		if (server.options.IsUpdatingInsecurePasswords()) {
			if (encryption_mode < EncryptionModeArgon2) {
				encryption_mode = EncryptionModeArgon2;
			}

			uint32 insecure_source_encryption_mode = 0;
			if (password_hash.length() == CryptoHash::md5_hash_length) {
				for (int i = EncryptionModeMD5; i <= EncryptionModeMD5Triple; ++i) {
					if (i != encryption_mode &&
						eqcrypt_verify_hash(account_username, account_password, password_hash, i)) {
						insecure_source_encryption_mode = i;
					}
				}
			}
			else if (password_hash.length() == CryptoHash::sha1_hash_length && insecure_source_encryption_mode == 0) {
				for (int i = EncryptionModeSHA; i <= EncryptionModeSHATriple; ++i) {
					if (i != encryption_mode &&
						eqcrypt_verify_hash(account_username, account_password, password_hash, i)) {
						insecure_source_encryption_mode = i;
					}
				}
			}
			else if (password_hash.length() == CryptoHash::sha512_hash_length && insecure_source_encryption_mode == 0) {
				for (int i = EncryptionModeSHA512; i <= EncryptionModeSHA512Triple; ++i) {
					if (i != encryption_mode &&
						eqcrypt_verify_hash(account_username, account_password, password_hash, i)) {
						insecure_source_encryption_mode = i;
					}
				}
			}

			if (insecure_source_encryption_mode > 0) {
				LogInfo(
					"[{}] Updated insecure password user [{}] loginserver [{}] from mode [{}] ({}) to mode [{}] ({})",
					__func__,
					account_username,
					source_loginserver,
					GetEncryptionByModeId(insecure_source_encryption_mode),
					insecure_source_encryption_mode,
					GetEncryptionByModeId(encryption_mode),
					encryption_mode
				);

				server.db->UpdateLoginserverAccountPasswordHash(
					account_username,
					source_loginserver,
					eqcrypt_hash(
						account_username,
						account_password,
						encryption_mode
					)
				);

				return true;
			}
		}
	}

	return false;
}

/**
 * @param in_account_name
 * @param db_account_id
 * @param db_loginserver
 */
void Client::DoSuccessfulLogin(
	const std::string in_account_name,
	int db_account_id,
	const std::string &db_loginserver
)
{
	m_stored_user.clear();
	m_stored_pass.clear();

	server.client_manager->RemoveExistingClient(db_account_id, db_loginserver);

	in_addr in{};
	in.s_addr = m_connection->GetRemoteIP();

	server.db->UpdateLSAccountData(db_account_id, std::string(inet_ntoa(in)));
	GenerateKey();

	m_account_id       = db_account_id;
	m_account_name     = in_account_name;
	m_loginserver_name = db_loginserver;

	// unencrypted
	LoginBaseMessage_Struct base_header{};
	base_header.sequence     = m_llrs.sequence;
	base_header.compressed   = false;
	base_header.encrypt_type = m_llrs.encrypt_type;
	base_header.unk3         = m_llrs.unk3;

	// not serializing any of the variable length strings so just use struct directly
	PlayerLoginReply_Struct login_reply{};
	login_reply.base_reply.success         = true;
	login_reply.base_reply.error_str_id    = 101; // No Error
	login_reply.unk1                       = 0;
	login_reply.unk2                       = 0;
	login_reply.lsid                       = db_account_id;
	login_reply.failed_attempts            = 0;
	login_reply.show_player_count          = server.options.IsShowPlayerCountEnabled();
	login_reply.offer_min_days             = 99;
	login_reply.offer_min_views            = -1;
	login_reply.offer_cooldown_minutes     = 0;
	login_reply.web_offer_number           = 0;
	login_reply.web_offer_min_days         = 99;
	login_reply.web_offer_min_views        = -1;
	login_reply.web_offer_cooldown_minutes = 0;
	memcpy(login_reply.key, m_key.c_str(), m_key.size());

	char encrypted_buffer[80] = {0};
	auto rc = eqcrypt_block((const char*)&login_reply, sizeof(login_reply), encrypted_buffer, 1);
	if (rc == nullptr) {
		LogDebug("Failed to encrypt eqcrypt block");
	}

	constexpr int outsize = sizeof(LoginBaseMessage_Struct) + sizeof(encrypted_buffer);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_LoginAccepted, outsize);
	outapp->WriteData(&base_header, sizeof(base_header));
	outapp->WriteData(&encrypted_buffer, sizeof(encrypted_buffer));

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(outapp.get());
	}

	m_connection->QueuePacket(outapp.get());

	m_client_status = cs_logged_in;
}

/**
 * @param username
 * @param password
 */
void Client::CreateLocalAccount(const std::string &username, const std::string &password)
{
	auto         mode  = server.options.GetEncryptionMode();
	auto         hash  = eqcrypt_hash(username, password, mode);
	unsigned int db_id = 0;
	if (!server.db->CreateLoginData(username, hash, "local", db_id)) {
		DoFailedLogin();
	}
	else {
		DoSuccessfulLogin(username, db_id, "local");
	}
}

/**
 * @param in_account_name
 * @param in_account_password
 * @param loginserver_account_id
 */
void Client::CreateEQEmuAccount(
	const std::string &in_account_name,
	const std::string &in_account_password,
	unsigned int loginserver_account_id
)
{
	auto mode = server.options.GetEncryptionMode();
	auto hash = eqcrypt_hash(in_account_name, in_account_password, mode);

	if (server.db->DoesLoginServerAccountExist(in_account_name, hash, "eqemu", loginserver_account_id)) {
		DoSuccessfulLogin(in_account_name, loginserver_account_id, "eqemu");
		return;
	}

	if (!server.db->CreateLoginDataWithID(in_account_name, hash, "eqemu", loginserver_account_id)) {
		DoFailedLogin();
	}
	else {
		DoSuccessfulLogin(in_account_name, loginserver_account_id, "eqemu");
	}
}

/**
 * @param connection
 */
void Client::LoginOnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection)
{
	m_login_connection = connection;
}

/**
 * @param conn
 * @param from
 * @param to
 */
void Client::LoginOnStatusChange(
	std::shared_ptr<EQ::Net::DaybreakConnection> conn,
	EQ::Net::DbProtocolStatus from,
	EQ::Net::DbProtocolStatus to
)
{
	if (to == EQ::Net::StatusConnected) {
		LogDebug("EQ::Net::StatusConnected");
		LoginSendSessionReady();
	}

	if (to == EQ::Net::StatusDisconnecting || to == EQ::Net::StatusDisconnected) {
		LogDebug("EQ::Net::StatusDisconnecting || EQ::Net::StatusDisconnected");

		DoFailedLogin();
	}
}

/**
 * @param conn
 * @param from
 * @param to
 */
void Client::LoginOnStatusChangeIgnored(
	std::shared_ptr<EQ::Net::DaybreakConnection> conn,
	EQ::Net::DbProtocolStatus from,
	EQ::Net::DbProtocolStatus to
)
{
}

/**
 * @param conn
 * @param p
 */
void Client::LoginOnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet &p)
{
	auto opcode = p.GetUInt16(0);
	switch (opcode) {
		case 0x0017: //OP_ChatMessage
			LoginSendLogin();
			break;
		case 0x0018:
			LoginProcessLoginResponse(p);
			break;
	}
}

void Client::LoginSendSessionReady()
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 1); //OP_SessionReady
	p.PutUInt32(2, 2);

	m_login_connection->QueuePacket(p);
}

void Client::LoginSendLogin()
{
	size_t                  buffer_len = m_stored_user.length() + m_stored_pass.length() + 2;
	std::unique_ptr<char[]> buffer(new char[buffer_len]);

	strcpy(&buffer[0], m_stored_user.c_str());
	strcpy(&buffer[m_stored_user.length() + 1], m_stored_pass.c_str());

	size_t encrypted_len = buffer_len;

	if (encrypted_len % 8 > 0) {
		encrypted_len = ((encrypted_len / 8) + 1) * 8;
	}

	EQ::Net::DynamicPacket p;
	p.Resize(12 + encrypted_len);
	p.PutUInt16(0, 2); //OP_Login
	p.PutUInt32(2, 3);

	eqcrypt_block(&buffer[0], buffer_len, (char *) p.Data() + 12, true);

	m_login_connection->QueuePacket(p);
}

/**
 * @param p
 */
void Client::LoginProcessLoginResponse(const EQ::Net::Packet &p)
{
	auto encrypt_size                    = p.Length() - 12;

	if (encrypt_size % 8 > 0) {
		encrypt_size = (encrypt_size / 8) * 8;
	}

	std::unique_ptr<char[]> decrypted(new char[encrypt_size]);

	eqcrypt_block((char *) p.Data() + 12, encrypt_size, &decrypted[0], false);

	EQ::Net::StaticPacket sp(&decrypted[0], encrypt_size);
	auto                  response_error = sp.GetUInt16(1);

	m_login_connection_manager->OnConnectionStateChange(
		std::bind(
			&Client::LoginOnStatusChangeIgnored,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3
		)
	);

	if (response_error > 101) {
		LogDebug("response [{0}] failed login", response_error);
		DoFailedLogin();
		m_login_connection->Close();
	}
	else {
		LogDebug(
			"response [{0}] login succeeded user [{1}]",
			response_error,
			m_stored_user
		);

		auto m_dbid = sp.GetUInt32(8);

		CreateEQEmuAccount(m_stored_user, m_stored_pass, m_dbid);
		m_login_connection->Close();
	}
}
bool Client::ProcessHealthCheck(std::string username)
{
	if (username == "healthcheckuser") {
		return true;
	}

	return false;
}
