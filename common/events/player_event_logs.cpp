#include <cereal/archives/json.hpp>
#include "player_event_logs.h"
#include "../repositories/player_event_logs_repository.h"
#include "../net/packet.h"
#include "../servertalk.h"
#include "../timer.h"

void PlayerEventLogs::Init()
{
	ValidateDatabaseConnection();

	// initialize settings array
	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		m_settings[i].id                 = i;
		m_settings[i].event_name         = PlayerEvent::EventName[i];
		m_settings[i].event_enabled      = 0;
		m_settings[i].retention_days     = 0;
		m_settings[i].discord_webhook_id = 0;
	}

	// initialize settings from database
	auto             s = PlayerEventLogSettingsRepository::All(*m_database);
	std::vector<int> db_settings{};
	db_settings.reserve(s.size());
	for (auto &e: s) {
		m_settings[e.id] = e;
		db_settings.emplace_back(e.id);
	}

	// insert entries that don't exist in database
	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		bool is_missing_in_database = std::find(db_settings.begin(), db_settings.end(), i) == db_settings.end();
		if (is_missing_in_database) {
			LogInfo(
				"[New] PlayerEvent [{}] ({})",
				PlayerEvent::EventName[i],
				i
			);

			auto c = PlayerEventLogSettingsRepository::NewEntity();
			c.id         = i;
			c.event_name = PlayerEvent::EventName[i];
			PlayerEventLogSettingsRepository::InsertOne(*m_database, c);
		}
	}
}

PlayerEventLogs *PlayerEventLogs::SetDatabase(Database *db)
{
	m_database = db;

	return this;
}

bool PlayerEventLogs::ValidateDatabaseConnection()
{
	if (!m_database) {
		LogError("[PlayerEventLogs::ValidateDatabaseConnection] No database connection");
		return false;
	}

	return true;
}

bool PlayerEventLogs::IsEventEnabled(PlayerEvent::Event event)
{
	return m_settings[event].event_enabled ? m_settings[event].event_enabled : false;
}

std::unique_ptr<ServerPacket> PlayerEventLogs::RecordGMCommandEvent(
	const PlayerEvent::PlayerEvent &p,
	PlayerEvent::GMCommandEvent e
)
{
	auto n = PlayerEventLogsRepository::NewEntity();
	n.account_id    = p.account_id;
	n.character_id  = p.character_id;
	n.zone_id       = p.zone_id;
	n.instance_id   = p.instance_id;
	n.x             = p.x;
	n.y             = p.y;
	n.z             = p.z;
	n.heading       = p.heading;
	n.event_type_id = PlayerEvent::GM_COMMAND;

	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		e.serialize(ar);
	}

	std::string output = ss.str();
	output = Strings::Replace(output, "	", "");
	output = Strings::Replace(output, "    ", "");
	output = Strings::Replace(output, "\n", "");

	n.event_data = output;
	n.created_at = std::time(nullptr);

	// return packet
	EQ::Net::DynamicPacket dyn_pack;
	dyn_pack.PutSerialize(0, n);
	auto pack_size = sizeof(ServerSendPlayerEvent_Struct) + dyn_pack.Length();
	auto pack      = std::make_unique<ServerPacket>(ServerOP_PlayerEvent, static_cast<uint32_t>(pack_size));
	auto buf       = reinterpret_cast<ServerSendPlayerEvent_Struct *>(pack->pBuffer);
	buf->cereal_size = static_cast<uint32_t>(dyn_pack.Length());
	memcpy(buf->cereal_data, dyn_pack.Data(), dyn_pack.Length());

	return pack;
}

void PlayerEventLogs::ProcessBatchQueue()
{
	if (m_record_batch_queue.empty()) {
		return;
	}

	BenchTimer benchmark;

	// flush many
	PlayerEventLogsRepository::InsertMany(*m_database, m_record_batch_queue);
	LogInfo(
		"Processing batch player event log queue of [{}] took [{}]",
		m_record_batch_queue.size(),
		benchmark.elapsed()
	);

	// empty
	m_record_batch_queue = {};
}

void PlayerEventLogs::AddToQueue(const PlayerEventLogsRepository::PlayerEventLogs& log)
{
	m_record_batch_queue.emplace_back(log);
}

