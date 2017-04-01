#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "queryserv.h"
#include "world_config.h"
#include "clientlist.h"
#include "zonelist.h"


#include "../common/md5.h"
#include "../common/emu_tcp_connection.h"
#include "../common/packet_dump.h"

extern ClientList client_list;
extern ZSList zoneserver_list;

QueryServConnection::QueryServConnection()
{
	Stream = 0;
	authenticated = false;
}

void QueryServConnection::SetConnection(EmuTCPConnection *inStream)
{
	if(Stream)
	{
		Log(Logs::Detail, Logs::QS_Server, "Incoming QueryServ Connection while we were already connected to a QueryServ.");
		Stream->Disconnect();
	}

	Stream = inStream;

	authenticated = false;
}

bool QueryServConnection::Process()
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
						Log(Logs::Detail, Logs::QS_Server, "QueryServ authorization failed.");
						auto pack = new ServerPacket(ServerOP_ZAAuthFailed);
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
					Log(Logs::Detail, Logs::QS_Server, "QueryServ authorization failed.");
					auto pack = new ServerPacket(ServerOP_ZAAuthFailed);
					SendPacket(pack);
					delete pack;
					Disconnect();
					return false;
				}
			}
			else
			{
				Log(Logs::Detail, Logs::QS_Server,"**WARNING** You have not configured a world shared key in your config file. You should add a <key>STRING</key> element to your <world> element to prevent unauthroized zone access.");
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
				Log(Logs::Detail, Logs::QS_Server, "Got authentication from QueryServ when they are already authenticated.");
				break;
			}
			case ServerOP_QueryServGeneric:
			{
				uint32 ZoneID = pack->ReadUInt32();
				uint16 InstanceID = pack->ReadUInt32();
				zoneserver_list.SendPacket(ZoneID, InstanceID, pack);
				break;
			}
			case ServerOP_LFGuildUpdate:
			{
				zoneserver_list.SendPacket(pack);
				break;
			}
			default:
			{
				Log(Logs::Detail, Logs::QS_Server, "Unknown ServerOPcode from QueryServ 0x%04x, size %d", pack->opcode, pack->size);
				DumpPacket(pack->pBuffer, pack->size);
				break;
			}
		}

		delete pack;
	}
	return(true);
}

bool QueryServConnection::SendPacket(ServerPacket* pack)
{
	if(!Stream)
		return false;

	return Stream->SendPacket(pack);
}

