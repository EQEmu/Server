/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "client.h"
#include "login_server.h"
#include "../common/misc_functions.h"
#include "../common/eqemu_logsys.h"
#include "../common/string_util.h"
#include "encryption.h"

extern LoginServer server;

Client::Client(std::shared_ptr<EQStreamInterface> c, LSClientVersion v)
{
	connection = c;
	version = v;
	status = cs_not_sent_session_ready;
	account_id = 0;
	play_server_id = 0;
	play_sequence_id = 0;
}

bool Client::Process()
{
	EQApplicationPacket *app = connection->PopPacket();
	while (app)
	{
		if (server.options.IsTraceOn())
		{
			Log(Logs::General, Logs::Login_Server, "Application packet received from client (size %u)", app->Size());
		}

		if (server.options.IsDumpInPacketsOn())
		{
			DumpPacket(app);
		}

		if (status == cs_failed_to_login) {
			delete app;
			app = connection->PopPacket();
			continue;
		}

		switch (app->GetOpcode())
		{
		case OP_SessionReady:
		{
			if (server.options.IsTraceOn())
			{
				Log(Logs::General, Logs::Login_Server, "Session ready received from client.");
			}
			Handle_SessionReady((const char*)app->pBuffer, app->Size());
			break;
		}
		case OP_Login:
		{
			if (app->Size() < 20)
			{
				Log(Logs::General, Logs::Error, "Login received but it is too small, discarding.");
				break;
			}

			if (server.options.IsTraceOn())
			{
				Log(Logs::General, Logs::Login_Server, "Login received from client.");
			}

			Handle_Login((const char*)app->pBuffer, app->Size());
			break;
		}
		case OP_ServerListRequest:
		{
			if (app->Size() < 4) {
				Log(Logs::General, Logs::Error, "Server List Request received but it is too small, discarding.");
				break;
			}

			if (server.options.IsTraceOn())
			{
				Log(Logs::General, Logs::Login_Server, "Server list request received from client.");
			}

			SendServerListPacket(*(uint32_t*)app->pBuffer);
			break;
		}
		case OP_PlayEverquestRequest:
		{
			if (app->Size() < sizeof(PlayEverquestRequest_Struct))
			{
				Log(Logs::General, Logs::Error, "Play received but it is too small, discarding.");
				break;
			}

			Handle_Play((const char*)app->pBuffer);
			break;
		}
		default:
		{
			if (LogSys.log_settings[Logs::Client_Server_Packet_Unhandled].is_category_enabled == 1) {
				char dump[64];
				app->build_header_dump(dump);
				Log(Logs::General, Logs::Error, "Recieved unhandled application packet from the client: %s.", dump);
			}
		}
		}

		delete app;
		app = connection->PopPacket();
	}

	return true;
}

void Client::Handle_SessionReady(const char* data, unsigned int size)
{
	if (status != cs_not_sent_session_ready)
	{
		Log(Logs::General, Logs::Error, "Session ready received again after already being received.");
		return;
	}

	if (size < sizeof(unsigned int))
	{
		Log(Logs::General, Logs::Error, "Session ready was too small.");
		return;
	}

	status = cs_waiting_for_login;

	/**
	* The packets are mostly the same but slightly different between the two versions.
	*/
	if (version == cv_sod)
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_ChatMessage, 17);
		outapp->pBuffer[0] = 0x02;
		outapp->pBuffer[10] = 0x01;
		outapp->pBuffer[11] = 0x65;

		if (server.options.IsDumpOutPacketsOn())
		{
			DumpPacket(outapp);
		}

		connection->QueuePacket(outapp);
		delete outapp;
	}
	else
	{
		const char *msg = "ChatMessage";
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_ChatMessage, 16 + strlen(msg));
		outapp->pBuffer[0] = 0x02;
		outapp->pBuffer[10] = 0x01;
		outapp->pBuffer[11] = 0x65;
		strcpy((char*)(outapp->pBuffer + 15), msg);

		if (server.options.IsDumpOutPacketsOn())
		{
			DumpPacket(outapp);
		}

		connection->QueuePacket(outapp);
		delete outapp;
	}
}

