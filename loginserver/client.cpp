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
#include "login_structures.h"
#include "../common/misc_functions.h"
#include "../common/eqemu_logsys.h"

extern EQEmuLogSys LogSys;
extern LoginServer server;

Client::Client(std::shared_ptr<EQStream> c, LSClientVersion v)
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
	while(app)
	{
		if(server.options.IsTraceOn())
		{
			Log(Logs::General, Logs::Login_Server, "Application packet received from client (size %u)", app->Size());
		}

		if(server.options.IsDumpInPacketsOn())
		{
			DumpPacket(app);
		}

		switch(app->GetOpcode())
		{
		case OP_SessionReady:
			{
				if(server.options.IsTraceOn())
				{
					Log(Logs::General, Logs::Login_Server, "Session ready received from client.");
				}
				Handle_SessionReady((const char*)app->pBuffer, app->Size());
				break;
			}
		case OP_Login:
			{
				if(app->Size() < 20)
				{
					Log(Logs::General, Logs::Error, "Login received but it is too small, discarding.");
					break;
				}

				if(server.options.IsTraceOn())
				{
					Log(Logs::General, Logs::Login_Server, "Login received from client.");
				}

				Handle_Login((const char*)app->pBuffer, app->Size());
				break;
			}
		case OP_ServerListRequest:
			{
				if(server.options.IsTraceOn())
				{
					Log(Logs::General, Logs::Login_Server, "Server list request received from client.");
				}

				SendServerListPacket();
				break;
			}
		case OP_PlayEverquestRequest:
			{
				if(app->Size() < sizeof(PlayEverquestRequest_Struct))
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
	if(status != cs_not_sent_session_ready)
	{
		Log(Logs::General, Logs::Error, "Session ready received again after already being received.");
		return;
	}

	if(size < sizeof(unsigned int))
	{
		Log(Logs::General, Logs::Error, "Session ready was too small.");
		return;
	}

	unsigned int mode = *((unsigned int*)data);
	if(mode == (unsigned int)lm_from_world)
	{
		Log(Logs::General, Logs::Login_Server, "Session ready indicated logged in from world(unsupported feature), disconnecting.");
		connection->Close();
		return;
	}

	status = cs_waiting_for_login;

	/**
	* The packets are mostly the same but slightly different between the two versions.
	*/
	if(version == cv_sod)
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_ChatMessage, 17);
		outapp->pBuffer[0] = 0x02;
		outapp->pBuffer[10] = 0x01;
		outapp->pBuffer[11] = 0x65;

		if(server.options.IsDumpOutPacketsOn())
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

		if(server.options.IsDumpOutPacketsOn())
		{
			DumpPacket(outapp);
		}

		connection->QueuePacket(outapp);
		delete outapp;
	}
}

