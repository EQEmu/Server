#ifndef EQEMU_PLAYER_EVENT_LOGS_H
#define EQEMU_PLAYER_EVENT_LOGS_H

#include "../repositories/player_event_log_settings_repository.h"
#include "player_events.h"
#include "../servertalk.h"
#include "../repositories/player_event_logs_repository.h"
#include "../timer.h"
#include <cereal/archives/json.hpp>

class PlayerEventLogs {
public:
	void Init();
	PlayerEventLogs *SetDatabase(Database *db);
	bool ValidateDatabaseConnection();
	bool IsEventEnabled(PlayerEvent::EventType event);

	void Process();

	// batch queue
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

		n.event_type_name = PlayerEvent::EventName[t];
		n.event_data      = Strings::Contains(ss.str(), "noop") ? "{}" : ss.str();
		n.created_at      = std::time(nullptr);

		auto c = PlayerEvent::PlayerEventContainer{
			.player_event = p,
			.player_event_log = n
		};

		return BuildPlayerEventPacket(c);
	}

	[[nodiscard]] const PlayerEventLogSettingsRepository::PlayerEventLogSettings *GetSettings() const;
	bool IsEventDiscordEnabled(int32_t event_type_id);
	std::string GetDiscordWebhookUrlFromEventType(int32_t event_type_id);

	static std::string GetDiscordPayloadFromEvent(const PlayerEvent::PlayerEventContainer &e);
private:
	Database                                                 *m_database; // reference to database
	PlayerEventLogSettingsRepository::PlayerEventLogSettings m_settings[PlayerEvent::EventType::MAX]{};

	// batch queue is used to record events in batch
	std::vector<PlayerEventLogsRepository::PlayerEventLogs> m_record_batch_queue{};
	static void FillPlayerEvent(const PlayerEvent::PlayerEvent &p, PlayerEventLogsRepository::PlayerEventLogs &n);
	static std::unique_ptr<ServerPacket>
	BuildPlayerEventPacket(const PlayerEvent::PlayerEventContainer &e);

	// timers
	Timer m_process_batch_events_timer; // events processing timer
	Timer m_process_retention_truncation_timer; // timer for truncating events based on retention settings

	// processing
	void ProcessBatchQueue();
	void ProcessRetentionTruncation();
};

extern PlayerEventLogs player_event_logs;

#endif //EQEMU_PLAYER_EVENT_LOGS_H
