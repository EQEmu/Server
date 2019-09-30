/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "client.h"
#include "login_server.h"
#include "../common/misc_functions.h"
#include "../common/eqemu_logsys.h"
#include "../common/string_util.h"
#include "encryption.h"

extern LoginServer server;

/**
 * @param c
 * @param v
 */
Client::Client(std::shared_ptr<EQStreamInterface> c, LSClientVersion v)
{
	connection       = c;
	version          = v;
	status           = cs_not_sent_session_ready;
	account_id       = 0;
	play_server_id   = 0;
	play_sequence_id = 0;
}

bool Client::Process()
{
	EQApplicationPacket *app = connection->PopPacket();
	while (app) {
		if (server.options.IsTraceOn()) {
			LogDebug("Application packet received from client (size {0})", app->Size());
		}

		if (server.options.IsDumpInPacketsOn()) {
			DumpPacket(app);
		}

		if (status == cs_failed_to_login) {
			delete app;
			app = connection->PopPacket();
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
		app = connection->PopPacket();
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
	if (status != cs_not_sent_session_ready) {
		LogError("Session ready received again after already being received");
		return;
	}

	if (size < sizeof(unsigned int)) {
		LogError("Session ready was too small");
		return;
	}

	status = cs_waiting_for_login;

	/**
	 * The packets are mostly the same but slightly different between the two versions
	 */
	if (version == cv_sod) {
		auto *outapp = new EQApplicationPacket(OP_ChatMessage, 17);
		outapp->pBuffer[0]  = 0x02;
		outapp->pBuffer[10] = 0x01;
		outapp->pBuffer[11] = 0x65;

		if (server.options.IsDumpOutPacketsOn()) {
			DumpPacket(outapp);
		}

		connection->QueuePacket(outapp);
		delete outapp;
	}
	else {
		const char *msg    = "ChatMessage";
		auto       *outapp = new EQApplicationPacket(OP_ChatMessage, 16 + strlen(msg));
		outapp->pBuffer[0]  = 0x02;
		outapp->pBuffer[10] = 0x01;
		outapp->pBuffer[11] = 0x65;
		strcpy((char *) (outapp->pBuffer + 15), msg);

		if (server.options.IsDumpOutPacketsOn()) {
			DumpPacket(outapp);
		}

		connection->QueuePacket(outapp);
		delete outapp;
	}
}

/**
 * Verifies login and send a reply
 *
 * @param data
 * @param size
 */
void Client::Handle_Login(const char *data, unsigned int size)
{
	if (status != cs_waiting_for_login) {
		LogError("Login received after already having logged in");
		return;
	}

	if ((size - 12) % 8 != 0) {
		LogError("Login received packet of size: {0}, this would cause a block corruption, discarding", size);

		return;
	}

	if (size < sizeof(LoginLoginRequest_Struct)) {
		LogError("Login received packet of size: {0}, this would cause a buffer overflow, discarding", size);

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
	outbuffer.resize(size - 12);
	if (outbuffer.length() == 0) {
		LogError("Corrupt buffer sent to server, no length");
		return;
	}

	auto r = eqcrypt_block(data + 10, size - 12, &outbuffer[0], 0);
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

	memcpy(&llrs, data, sizeof(LoginLoginRequest_Struct));

	bool result = false;
	if (outbuffer[0] == 0 && outbuffer[1] == 0) {
		if (server.options.IsTokenLoginAllowed()) {
			cred   = (&outbuffer[2 + user.length()]);
			result = server.db->GetLoginTokenDataFromToken(
				cred,
				connection->GetRemoteAddr(),
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

			LogInfo(
				"Attempting password based login [{0}] login [{1}] user [{2}]",
				user,
				db_loginserver,
				user
			);

			ParseAccountString(user, user, db_loginserver);

			if (server.db->GetLoginDataFromAccountInfo(user, db_loginserver, db_account_password_hash, db_account_id)) {
				result = VerifyLoginHash(user, db_loginserver, cred, db_account_password_hash);

				LogDebug("[VerifyLoginHash] Success [{0}]", (result ? "true" : "false"));
			}
			else {
				status = cs_creating_account;
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
	if (status != cs_logged_in) {
		LogError("Client sent a play request when they were not logged in, discarding");
		return;
	}

	const auto *play        = (const PlayEverquestRequest_Struct *) data;
	auto       server_id_in = (unsigned int) play->ServerNumber;
	auto       sequence_in  = (unsigned int) play->Sequence;

	if (server.options.IsTraceOn()) {
		LogInfo(
			"Play received from client [{0}] server number {1} sequence {2}",
			GetAccountName(),
			server_id_in,
			sequence_in
		);
	}

	this->play_server_id = (unsigned int) play->ServerNumber;
	play_sequence_id = sequence_in;
	play_server_id   = server_id_in;
	server.server_manager->SendUserToWorldRequest(server_id_in, account_id, loginserver_name);
}

/**
 * @param seq
 */
void Client::SendServerListPacket(uint32 seq)
{
	EQApplicationPacket *outapp = server.server_manager->CreateServerListPacket(this, seq);

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(outapp);
	}

	connection->QueuePacket(outapp);
	delete outapp;
}

void Client::SendPlayResponse(EQApplicationPacket *outapp)
{
	if (server.options.IsTraceOn()) {
		LogDebug("Sending play response for {0}", GetAccountName());
		// server_log->LogPacket(log_network_trace, (const char*)outapp->pBuffer, outapp->size);
	}
	connection->QueuePacket(outapp);
}

void Client::GenerateKey()
{
	key.clear();
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

		key.append((const char *) &key_selection[random.Int(0, 35)], 1);
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
#ifdef LSPX
	if (loginserver == "eqemu") {
		LogInfo("Attempting login account creation via '{0}'", loginserver);

		if (!server.options.CanAutoLinkAccounts()) {
			LogInfo("CanAutoLinkAccounts disabled - sending failed login");
			DoFailedLogin();
			return;
		}

		if (server.options.GetEQEmuLoginServerAddress().length() == 0) {
			DoFailedLogin();
			return;
		}

		auto addr_components = SplitString(server.options.GetEQEmuLoginServerAddress(), ':');
		if (addr_components.size() != 2) {
			DoFailedLogin();
			return;
		}

		stored_user = user;
		stored_pass = pass;

		auto address = addr_components[0];
		auto port    = std::stoi(addr_components[1]);
		EQ::Net::DNSLookup(
			address, port, false, [=](const std::string &addr) {
				if (addr.empty()) {
					DoFailedLogin();
					return;
				}

				login_connection_manager.reset(new EQ::Net::DaybreakConnectionManager());
				login_connection_manager->OnNewConnection(
					std::bind(
						&Client::LoginOnNewConnection,
						this,
						std::placeholders::_1
					)
				);
				login_connection_manager->OnConnectionStateChange(
					std::bind(
						&Client::LoginOnStatusChange,
						this,
						std::placeholders::_1,
						std::placeholders::_2,
						std::placeholders::_3
					)
				);
				login_connection_manager->OnPacketRecv(
					std::bind(
						&Client::LoginOnPacketRecv,
						this,
						std::placeholders::_1,
						std::placeholders::_2
					)
				);

				login_connection_manager->Connect(addr, port);
			}
		);

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
	stored_user.clear();
	stored_pass.clear();

	EQApplicationPacket outapp(OP_LoginAccepted, sizeof(LoginLoginFailed_Struct));
	auto                *login_failed = (LoginLoginFailed_Struct *) outapp.pBuffer;

	login_failed->unknown1 = llrs.unknown1;
	login_failed->unknown2 = llrs.unknown2;
	login_failed->unknown3 = llrs.unknown3;
	login_failed->unknown4 = llrs.unknown4;
	login_failed->unknown5 = llrs.unknown5;

	memcpy(login_failed->unknown6, FailedLoginResponseData, sizeof(FailedLoginResponseData));

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(&outapp);
	}

	connection->QueuePacket(&outapp);
	status = cs_failed_to_login;
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
	stored_user.clear();
	stored_pass.clear();

	server.client_manager->RemoveExistingClient(db_account_id, db_loginserver);

	in_addr in{};
	in.s_addr = connection->GetRemoteIP();

	server.db->UpdateLSAccountData(db_account_id, std::string(inet_ntoa(in)));
	GenerateKey();

	account_id       = db_account_id;
	account_name     = in_account_name;
	loginserver_name = db_loginserver;

	auto *outapp         = new EQApplicationPacket(OP_LoginAccepted, 10 + 80);
	auto *login_accepted = (LoginAccepted_Struct *) outapp->pBuffer;
	login_accepted->unknown1 = llrs.unknown1;
	login_accepted->unknown2 = llrs.unknown2;
	login_accepted->unknown3 = llrs.unknown3;
	login_accepted->unknown4 = llrs.unknown4;
	login_accepted->unknown5 = llrs.unknown5;

	auto *login_failed_attempts = new LoginFailedAttempts_Struct;
	memset(login_failed_attempts, 0, sizeof(LoginFailedAttempts_Struct));

	login_failed_attempts->failed_attempts = 0;
	login_failed_attempts->message         = 0x01;
	login_failed_attempts->lsid            = db_account_id;
	login_failed_attempts->unknown3[3]  = 0x03;
	login_failed_attempts->unknown4[3]  = 0x02;
	login_failed_attempts->unknown5[0]  = 0xe7;
	login_failed_attempts->unknown5[1]  = 0x03;
	login_failed_attempts->unknown6[0]  = 0xff;
	login_failed_attempts->unknown6[1]  = 0xff;
	login_failed_attempts->unknown6[2]  = 0xff;
	login_failed_attempts->unknown6[3]  = 0xff;
	login_failed_attempts->unknown7[0]  = 0xa0;
	login_failed_attempts->unknown7[1]  = 0x05;
	login_failed_attempts->unknown8[3]  = 0x02;
	login_failed_attempts->unknown9[0]  = 0xff;
	login_failed_attempts->unknown9[1]  = 0x03;
	login_failed_attempts->unknown11[0] = 0x63;
	login_failed_attempts->unknown12[0] = 0x01;
	memcpy(login_failed_attempts->key, key.c_str(), key.size());

	char encrypted_buffer[80] = {0};
	auto rc                   = eqcrypt_block((const char *) login_failed_attempts, 75, encrypted_buffer, 1);
	if (rc == nullptr) {
		LogDebug("Failed to encrypt eqcrypt block");
	}

	memcpy(login_accepted->encrypt, encrypted_buffer, 80);

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(outapp);
	}

	connection->QueuePacket(outapp);
	delete outapp;

	status = cs_logged_in;
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
	login_connection = connection;
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

	login_connection->QueuePacket(p);
}

void Client::LoginSendLogin()
{
	size_t                  buffer_len = stored_user.length() + stored_pass.length() + 2;
	std::unique_ptr<char[]> buffer(new char[buffer_len]);

	strcpy(&buffer[0], stored_user.c_str());
	strcpy(&buffer[stored_user.length() + 1], stored_pass.c_str());

	size_t encrypted_len = buffer_len;

	if (encrypted_len % 8 > 0) {
		encrypted_len = ((encrypted_len / 8) + 1) * 8;
	}

	EQ::Net::DynamicPacket p;
	p.Resize(12 + encrypted_len);
	p.PutUInt16(0, 2); //OP_Login
	p.PutUInt32(2, 3);

	eqcrypt_block(&buffer[0], buffer_len, (char *) p.Data() + 12, true);

	login_connection->QueuePacket(p);
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

	login_connection_manager->OnConnectionStateChange(
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
		login_connection->Close();
	}
	else {
		LogDebug(
			"response [{0}] login succeeded user [{1}]",
			response_error,
			stored_user
		);

		auto m_dbid = sp.GetUInt32(8);

		CreateEQEmuAccount(stored_user, stored_pass, m_dbid);
		login_connection->Close();
	}
}
