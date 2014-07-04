#include "../common/debug.h"
#include "socket_server.h"
#include "WorldConfig.h"
#include "clientlist.h"
#include "zonelist.h"
#include "../common/logsys.h"
#include "../common/logtypes.h"
#include "../common/md5.h"
#include "../common/EmuTCPConnection.h"
#include "../common/packet_dump.h"

extern ClientList client_list;
extern ZSList zoneserver_list;

Socket_Server_Connection::Socket_Server_Connection()
{
	Stream = 0;
	authenticated = false;
}

void Socket_Server_Connection::SetConnection(EmuTCPConnection *inStream)
{
	if(Stream)
	{
		_log(SOCKET_SERVER__ERROR, "Incoming Socket_Server Connection while we were already connected to a Socket_Server.");
		Stream->Disconnect();
	}

	Stream = inStream;

	authenticated = false;
}

bool Socket_Server_Connection::Process()
{
	if (!Stream || !Stream->Connected())
		return false;

	ServerPacket *pack = 0;

	while((pack = Stream->PopPacket()))
	{
		if (!authenticated)
		{
			if (WorldConfig::get()->SharedKey.length() > 0)
			{
				if (pack->opcode == ServerOP_ZAAuth && pack->size == 16)
				{
					uint8 tmppass[16];

					MD5::Generate((const uchar*) WorldConfig::get()->SharedKey.c_str(), WorldConfig::get()->SharedKey.length(), tmppass);

					if (memcmp(pack->pBuffer, tmppass, 16) == 0)
						authenticated = true;
					else
					{
						struct in_addr in;
						in.s_addr = GetIP();
						_log(SOCKET_SERVER__ERROR, "Socket_Server authorization failed.");
						ServerPacket* pack = new ServerPacket(ServerOP_ZAAuthFailed);
						SendPacket(pack);
						delete pack;
						Disconnect();
						return false;
					}
				}
				else
				{
					struct in_addr in;
					in.s_addr = GetIP();
					_log(SOCKET_SERVER__ERROR, "Socket_Server_ authorization failed.");
					ServerPacket* pack = new ServerPacket(ServerOP_ZAAuthFailed);
					SendPacket(pack);
					delete pack;
					Disconnect();
					return false;
				}
			}
			else
			{
				_log(SOCKET_SERVER__ERROR,"**WARNING** You have not configured a world shared key in your config file. You should add a <key>STRING</key> element to your <world> element to prevent unauthorized zone access.");
				authenticated = true;
			}
			delete pack;
			continue;
		}
		switch(pack->opcode)
		{
			case 0:
				break;

			case ServerOP_KeepAlive:
			{
				// ignore this
				break;
			}
			case ServerOP_ZAAuth:
			{
				_log(SOCKET_SERVER__ERROR, "Got authentication from Socket_Server_ when they are already authenticated.");
				break;
			}
			case ServerOP_LFGuildUpdate:
			{
				zoneserver_list.SendPacket(pack);
				break;
			}
			default:
			{
				_log(SOCKET_SERVER__ERROR, "Unknown ServerOPcode from Socket_Server_ 0x%04x, size %d", pack->opcode, pack->size);
				DumpPacket(pack->pBuffer, pack->size);
				break;
			}
		}

		delete pack;
	}
	return(true);
}

bool Socket_Server_Connection::SendPacket(ServerPacket* pack)
{
	if(!Stream)
		return false;

	return Stream->SendPacket(pack);
}

