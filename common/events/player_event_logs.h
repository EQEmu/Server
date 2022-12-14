#ifndef EQEMU_PLAYER_EVENT_LOGS_H
#define EQEMU_PLAYER_EVENT_LOGS_H

#include "../repositories/player_event_log_settings_repository.h"
#include "player_events.h"
#include "../servertalk.h"
#include "../repositories/player_event_logs_repository.h"
#include <cereal/archives/json.hpp>

class PlayerEventLogs {
public:
	void Init();
	PlayerEventLogs *SetDatabase(Database *db);
	bool ValidateDatabaseConnection();
	bool IsEventEnabled(PlayerEvent::EventType event);

	// batch queue
	void ProcessBatchQueue();
	void AddToQueue(const PlayerEventLogsRepository::PlayerEventLogs &logs);

	// main event record generic function
	// can ingest any struct event types
	template<typename T>
	std::unique_ptr<ServerPacket> RecordEvent(
		PlayerEvent::EventType t,
		const PlayerEvent::PlayerEvent &p,
		T e
	)
	{
		auto n = PlayerEventLogsRepository::NewEntity();
		FillPlayerEvent(p, n);
		n.event_type_id = t;

		std::stringstream ss;
		{
			cereal::JSONOutputArchive ar(ss);
			e.serialize(ar);
		}

		n.event_data = Strings::Contains(ss.str(), "noop") ? "{}" : ss.str();
		n.created_at = std::time(nullptr);
		return BuildPlayerEventPacket(n);
	}

private:
	Database                                                 *m_database; // reference to database
	PlayerEventLogSettingsRepository::PlayerEventLogSettings m_settings[PlayerEvent::EventType::MAX]{};
	// batch queue is used to record events in batch
	std::vector<PlayerEventLogsRepository::PlayerEventLogs>  m_record_batch_queue{};
	static void FillPlayerEvent(const PlayerEvent::PlayerEvent &p, PlayerEventLogsRepository::PlayerEventLogs &n);
	static std::unique_ptr<ServerPacket>
	BuildPlayerEventPacket(const BasePlayerEventLogsRepository::PlayerEventLogs &e);
};

extern PlayerEventLogs player_event_logs;

#endif //EQEMU_PLAYER_EVENT_LOGS_H
