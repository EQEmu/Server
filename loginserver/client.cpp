#include "client.h"
#include "login_server.h"
#include "../common/misc_functions.h"
#include "../common/eqemu_logsys.h"
#include "../common/strings.h"
#include "encryption.h"
#include "account_management.h"

extern LoginServer server;

Client::Client(std::shared_ptr<EQStreamInterface> c, LSClientVersion v)
{
	m_connection              = c;
	m_client_version          = v;
	m_client_status           = cs_not_sent_session_ready;
	m_account_id              = 0;
	m_selected_play_server_id = 0;
	m_play_sequence_id        = 0;
}

bool Client::Process()
{
	EQApplicationPacket *app = m_connection->PopPacket();
	while (app) {
		auto o = m_connection->GetOpcodeManager();
		LogPacketClientServer(
			"[{}] [{:#06x}] Size [{}] {}",
			OpcodeManager::EmuToName(app->GetOpcode()),
			o->EmuToEQ(app->GetOpcode()) == 0 ? app->GetProtocolOpcode() : o->EmuToEQ(app->GetOpcode()),
			app->Size(),
			(LogSys.IsLogEnabled(Logs::Detail, Logs::PacketClientServer) ? DumpPacketToString(app) : "")
		);

		if (m_client_status == cs_failed_to_login) {
			delete app;
			app = m_connection->PopPacket();
			continue;
		}

		switch (app->GetOpcode()) {
			case OP_SessionReady: {
				LogInfo("Session ready received from client account {}", GetClientLoggingDescription());
				HandleSessionReady((const char *) app->pBuffer, app->Size());
				break;
			}
			case OP_Login: {
				if (app->Size() < 20) {
					LogError("Login received but it is too small, discarding");
					break;
				}

				LogInfo("Login received from client {}", GetClientLoggingDescription());

				HandleLogin((const char *) app->pBuffer, app->Size());
				break;
			}
			case OP_ServerListRequest: {
				if (app->Size() < 4) {
					LogError("Server List Request received but it is too small, discarding");
					break;
				}

				LogInfo("Server list request received from client {}", GetClientLoggingDescription());

				SendServerListPacket(*(uint32_t *) app->pBuffer);
				break;
			}
			case OP_PlayEverquestRequest: {
				if (app->Size() < sizeof(PlayEverquestRequest)) {
					LogError("Play received but it is too small, discarding");
					break;
				}

				SendPlayToWorld((const char *) app->pBuffer);
				break;
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
void Client::HandleSessionReady(const char *data, unsigned int size)
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
	auto *outapp = new EQApplicationPacket(OP_ChatMessage, sizeof(LoginHandShakeReply));
	auto buf     = reinterpret_cast<LoginHandShakeReply *>(outapp->pBuffer);
	buf->base_header.sequence    = 0x02;
	buf->base_reply.success      = true;
	buf->base_reply.error_str_id = 0x65; // 101 "No Error"

	m_connection->QueuePacket(outapp);
	delete outapp;
}

/**
 * Verifies login and send a reply
 *
 * @param data
 * @param size
 */
void Client::HandleLogin(const char *data, unsigned int size)
{
	if (m_client_status != cs_waiting_for_login) {
		LogError("Login received after already having logged in");
		return;
	}

	// login user/pass are variable length after unencrypted opcode and base message header (size includes opcode)
	constexpr int header_size = sizeof(uint16_t) + sizeof(LoginBaseMessage);
	int           data_size   = size - header_size;

	if (size <= header_size) {
		LogError("Login received packet of size: {}, this would cause a buffer overflow, discarding", size);

		return;
	}

	if (data_size % 8 != 0) {
		LogError("Login received packet of size: {}, this would cause a block corruption, discarding", size);

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
	auto r = eqcrypt_block(data + sizeof(LoginBaseMessage), data_size, &outbuffer[0], 0);
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
	memcpy(&m_login_base_message, data, sizeof(LoginBaseMessage));

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
			auto components = Strings::Split(user, ':');
			if (components.size() == 2) {
				db_loginserver = components[0];
				user           = components[1];
			}

			// health checks
			if (ProcessHealthCheck(user)) {
				SendFailedLogin();
				return;
			}

			LogInfo(
				"Attempting password based login [{}] login [{}]",
				user,
				db_loginserver
			);

			ParseAccountString(user, user, db_loginserver);

			if (server.db->GetLoginDataFromAccountInfo(user, db_loginserver, db_account_password_hash, db_account_id)) {
				result = VerifyAndUpdateLoginHash(user, db_loginserver, cred, db_account_password_hash);

#ifdef LSPX
				// if user updated their password on the login server, update it here by validating their credentials with the login server
				if (!result && db_loginserver == "eqemu") {
					uint32 account_id = AccountManagement::CheckExternalLoginserverUserCredentials(user, cred);
					if (account_id > 0) {
						auto encryption_mode = server.options.GetEncryptionMode();
						server.db->UpdateLoginserverAccountPasswordHash(
							user,
							db_loginserver,
							eqcrypt_hash(user, cred, encryption_mode)
						);
						LogInfo("Updating eqemu account [{}] password hash", account_id);
						result = true;
					}
				}
#endif

				LogDebug("Success [{}]", (result ? "true" : "false"));
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
			"login [{}] user [{}] Login succeeded",
			db_loginserver,
			user
		);

		DoSuccessfulLogin(user, db_account_id, db_loginserver);
	}
	else {
		LogInfo(
			"login [{}] user [{}] Login failed",
			db_loginserver,
			user
		);

		SendFailedLogin();
	}
}

void Client::SendPlayToWorld(const char *data)
{
	if (m_client_status != cs_logged_in) {
		LogError("Client sent a play request when they were not logged in, discarding");
		return;
	}

	const auto *play        = (const PlayEverquestRequest *) data;
	auto       server_id_in = (unsigned int) play->server_number;
	auto       sequence_in  = (unsigned int) play->base_header.sequence;

	LogInfo(
		"[SendPlayToWorld] Play received from client [{}] server number [{}] sequence [{}]",
		GetAccountName(),
		server_id_in,
		sequence_in
	);

	m_selected_play_server_id = (unsigned int) play->server_number;
	m_play_sequence_id        = sequence_in;
	m_selected_play_server_id = server_id_in;
	server.server_manager->SendUserLoginToWorldRequest(server_id_in, m_account_id, m_loginserver_name);
}

void Client::SendServerListPacket(uint32 seq)
{
	auto app = server.server_manager->CreateServerListPacket(this, seq);

	m_connection->QueuePacket(app.get());
}

void Client::SendPlayResponse(EQApplicationPacket *outapp)
{
	LogInfo("Sending play response for {}", GetClientLoggingDescription());
	m_connection->QueuePacket(outapp);
}

void Client::GenerateRandomLoginKey()
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

void Client::AttemptLoginAccountCreation(
	const std::string &user,
	const std::string &pass,
	const std::string &loginserver
)
{
	LogInfo("user [{}] loginserver [{}]", user, loginserver);

#ifdef LSPX
	if (loginserver == "eqemu") {
		LogInfo("Attempting login account creation via '{}'", loginserver);

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
			LogInfo("Found and creating eqemu account [{}]", account_id);
			CreateEQEmuAccount(user, pass, account_id);
			return;
		}

		DoFailedLogin();
		return;
	}
#endif

	if (server.options.CanAutoCreateAccounts() && loginserver == "local") {
		LogInfo("CanAutoCreateAccounts enabled, attempting to creating account [{}]", user);
		CreateLocalAccount(user, pass);
		return;
	}

	SendFailedLogin();
}

void Client::SendFailedLogin()
{
	m_stored_username.clear();
	m_stored_password.clear();

	// unencrypted
	LoginBaseMessage base_header{};
	base_header.sequence     = m_login_base_message.sequence; // login (3)
	base_header.encrypt_type = m_login_base_message.encrypt_type;

	// encrypted
	PlayerLoginReply login_reply{};
	login_reply.base_reply.success      = false;
	login_reply.base_reply.error_str_id = 105; // Error - The username and/or password were not valid

	char encrypted_buffer[80] = {0};
	auto rc                   = eqcrypt_block((const char *) &login_reply, sizeof(login_reply), encrypted_buffer, 1);
	if (rc == nullptr) {
		LogDebug("Failed to encrypt eqcrypt block for failed login");
	}

	constexpr int       outsize = sizeof(LoginBaseMessage) + sizeof(encrypted_buffer);
	EQApplicationPacket outapp(OP_LoginAccepted, outsize);
	outapp.WriteData(&base_header, sizeof(base_header));
	outapp.WriteData(&encrypted_buffer, sizeof(encrypted_buffer));

	m_connection->QueuePacket(&outapp);
	m_client_status = cs_failed_to_login;
}

bool Client::VerifyAndUpdateLoginHash(
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

void Client::DoSuccessfulLogin(
	const std::string &in_account_name,
	int db_account_id,
	const std::string &db_loginserver
)
{
	m_stored_username.clear();
	m_stored_password.clear();

	server.client_manager->RemoveExistingClient(db_account_id, db_loginserver);

	in_addr in{};
	in.s_addr = m_connection->GetRemoteIP();

	server.db->UpdateLSAccountData(db_account_id, std::string(inet_ntoa(in)));
	GenerateRandomLoginKey();

	m_account_id       = db_account_id;
	m_account_name     = in_account_name;
	m_loginserver_name = db_loginserver;

	// unencrypted
	LoginBaseMessage base_header{};
	base_header.sequence     = m_login_base_message.sequence;
	base_header.compressed   = false;
	base_header.encrypt_type = m_login_base_message.encrypt_type;
	base_header.unk3         = m_login_base_message.unk3;

	// not serializing any of the variable length strings so just use struct directly
	PlayerLoginReply login_reply{};
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

	SendExpansionPacketData(login_reply);

	char encrypted_buffer[80] = {0};
	auto rc                   = eqcrypt_block((const char *) &login_reply, sizeof(login_reply), encrypted_buffer, 1);
	if (rc == nullptr) {
		LogDebug("Failed to encrypt eqcrypt block");
	}

	constexpr int outsize = sizeof(LoginBaseMessage) + sizeof(encrypted_buffer);
	auto          outapp  = std::make_unique<EQApplicationPacket>(OP_LoginAccepted, outsize);
	outapp->WriteData(&base_header, sizeof(base_header));
	outapp->WriteData(&encrypted_buffer, sizeof(encrypted_buffer));

	m_connection->QueuePacket(outapp.get());

	m_client_status = cs_logged_in;
}

void Client::SendExpansionPacketData(PlayerLoginReply &plrs)
{
	SerializeBuffer buf;
	//from eqlsstr_us.txt id of each expansion, excluding 'Everquest'
	int             ExpansionLookup[20] = {
		3007, 3008, 3009, 3010, 3012,
		3014, 3031, 3033, 3036, 3040,
		3045, 3046, 3047, 3514, 3516,
		3518, 3520, 3522, 3524
	};


	if (server.options.IsDisplayExpansions()) {

		int32_t expansion       = server.options.GetMaxExpansions();
		int32_t owned_expansion = (expansion << 1) | 1;

		if (m_client_version == cv_sod) {

			// header info of packet.  Requires OP_LoginExpansionPacketData=0x0031 to be in login_opcodes_sod.conf
			buf.WriteInt32(0x00);
			buf.WriteInt32(0x01);
			buf.WriteInt16(0x00);
			buf.WriteInt32(19); //number of expansions to include in packet

			//generate expansion data
			for (int i = 0; i < 19; i++) {
				buf.WriteInt32(i);                                                    //sequenctial number
				buf.WriteInt32((expansion & (1 << i)) == (1 << i) ? 0x01 : 0x00);    //1 own 0 not own
				buf.WriteInt8(0x00);
				buf.WriteInt32(ExpansionLookup[i]);                                    //from eqlsstr_us.txt
				buf.WriteInt32(0x179E);                                                //from eqlsstr_us.txt for buttons/order
				buf.WriteInt32(0xFFFFFFFF);                                            //end identification
				buf.WriteInt8(0x0);                                                    //force order window to appear 1 appear 0 not appear
				buf.WriteInt8(0x0);
				buf.WriteInt32(0x0000);
				buf.WriteInt32(0x0000);
				buf.WriteInt32(0xFFFFFFFF);
			}

			auto out = std::make_unique<EQApplicationPacket>(OP_LoginExpansionPacketData, buf);
			m_connection->QueuePacket(out.get());

		}
		else if (m_client_version == cv_titanium) {
			if (expansion >= EQ::expansions::bitPoR) {
				// Titanium shipped with 10 expansions.  Set owned expansions to be max 10.
				plrs.offer_min_days = ((EQ::expansions::bitDoD << 2) | 1) - 2;
			}
			else {
				plrs.offer_min_days = owned_expansion;
			}
			// Titanium shipped with 10 expansions.  Set owned expansions to be max 10.
			plrs.web_offer_min_views = ((EQ::expansions::bitDoD << 2) | 1) - 2;
		}

	}

}

void Client::CreateLocalAccount(const std::string &username, const std::string &password)
{
	auto         mode  = server.options.GetEncryptionMode();
	auto         hash  = eqcrypt_hash(username, password, mode);
	unsigned int db_id = 0;
	if (!server.db->CreateLoginData(username, hash, "local", db_id)) {
		SendFailedLogin();
	}
	else {
		DoSuccessfulLogin(username, db_id, "local");
	}
}

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
		SendFailedLogin();
	}
	else {
		DoSuccessfulLogin(in_account_name, loginserver_account_id, "eqemu");
	}
}

bool Client::ProcessHealthCheck(std::string username)
{
	if (username == "healthcheckuser") {
		return true;
	}

	return false;
}

std::string Client::GetClientLoggingDescription()
{
	in_addr in{};
	in.s_addr = GetConnection()->GetRemoteIP();
	std::string client_ip = inet_ntoa(in);

	return fmt::format(
		"account_name [{}] account_id ({}) ip_address [{}]",
		GetAccountName(),
		GetAccountID(),
		client_ip
	);
}
