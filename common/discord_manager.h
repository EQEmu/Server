#ifndef EQEMU_DISCORD_MANAGER_H
#define EQEMU_DISCORD_MANAGER_H

#include <mutex>
#include <map>
#include <vector>
#include "../common/types.h"

class DiscordManager {
public:
	void QueueWebhookMessage(uint32 webhook_id, const std::string& message);
	void ProcessMessageQueue();
private:
	std::mutex webhook_queue_lock{};
	std::map<uint32, std::vector<std::string>> webhook_message_queue{};
};


#endif