void Client::Handle_Login(const char* data, unsigned int size)
{
	if (status != cs_waiting_for_login) {
		Log(Logs::General, Logs::Error, "Login received after already having logged in.");
		return;
	}

	if ((size - 12) % 8 != 0) {
		Log(Logs::General, Logs::Error, "Login received packet of size: %u, this would cause a block corruption, discarding.", size);
		return;
	}

	if (size < sizeof(LoginLoginRequest_Struct)) {
		Log(Logs::General, Logs::Error, "Login received packet of size: %u, this would cause a buffer overflow, discarding.", size);
		return;
	}

	char *login_packet_buffer = nullptr;

	unsigned int db_account_id = 0;
	std::string db_loginserver = "eqemu";
	std::string db_account_password_hash;

	std::string outbuffer;
	outbuffer.resize(size - 12);
	if (outbuffer.length() == 0) {
		LogF(Logs::General, Logs::Debug, "Corrupt buffer sent to server, no length.");
		return;
	}

	auto r = eqcrypt_block(data + 10, size - 12, &outbuffer[0], 0);
	if (r == nullptr) {
		LogF(Logs::General, Logs::Debug, "Failed to decrypt eqcrypt block");
		return;
	}

	std::string cred;

	std::string user(&outbuffer[0]);
	if (user.length() >= outbuffer.length()) {
		LogF(Logs::General, Logs::Debug, "Corrupt buffer sent to server, preventing buffer overflow.");
		return;
	}

	memcpy(&llrs, data, sizeof(LoginLoginRequest_Struct));

	bool result = false;
	if (outbuffer[0] == 0 && outbuffer[1] == 0) {
		if (server.options.IsTokenLoginAllowed()) {
			cred = (&outbuffer[2 + user.length()]);
			result = server.db->GetLoginTokenDataFromToken(cred, connection->GetRemoteAddr(), db_account_id, db_loginserver, user);
		}
	}
	else {
		if (server.options.IsPasswordLoginAllowed()) {
			cred = (&outbuffer[1 + user.length()]);
			auto components = SplitString(user, '.');
			if (components.size() == 2) {
				db_loginserver = components[0];
				user = components[1];
			}

			ParseAccountString(user, user, db_loginserver);
			
			if (server.db->GetLoginDataFromAccountInfo(user, db_loginserver, db_account_password_hash, db_account_id) == false) {
				status = cs_creating_account;
				AttemptLoginAccountCreation(user, cred, db_loginserver);
				return;
			}
			else {
				result = VerifyLoginHash(user, db_loginserver, cred, db_account_password_hash);
			}
		}
	}

	/* Login Accepted */
	if (result) {
		DoSuccessfulLogin(user, db_account_id, db_loginserver);
	}
	else {
		DoFailedLogin();
	}
}

void Client::Handle_Play(const char* data)
{
	if (status != cs_logged_in)
	{
		Log(Logs::General, Logs::Error, "Client sent a play request when they were not logged in, discarding.");
		return;
	}

	const PlayEverquestRequest_Struct *play = (const PlayEverquestRequest_Struct*)data;
	unsigned int server_id_in = (unsigned int)play->ServerNumber;
	unsigned int sequence_in = (unsigned int)play->Sequence;

	if (server.options.IsTraceOn())
	{
		Log(Logs::General, Logs::Login_Server, "Play received from client, server number %u sequence %u.", server_id_in, sequence_in);
	}

	this->play_server_id = (unsigned int)play->ServerNumber;
	play_sequence_id = sequence_in;
	play_server_id = server_id_in;
	server.server_manager->SendUserToWorldRequest(server_id_in, account_id, loginserver_name);
}

void Client::SendServerListPacket(uint32 seq)
{
	EQApplicationPacket *outapp = server.server_manager->CreateServerListPacket(this, seq);

	if (server.options.IsDumpOutPacketsOn())
	{
		DumpPacket(outapp);
	}

	connection->QueuePacket(outapp);
	delete outapp;
}

void Client::SendPlayResponse(EQApplicationPacket *outapp)
{
	if (server.options.IsTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "Sending play response for %s.", GetAccountName().c_str());
		// server_log->LogPacket(log_network_trace, (const char*)outapp->pBuffer, outapp->size);
	}
	connection->QueuePacket(outapp);
}

void Client::GenerateKey()
{
	key.clear();
	int count = 0;
	while (count < 10)
	{
		static const char key_selection[] =
		{
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
			'Y', 'Z', '0', '1', '2', '3', '4', '5',
			'6', '7', '8', '9'
		};

		key.append((const char*)&key_selection[random.Int(0, 35)], 1);
		count++;
	}
}

