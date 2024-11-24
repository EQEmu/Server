
/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
#include "zoneserver.h"
#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/player_event_logs_repository.h"
#include "../common/events/player_event_logs.h"

ZoneServer::ZoneServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> in_connection, EQ::Net::ConsoleServer *in_console)
	: tcpc(in_connection) {

	tcpc->OnMessage(std::bind(&ZoneServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	console = in_console;
}

ZoneServer::~ZoneServer()
{
}

void ZoneServer::HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	auto pack = &tpack;

	switch (opcode) {
		case ServerOP_KeepAlive: {
			SendPacket(pack);
			break;
		}
		case ServerOP_PlayerEvent: {
			auto                         n = PlayerEvent::PlayerEventContainer{};
			auto                         s = reinterpret_cast<ServerSendPlayerEvent_Struct*>(pack->pBuffer);
			EQ::Util::MemoryStreamReader ss(s->cereal_data, s->cereal_size);
			cereal::BinaryInputArchive   archive(ss);
			archive(n);

			player_event_logs.AddToQueue(n.player_event_log);

			// if discord enabled for event, ship to UCS to process
			// if (player_event_logs.IsEventDiscordEnabled(n.player_event_log.event_type_id)) {
			// 	UCSLink.SendPacket(pack);
			// }
			break;
		}
		default: {
			LogInfo("Unknown ServerOP Received <red>[{}]", opcode);
			break;
		}
	}
}
