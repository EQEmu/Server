#include "../common/debug.h"
#include "web_interface.h"
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

WebInterfaceConnection::WebInterfaceConnection()
{
	stream = 0;
	authenticated = false;
}

void WebInterfaceConnection::SetConnection(EmuTCPConnection *inStream)
{
	if(stream)
	{
		_log(WEB_INTERFACE__ERROR, "Incoming WebInterface Connection while we were already connected to a WebInterface.");
		stream->Disconnect();
	}

	stream = inStream;

	authenticated = false;
}

bool WebInterfaceConnection::Process()
{
	if (!stream || !stream->Connected())
		return false;

	ServerPacket *pack = 0;

	while((pack = stream->PopPacket()))
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
						_log(WEB_INTERFACE__ERROR, "WebInterface authorization failed.");
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
					_log(WEB_INTERFACE__ERROR, "WebInterface authorization failed.");
					ServerPacket* pack = new ServerPacket(ServerOP_ZAAuthFailed);
					SendPacket(pack);
					delete pack;
					Disconnect();
					return false;
				}
			}
			else
			{
				_log(WEB_INTERFACE__ERROR, "**WARNING** You have not configured a world shared key in your config file. You should add a <key>STRING</key> element to your <world> element to prevent unauthorized zone access.");
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
				_log(WEB_INTERFACE__ERROR, "Got authentication from WebInterface when they are already authenticated.");
				break;
			}
			default:
			{
				_log(WEB_INTERFACE__ERROR, "Unknown ServerOPcode from WebInterface 0x%04x, size %d", pack->opcode, pack->size);
				DumpPacket(pack->pBuffer, pack->size);
				break;
			}
		}

		delete pack;
	}
	return(true);
}

bool WebInterfaceConnection::SendPacket(ServerPacket* pack)
{
	if(!stream)
		return false;

	return stream->SendPacket(pack);
}