void Client::AttemptLoginAccountCreation(const std::string &user, const std::string &pass, const std::string &loginserver)
{
	if (loginserver == "eqemu") {
		if (!server.options.CanAutoLinkAccounts()) {
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
		auto port = std::stoi(addr_components[1]);
		EQ::Net::DNSLookup(address, port, false, [=](const std::string &addr) {
			if (addr.empty()) {
				DoFailedLogin();
				return;
			}
		
			login_connection_manager.reset(new EQ::Net::DaybreakConnectionManager());
			login_connection_manager->OnNewConnection(std::bind(&Client::LoginOnNewConnection, this, std::placeholders::_1));
			login_connection_manager->OnConnectionStateChange(std::bind(&Client::LoginOnStatusChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			login_connection_manager->OnPacketRecv(std::bind(&Client::LoginOnPacketRecv, this, std::placeholders::_1, std::placeholders::_2));
			
			login_connection_manager->Connect(addr, port);
		});
	}
	else {
		if (!server.options.CanAutoCreateAccounts()) {
			DoFailedLogin();
			return;
		}

		CreateLocalAccount(user, pass);
	}
}

void Client::DoFailedLogin()
{
	stored_user.clear();
	stored_pass.clear();

	EQApplicationPacket outapp(OP_LoginAccepted, sizeof(LoginLoginFailed_Struct));
	LoginLoginFailed_Struct* llas = (LoginLoginFailed_Struct *)outapp.pBuffer;
	llas->unknown1 = llrs.unknown1;
	llas->unknown2 = llrs.unknown2;
	llas->unknown3 = llrs.unknown3;
	llas->unknown4 = llrs.unknown4;
	llas->unknown5 = llrs.unknown5;
	memcpy(llas->unknown6, FailedLoginResponseData, sizeof(FailedLoginResponseData));

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(&outapp);
	}

	connection->QueuePacket(&outapp);
	status = cs_failed_to_login;
}

bool Client::VerifyLoginHash(const std::string &user, const std::string &loginserver, const std::string &cred, const std::string &hash)
{
	auto mode = server.options.GetEncryptionMode();
	if (eqcrypt_verify_hash(user, cred, hash, mode)) {
		return true;
	}
	else {
		if (server.options.IsUpdatingInsecurePasswords()) {
			if (mode < EncryptionModeArgon2) {
				mode = EncryptionModeArgon2;
			}

			if (hash.length() == 32) { //md5 is insecure
				for (int i = EncryptionModeMD5; i <= EncryptionModeMD5Triple; ++i) {
					if (i != mode && eqcrypt_verify_hash(user, cred, hash, i)) {
						server.db->UpdateLoginHash(user, loginserver, eqcrypt_hash(user, cred, mode));
						return true;
					}
				}
			}
			else if (hash.length() == 40) { //sha1 is insecure
				for (int i = EncryptionModeSHA; i <= EncryptionModeSHATriple; ++i) {
					if (i != mode && eqcrypt_verify_hash(user, cred, hash, i)) {
						server.db->UpdateLoginHash(user, loginserver, eqcrypt_hash(user, cred, mode));
						return true;
					}
				}
			}
			else if (hash.length() == 128) { //sha2-512 is insecure
				for (int i = EncryptionModeSHA512; i <= EncryptionModeSHA512Triple; ++i) {
					if (i != mode && eqcrypt_verify_hash(user, cred, hash, i)) {
						server.db->UpdateLoginHash(user, loginserver, eqcrypt_hash(user, cred, mode));
						return true;
					}
				}
			}
			//argon2 is still secure
			//scrypt is still secure
		}
	}

	return false;
}

void Client::DoSuccessfulLogin(const std::string &user, int db_account_id, const std::string &db_loginserver)
{
	stored_user.clear();
	stored_pass.clear();

	server.client_manager->RemoveExistingClient(db_account_id, db_loginserver);

	in_addr in;
	in.s_addr = connection->GetRemoteIP();

	server.db->UpdateLSAccountData(db_account_id, std::string(inet_ntoa(in)));
	GenerateKey();

	account_id = db_account_id;
	account_name = user;
	loginserver_name = db_loginserver;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_LoginAccepted, 10 + 80);
	LoginAccepted_Struct* login_accepted = (LoginAccepted_Struct *)outapp->pBuffer;
	login_accepted->unknown1 = llrs.unknown1;
	login_accepted->unknown2 = llrs.unknown2;
	login_accepted->unknown3 = llrs.unknown3;
	login_accepted->unknown4 = llrs.unknown4;
	login_accepted->unknown5 = llrs.unknown5;

	LoginFailedAttempts_Struct * login_failed_attempts = new LoginFailedAttempts_Struct;
	memset(login_failed_attempts, 0, sizeof(LoginFailedAttempts_Struct));

	login_failed_attempts->failed_attempts = 0;
	login_failed_attempts->message = 0x01;
	login_failed_attempts->lsid = db_account_id;
	login_failed_attempts->unknown3[3] = 0x03;
	login_failed_attempts->unknown4[3] = 0x02;
	login_failed_attempts->unknown5[0] = 0xe7;
	login_failed_attempts->unknown5[1] = 0x03;
	login_failed_attempts->unknown6[0] = 0xff;
	login_failed_attempts->unknown6[1] = 0xff;
	login_failed_attempts->unknown6[2] = 0xff;
	login_failed_attempts->unknown6[3] = 0xff;
	login_failed_attempts->unknown7[0] = 0xa0;
	login_failed_attempts->unknown7[1] = 0x05;
	login_failed_attempts->unknown8[3] = 0x02;
	login_failed_attempts->unknown9[0] = 0xff;
	login_failed_attempts->unknown9[1] = 0x03;
	login_failed_attempts->unknown11[0] = 0x63;
	login_failed_attempts->unknown12[0] = 0x01;
	memcpy(login_failed_attempts->key, key.c_str(), key.size());

	char encrypted_buffer[80] = { 0 };
	auto rc = eqcrypt_block((const char*)login_failed_attempts, 75, encrypted_buffer, 1);
	if (rc == nullptr) {
		LogF(Logs::General, Logs::Debug, "Failed to encrypt eqcrypt block");
	}

	memcpy(login_accepted->encrypt, encrypted_buffer, 80);

	if (server.options.IsDumpOutPacketsOn()) {
		DumpPacket(outapp);
	}

	connection->QueuePacket(outapp);
	delete outapp;

	status = cs_logged_in;
}

void Client::CreateLocalAccount(const std::string &user, const std::string &pass)
{
	auto mode = server.options.GetEncryptionMode();
	auto hash = eqcrypt_hash(user, pass, mode);

	unsigned int db_id = 0;
	std::string db_login = server.options.GetDefaultLoginServerName();
	if (!server.db->CreateLoginData(user, hash, db_login, db_id)) {
		DoFailedLogin();
	}
	else {
		DoSuccessfulLogin(user, db_id, db_login);
	}
}

void Client::CreateEQEmuAccount(const std::string &user, const std::string &pass, unsigned int id)
{
	auto mode = server.options.GetEncryptionMode();
	auto hash = eqcrypt_hash(user, pass, mode);

	if (!server.db->CreateLoginDataWithID(user, hash, "eqemu", id)) {
		DoFailedLogin();
	}
	else {
		DoSuccessfulLogin(user, id, "eqemu");
	}
}

void Client::LoginOnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection)
{
	login_connection = connection;
}