void Client::Handle_Login(const char* data, unsigned int size)
{
	if(status != cs_waiting_for_login) {
		Log(Logs::General, Logs::Error, "Login received after already having logged in.");
		return;
	}

	if((size - 12) % 8 != 0) {
		Log(Logs::General, Logs::Error, "Login received packet of size: %u, this would cause a block corruption, discarding.", size);
		return;
	}

	status = cs_logged_in;

	string entered_username;
	string entered_password_hash_result;

	char *login_packet_buffer = nullptr;

	unsigned int db_account_id = 0;
	string db_account_password_hash;

#ifdef WIN32
	login_packet_buffer = server.eq_crypto->DecryptUsernamePassword(data, size, server.options.GetEncryptionMode());

	int login_packet_buffer_length = strlen(login_packet_buffer);
	entered_password_hash_result.assign(login_packet_buffer, login_packet_buffer_length);
	entered_username.assign((login_packet_buffer + login_packet_buffer_length + 1), strlen(login_packet_buffer + login_packet_buffer_length + 1));

	if(server.options.IsTraceOn()) {
		Log(Logs::General, Logs::Debug, "User: %s", entered_username.c_str());
		Log(Logs::General, Logs::Debug, "Hash: %s", entered_password_hash_result.c_str());
	}

	server.eq_crypto->DeleteHeap(login_packet_buffer);
#else
	login_packet_buffer = DecryptUsernamePassword(data, size, server.options.GetEncryptionMode());

	int login_packet_buffer_length = strlen(login_packet_buffer);
	entered_password_hash_result.assign(login_packet_buffer, login_packet_buffer_length);
	entered_username.assign((login_packet_buffer + login_packet_buffer_length + 1), strlen(login_packet_buffer + login_packet_buffer_length + 1));

	if(server.options.IsTraceOn()) {
		Log(Logs::General, Logs::Debug, "User: %s", entered_username.c_str());
		Log(Logs::General, Logs::Debug, "Hash: %s", entered_password_hash_result.c_str());
	}

	_HeapDeleteCharBuffer(login_packet_buffer);
#endif

	bool result;
	if(server.db->GetLoginDataFromAccountName(entered_username, db_account_password_hash, db_account_id) == false) {
		/* If we have auto_create_accounts enabled in the login.ini, we will process the creation of an account on our own*/
		if (
			server.config->GetVariable("options", "auto_create_accounts").compare("TRUE") == 0 && 
			server.db->CreateLoginData(entered_username, entered_password_hash_result, db_account_id) == true
		){
			Log(Logs::General, Logs::Error, "User %s does not exist in the database, so we created it...", entered_username.c_str());
			result = true;
		}
		else{
			Log(Logs::General, Logs::Error, "Error logging in, user %s does not exist in the database.", entered_username.c_str());
			result = false;
		}
	}
	else {
		if(db_account_password_hash.compare(entered_password_hash_result) == 0) {
			result = true;
		}
		else {
			result = false;
		}
	}

	/* Login Accepted */
	if(result) {

		server.client_manager->RemoveExistingClient(db_account_id);

		in_addr in;
		in.s_addr = connection->GetRemoteIP();

		server.db->UpdateLSAccountData(db_account_id, string(inet_ntoa(in)));
		GenerateKey();

		account_id = db_account_id;
		account_name = entered_username;

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_LoginAccepted, 10 + 80);
		const LoginLoginRequest_Struct* llrs = (const LoginLoginRequest_Struct *)data;
		LoginAccepted_Struct* login_accepted = (LoginAccepted_Struct *)outapp->pBuffer;
		login_accepted->unknown1 = llrs->unknown1;
		login_accepted->unknown2 = llrs->unknown2;
		login_accepted->unknown3 = llrs->unknown3;
		login_accepted->unknown4 = llrs->unknown4;
		login_accepted->unknown5 = llrs->unknown5;

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

#ifdef WIN32
		unsigned int e_size;
		char *encrypted_buffer = server.eq_crypto->Encrypt((const char*)login_failed_attempts, 75, e_size);
		memcpy(login_accepted->encrypt, encrypted_buffer, 80);
		server.eq_crypto->DeleteHeap(encrypted_buffer);
#else
		unsigned int e_size;
		char *encrypted_buffer = Encrypt((const char*)login_failed_attempts, 75, e_size);
		memcpy(login_accepted->encrypt, encrypted_buffer, 80);
		_HeapDeleteCharBuffer(encrypted_buffer);
#endif

		if(server.options.IsDumpOutPacketsOn()) {
			DumpPacket(outapp);
		}

		connection->QueuePacket(outapp);
		delete outapp;
	}
	else {
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_LoginAccepted, sizeof(LoginLoginFailed_Struct));
		const LoginLoginRequest_Struct* llrs = (const LoginLoginRequest_Struct *)data;
		LoginLoginFailed_Struct* llas = (LoginLoginFailed_Struct *)outapp->pBuffer;
		llas->unknown1 = llrs->unknown1;
		llas->unknown2 = llrs->unknown2;
		llas->unknown3 = llrs->unknown3;
		llas->unknown4 = llrs->unknown4;
		llas->unknown5 = llrs->unknown5;
		memcpy(llas->unknown6, FailedLoginResponseData, sizeof(FailedLoginResponseData));

		if(server.options.IsDumpOutPacketsOn()) {
			DumpPacket(outapp);
		}

		connection->QueuePacket(outapp);
		delete outapp;
	}
}

void Client::Handle_Play(const char* data)
{
	if(status != cs_logged_in)
	{
		Log(Logs::General, Logs::Error, "Client sent a play request when they either were not logged in, discarding.");
		return;
	}

	const PlayEverquestRequest_Struct *play = (const PlayEverquestRequest_Struct*)data;
	unsigned int server_id_in = (unsigned int)play->ServerNumber;
	unsigned int sequence_in = (unsigned int)play->Sequence;

	if(server.options.IsTraceOn())
	{
		Log(Logs::General, Logs::Login_Server, "Play received from client, server number %u sequence %u.", server_id_in, sequence_in);
	}

	this->play_server_id = (unsigned int)play->ServerNumber;
	play_sequence_id = sequence_in;
	play_server_id = server_id_in;
	server.server_manager->SendUserToWorldRequest(server_id_in, account_id);
}

void Client::SendServerListPacket()
{
	EQApplicationPacket *outapp = server.server_manager->CreateServerListPacket(this);

	if(server.options.IsDumpOutPacketsOn())
	{
		DumpPacket(outapp);
	}

	connection->QueuePacket(outapp);
	delete outapp;
}

void Client::SendPlayResponse(EQApplicationPacket *outapp)
{
	if(server.options.IsTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "Sending play response for %s.", GetAccountName().c_str());
		// server_log->LogPacket(log_network_trace, (const char*)outapp->pBuffer, outapp->size);
	}
	connection->QueuePacket(outapp);
	status = cs_logged_in;
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

