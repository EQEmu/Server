#ifndef EQEMU_DISCORD_H
#define EQEMU_DISCORD_H


#include <string>
#include "../types.h"

class Discord {
public:
	static void SendWebhookMessage(const std::string& message, const std::string& webhook_url);
	static std::string FormatDiscordMessage(uint16 category_id, const std::string& message);
};


#endif //EQEMU_DISCORD_H