void Client::LoginOnStatusChange(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusConnected) {
		LoginSendSessionReady();
	}

	if (to == EQ::Net::StatusDisconnecting || to == EQ::Net::StatusDisconnected) {
		DoFailedLogin();
	}
}

void Client::LoginOnStatusChangeIgnored(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
}

void Client::LoginOnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet & p)
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
	size_t buffer_len = stored_user.length() + stored_pass.length() + 2;
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

	eqcrypt_block(&buffer[0], buffer_len, (char*)p.Data() + 12, true);

	login_connection->QueuePacket(p);
}

void Client::LoginProcessLoginResponse(const EQ::Net::Packet & p)
{
	auto encrypt_size = p.Length() - 12;
	if (encrypt_size % 8 > 0) {
		encrypt_size = (encrypt_size / 8) * 8;
	}

	std::unique_ptr<char[]> decrypted(new char[encrypt_size]);

	eqcrypt_block((char*)p.Data() + 12, encrypt_size, &decrypted[0], false);

	EQ::Net::StaticPacket sp(&decrypted[0], encrypt_size);
	auto response_error = sp.GetUInt16(1);

	login_connection_manager->OnConnectionStateChange(std::bind(&Client::LoginOnStatusChangeIgnored, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	if (response_error > 101) {
		DoFailedLogin();
		login_connection->Close();
	}
	else {
		auto m_dbid = sp.GetUInt32(8);

		CreateEQEmuAccount(stored_user, stored_pass, m_dbid);
		login_connection->Close();
	}
}
