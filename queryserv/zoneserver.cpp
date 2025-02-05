#include "zoneserver.h"
#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/player_event_logs_repository.h"
#include "../common/events/player_event_logs.h"
#include "../common/discord/discord_manager.h"

extern DiscordManager discord_manager;

ZoneServer::ZoneServer(
	std::shared_ptr<EQ::Net::ServertalkServerConnection> in_connection,
	EQ::Net::ConsoleServer *in_console
)
	: m_connection(in_connection)
{

	m_connection->OnMessage(std::bind(&ZoneServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	m_console = in_console;
}

ZoneServer::~ZoneServer()
{
}

void ZoneServer::HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	auto         pack = &tpack;

	switch (opcode) {
		case ServerOP_PlayerEvent: {
			auto n = PlayerEvent::PlayerEventContainer{};
			auto s = reinterpret_cast<ServerSendPlayerEvent_Struct *>(pack->pBuffer);
			EQ::Util::MemoryStreamReader ss(s->cereal_data, s->cereal_size);
			cereal::BinaryInputArchive archive(ss);
			archive(n);

			player_event_logs.AddToQueue(n.player_event_log);

			discord_manager.QueuePlayerEventMessage(n);
			break;
		}
		default: {
			LogInfo("Unknown ServerOP Received [{}]", opcode);
			break;
		}
	}
}
