#ifndef EQEMU_DISCORD_MANAGER_H
#define EQEMU_DISCORD_MANAGER_H

#include <mutex>
#include <map>
#include <vector>
#include "../../common/types.h"
#include "../repositories/player_event_logs_repository.h"
#include "../events/player_events.h"

class DiscordManager {
public:
	void QueueWebhookMessage(uint32 webhook_id, const std::string& message);
	void ProcessMessageQueue();
	void QueuePlayerEventMessage(const PlayerEvent::PlayerEventContainer& e);

	static DiscordManager* Instance()
	{
		static DiscordManager instance;
		return &instance;
	}
private:
	std::mutex webhook_queue_lock{};
	std::map<uint32, std::vector<std::string>> webhook_message_queue{};
};


#endif
