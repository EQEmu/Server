#ifndef EQEMU_DISCORD_H
#define EQEMU_DISCORD_H


#include <string>

class Discord {
public:
	static void SendWebhookMessage(const std::string& message, const std::string& webhook_url);
};


#endif //EQEMU_DISCORD_H
