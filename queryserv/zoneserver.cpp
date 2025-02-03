#include "zoneserver.h"
#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/player_event_logs_repository.h"
#include "../common/events/player_event_logs.h"

ZoneServer::ZoneServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> in_connection, EQ::Net::ConsoleServer *in_console)
	: tcpc(in_connection) {

	tcpc->OnMessage(std::bind(&ZoneServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	console    = in_console;
	keep_alive = std::make_unique<Timer>(5123);
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
			SendKeepAlive();

			LogInfo("ServerOP_KeepAlive Received.  Remaining Time {}.  Check? {}", keep_alive->GetRemainingTime(), keep_alive->Check());
			keep_alive->Start(5000);
			LogInfo("ServerOP_KeepAlive Received2.  Remaining Time {}", keep_alive->GetRemainingTime());
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

void ZoneServer::SendKeepAlive()
{
	ServerPacket pack(ServerOP_KeepAlive, 0);
	SendPacket(&pack);
}
