#ifndef EQEMU_PLAYER_EVENT_LOGS_H
#define EQEMU_PLAYER_EVENT_LOGS_H

#include "../repositories/player_event_log_settings_repository.h"
#include "player_events.h"
#include "../servertalk.h"
#include "../repositories/player_event_logs_repository.h"

class PlayerEventLogs {
public:
	void Init();
	PlayerEventLogs *SetDatabase(Database *db);
	bool ValidateDatabaseConnection();
	bool IsEventEnabled(PlayerEvent::Event event);
	std::unique_ptr<ServerPacket>
	RecordGMCommandEvent(const PlayerEvent::PlayerEvent &p, PlayerEvent::GMCommandEvent e);

	// batch queue
	void ProcessBatchQueue();
	void AddToQueue(const PlayerEventLogsRepository::PlayerEventLogs& logs);
private:
	Database                                                 *m_database; // reference to database
	PlayerEventLogSettingsRepository::PlayerEventLogSettings m_settings[PlayerEvent::Event::MAX]{};
	// batch queue is used to record events in batch
	std::vector<PlayerEventLogsRepository::PlayerEventLogs> m_record_batch_queue{};
};

extern PlayerEventLogs player_event_logs;

#endif //EQEMU_PLAYER_EVENT_LOGS_H
