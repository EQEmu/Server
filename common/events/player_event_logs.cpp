#include <cereal/archives/json.hpp>
#include "player_event_logs.h"
#include "../repositories/player_event_logs_repository.h"

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

void PlayerEventLogs::RecordGMCommandEvent(const PlayerEvent::PlayerEvent &p, PlayerEvent::GMCommandEvent e)
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

	std::cout << output << std::endl;

	n.event_data = output;
	n.created_at = std::time(nullptr);

	PlayerEventLogsRepository::InsertOne(*m_database, n);
}

