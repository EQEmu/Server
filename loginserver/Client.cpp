/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2010  EQEMu Development Team (http://eqemulator.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "Client.h"
#include "ErrorLog.h"
#include "LoginServer.h"
#include "LoginStructures.h"
#include "../common/MiscFunctions.h"

extern ErrorLog *server_log;
extern LoginServer server;

Client::Client(EQStream *c, ClientVersion v)
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
			server_log->Log(log_network, "Application packet received from client (size %u)", app->Size());
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
					server_log->Log(log_network, "Session ready received from client.");
				}
				Handle_SessionReady((const char*)app->pBuffer, app->Size());
				break;
			}
		case OP_Login:
			{
				if(app->Size() < 20)
				{
					server_log->Log(log_network_error, "Login received but it is too small, discarding.");
					break;
				}

				if(server.options.IsTraceOn())
				{
					server_log->Log(log_network, "Login received from client.");
				}

				Handle_Login((const char*)app->pBuffer, app->Size());
				break;
			}
		case OP_ServerListRequest:
			{
				if(server.options.IsTraceOn())
				{
					server_log->Log(log_network, "Server list request received from client.");
				}

				SendServerListPacket();
				break;
			}
		case OP_PlayEverquestRequest:
			{
				if(app->Size() < sizeof(PlayEverquestRequest_Struct))
				{
					server_log->Log(log_network_error, "Play received but it is too small, discarding.");
					break;
				}

				Handle_Play((const char*)app->pBuffer);
				break;
			}
		default:
			{
				char dump[64];
				app->build_header_dump(dump);
				server_log->Log(log_network_error, "Recieved unhandled application packet from the client: %s.", dump);
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
		server_log->Log(log_network_error, "Session ready received again after already being received.");
		return;
	}

	if(size < sizeof(unsigned int))
	{
		server_log->Log(log_network_error, "Session ready was too small.");
		return;
	}

	unsigned int mode = *((unsigned int*)data);
	if(mode == (unsigned int)lm_from_world)
	{
		server_log->Log(log_network, "Session ready indicated logged in from world(unsupported feature), disconnecting.");
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
	if(status != cs_waiting_for_login)
	{
		server_log->Log(log_network_error, "Login received after already having logged in.");
		return;
	}

	if((size - 12) % 8 != 0)
	{
		server_log->Log(log_network_error, "Login received packet of size: %u, this would cause a block corruption, discarding.", size);
		return;
	}

	status = cs_logged_in;

	string e_user;
	string e_hash;
	char *e_buffer = nullptr;
	unsigned int d_account_id = 0;
	string d_pass_hash;

#ifdef WIN32
	e_buffer = server.eq_crypto->DecryptUsernamePassword(data, size, server.options.GetEncryptionMode());
	
	int buffer_len = strlen(e_buffer);
	e_hash.assign(e_buffer, buffer_len);
	e_user.assign((e_buffer + buffer_len + 1), strlen(e_buffer + buffer_len + 1));

	if(server.options.IsTraceOn())
	{
		server_log->Log(log_client, "User: %s", e_user.c_str());
		server_log->Log(log_client, "Hash: %s", e_hash.c_str());
	}

	server.eq_crypto->DeleteHeap(e_buffer);
#else
	e_buffer = DecryptUsernamePassword(data, size, server.options.GetEncryptionMode());

	int buffer_len = strlen(e_buffer);
	e_hash.assign(e_buffer, buffer_len);
	e_user.assign((e_buffer + buffer_len + 1), strlen(e_buffer + buffer_len + 1));

	if(server.options.IsTraceOn())
	{
		server_log->Log(log_client, "User: %s", e_user.c_str());
		server_log->Log(log_client, "Hash: %s", e_hash.c_str());
	}

	_HeapDeleteCharBuffer(e_buffer);
#endif

	bool result;
	if(server.db->GetLoginDataFromAccountName(e_user, d_pass_hash, d_account_id) == false)
	{
		server_log->Log(log_client_error, "Error logging in, user %s does not exist in the database.", e_user.c_str());
		result = false;
	}
	else
	{
		if(d_pass_hash.compare(e_hash) == 0)
		{
			result = true;
		}
		else
		{
			result = false;
		}
	}

	if(result)
	{
		server.CM->RemoveExistingClient(d_account_id);
		in_addr in;
		in.s_addr = connection->GetRemoteIP();
		server.db->UpdateLSAccountData(d_account_id, string(inet_ntoa(in)));
		GenerateKey();
		account_id = d_account_id;
		account_name = e_user;

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_LoginAccepted, 10 + 80);
		const LoginLoginRequest_Struct* llrs = (const LoginLoginRequest_Struct *)data;
		LoginLoginAccepted_Struct* llas = (LoginLoginAccepted_Struct *)outapp->pBuffer;
		llas->unknown1 = llrs->unknown1;
		llas->unknown2 = llrs->unknown2;
		llas->unknown3 = llrs->unknown3;
		llas->unknown4 = llrs->unknown4;
		llas->unknown5 = llrs->unknown5;

		Login_ReplyBlock_Struct * lrbs = new Login_ReplyBlock_Struct;
		memset(lrbs, 0, sizeof(Login_ReplyBlock_Struct));

		lrbs->failed_attempts = 0;
		lrbs->message = 0x01;
		lrbs->lsid = d_account_id;
		lrbs->unknown3[3] = 0x03;
		lrbs->unknown4[3] = 0x02;
		lrbs->unknown5[0] = 0xe7;
		lrbs->unknown5[1] = 0x03;
		lrbs->unknown6[0] = 0xff;
		lrbs->unknown6[1] = 0xff;
		lrbs->unknown6[2] = 0xff;
		lrbs->unknown6[3] = 0xff;
		lrbs->unknown7[0] = 0xa0;
		lrbs->unknown7[1] = 0x05;
		lrbs->unknown8[3] = 0x02;
		lrbs->unknown9[0] = 0xff;
		lrbs->unknown9[1] = 0x03;
		lrbs->unknown11[0] = 0x63;
		lrbs->unknown12[0] = 0x01;
		memcpy(lrbs->key, key.c_str(), key.size());

#ifdef WIN32
		unsigned int e_size;
		char *encrypted_buffer = server.eq_crypto->Encrypt((const char*)lrbs, 75, e_size);
		memcpy(llas->encrypt, encrypted_buffer, 80);
		server.eq_crypto->DeleteHeap(encrypted_buffer);
#else
		unsigned int e_size;
		char *encrypted_buffer = Encrypt((const char*)lrbs, 75, e_size);
		memcpy(llas->encrypt, encrypted_buffer, 80);
		_HeapDeleteCharBuffer(encrypted_buffer);
#endif

		if(server.options.IsDumpOutPacketsOn())
		{
			DumpPacket(outapp);
		}

		connection->QueuePacket(outapp);
		delete outapp;
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_LoginAccepted, sizeof(LoginLoginFailed_Struct));
		const LoginLoginRequest_Struct* llrs = (const LoginLoginRequest_Struct *)data;
		LoginLoginFailed_Struct* llas = (LoginLoginFailed_Struct *)outapp->pBuffer;
		llas->unknown1 = llrs->unknown1;
		llas->unknown2 = llrs->unknown2;
		llas->unknown3 = llrs->unknown3;
		llas->unknown4 = llrs->unknown4;
		llas->unknown5 = llrs->unknown5;
		memcpy(llas->unknown6, FailedLoginResponseData, sizeof(FailedLoginResponseData));

		if(server.options.IsDumpOutPacketsOn())
		{
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
		server_log->Log(log_client_error, "Client sent a play request when they either were not logged in, discarding.");
		return;
	}

	const PlayEverquestRequest_Struct *play = (const PlayEverquestRequest_Struct*)data;
	unsigned int server_id_in = (unsigned int)play->ServerNumber;
	unsigned int sequence_in = (unsigned int)play->Sequence;

	if(server.options.IsTraceOn())
	{
		server_log->Log(log_network, "Play received from client, server number %u sequence %u.", server_id_in, sequence_in);
	}

	this->play_server_id = (unsigned int)play->ServerNumber;
	play_sequence_id = sequence_in;
	play_server_id = server_id_in;
	server.SM->SendUserToWorldRequest(server_id_in, account_id);
}

void Client::SendServerListPacket()
{
	EQApplicationPacket *outapp = server.SM->CreateServerListPacket(this);
	
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
		server_log->Log(log_network_trace, "Sending play response for %s.", GetAccountName().c_str());
		server_log->LogPacket(log_network_trace, (const char*)outapp->pBuffer, outapp->size);
	}
	connection->QueuePacket(outapp);
	status = cs_logged_in;
}

void Client::GenerateKey()
{
	key.clear();
	int count = 0;
	while(count < 10)
	{
		static const char key_selection[] = 
		{
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
			'Y', 'Z', '0', '1', '2', '3', '4', '5', 
			'6', '7', '8', '9'
		};

		key.append((const char*)&key_selection[MakeRandomInt(0, 35)], 1);
		count++;
	}
}

