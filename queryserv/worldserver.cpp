#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/md5.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "../common/servertalk.h"
#include "../common/net/packet.h"

#include "database.h"
#include "lfguild.h"
#include "queryservconfig.h"
#include "worldserver.h"
#include "../common/events/player_events.h"
#include "../common/events/player_event_logs.h"
#include "../common/server_reload_types.h"
#include <iomanip>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern WorldServer           worldserver;
extern const queryservconfig *Config;
extern QSDatabase            qs_database;
extern LFGuildManager        lfguildmanager;

WorldServer::WorldServer()
{
}

WorldServer::~WorldServer()
{
}

void WorldServer::Connect()
{
	m_connection = std::make_unique<EQ::Net::ServertalkClient>(
		Config->WorldIP,
		Config->WorldTCPPort,
		false,
		"QueryServ",
		Config->SharedKey
	);
	m_connection->OnMessage(std::bind(&WorldServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
}

bool WorldServer::SendPacket(ServerPacket *pack)
{
	m_connection->SendPacket(pack);
	return true;
}

std::string WorldServer::GetIP() const
{
	return m_connection->Handle()->RemoteIP();
}

uint16 WorldServer::GetPort() const
{
	return m_connection->Handle()->RemotePort();
}

bool WorldServer::Connected() const
{
	return m_connection->Connected();
}

void WorldServer::HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	switch (opcode) {
		case 0: {
			break;
		}
		case ServerOP_ServerReloadRequest: {
			auto o = (ServerReload::Request*) p.Data();
			if (o->type == ServerReload::Type::Logs) {
				LogSys.LoadLogDatabaseSettings();
				player_event_logs.ReloadSettings();
			}

			break;
		}
		case ServerOP_QueryServGeneric: {
			/*
			The purpose of ServerOP_QueryServerGeneric is so that we don't have to add code to world just to relay packets
			each time we add functionality to queryserv.

			A ServerOP_QueryServGeneric packet has the following format:

			uint32 SourceZoneID
			uint32 SourceInstanceID
			char OriginatingCharacterName[0]
			- Null terminated name of the character this packet came from. This could be just
			- an empty string if it has no meaning in the context of a particular packet.
			uint32 Type

			The 'Type' field is a 'sub-opcode'. A value of 0 is used for the LFGuild packets. The next feature to be added
			to queryserv would use 1, etc.

			Obviously, any fields in the packet following the 'Type' will be unique to the particular type of packet. The
			'Generic' in the name of this ServerOP code relates to the four header fields.
			*/

			auto   from = p.GetCString(8);
			uint32 Type = p.GetUInt32(8 + from.length() + 1);

			switch (Type) {
				case QSG_LFGuild: {
					ServerPacket pack;
					pack.pBuffer = (uchar *) p.Data();
					pack.opcode  = opcode;
					pack.size    = (uint32) p.Length();
					lfguildmanager.HandlePacket(&pack);
					pack.pBuffer = nullptr;
					break;
				}
				default:
					LogInfo("Received unhandled ServerOP_QueryServGeneric", Type);
					break;
			}
			break;
		}
		case ServerOP_QSSendQuery: {
			/* Process all packets here */
			ServerPacket pack;
			pack.pBuffer = (uchar *) p.Data();
			pack.opcode  = opcode;
			pack.size    = (uint32) p.Length();

			qs_database.GeneralQueryReceive(&pack);
			pack.pBuffer = nullptr;
			break;
		}
		default:
			LogInfo("Unhandled opcode: {}", opcode);
			break;
	}
}
