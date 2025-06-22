#ifndef EQEMU_DISCORD_H
#define EQEMU_DISCORD_H


#include <string>
#include "../types.h"
#include "../repositories/player_event_logs_repository.h"
#include "../events/player_events.h"

class Discord {
public:
	static void SendWebhookMessage(const std::string& message, const std::string& webhook_url);
	static std::string FormatDiscordMessage(uint16 category_id, const std::string& message);
	static void SendPlayerEventMessage(const PlayerEvent::PlayerEventContainer& e, const std::string &webhook_url);
	static bool ValidateWebhookUrl(const std::string &webhook_url);
};


#endif //EQEMU_DISCORD_H
