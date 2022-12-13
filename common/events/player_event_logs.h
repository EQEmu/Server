#ifndef EQEMU_PLAYER_EVENT_LOGS_H
#define EQEMU_PLAYER_EVENT_LOGS_H

#include "../repositories/player_event_log_settings_repository.h"
#include "player_events.h"

class PlayerEventLogs {
public:
	void Init();
	PlayerEventLogs *SetDatabase(Database *db);
	bool ValidateDatabaseConnection();
	bool IsEventEnabled(PlayerEvent::Event event);
	void RecordGMCommandEvent(const PlayerEvent::PlayerEvent& p, PlayerEvent::GMCommandEvent e);
private:
	Database *m_database; // reference to database
	PlayerEventLogSettingsRepository::PlayerEventLogSettings m_settings[PlayerEvent::Event::MAX]{};
};

extern PlayerEventLogs player_event_logs;

#endif //EQEMU_PLAYER_EVENT_LOGS_H
