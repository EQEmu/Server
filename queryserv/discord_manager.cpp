#include "discord_manager.h"
#include "../common/discord/discord.h"
#include "../common/eqemu_logsys.h"
#include "../common/string_util.h"

void DiscordManager::QueueWebhookMessage(uint32 webhook_id, const std::string &message)
{
	webhook_queue_lock.lock();
	webhook_message_queue[webhook_id].emplace_back(message);
	webhook_queue_lock.unlock();
}

constexpr int MAX_MESSAGE_LENGTH = 1900;

void DiscordManager::ProcessMessageQueue()
{
	if (webhook_message_queue.empty()) {
		return;
	}

	webhook_queue_lock.lock();

	for (auto &q: webhook_message_queue) {
		LogDiscord("Processing [{}] messages in queue for webhook ID [{}]...", q.second.size(), q.first);

		std::string messages = implode("\n", q.second);
		auto        webhook  = LogSys.discord_webhooks[q.first];
		std::string buffer;

		for (auto &message: q.second) {
			// next message would become too large
			bool next_message_too_large = ((int) message.length() + (int) buffer.length()) > MAX_MESSAGE_LENGTH;
			if (next_message_too_large) {
				Discord::SendWebhookMessage(
					buffer,
					webhook.webhook_url
				);
				buffer = "";
			}

			buffer += message;

			// one single message was too large
			// this should rarely happen but the message will need to be split
			if ((int) buffer.length() > MAX_MESSAGE_LENGTH) {
				for (unsigned mi = 0; mi < buffer.length(); mi += MAX_MESSAGE_LENGTH) {
					Discord::SendWebhookMessage(
						buffer.substr(mi, MAX_MESSAGE_LENGTH),
						webhook.webhook_url
					);
				}
				buffer = "";
			}
		}

		// final flush
		if (!buffer.empty()) {
			Discord::SendWebhookMessage(
				buffer,
				webhook.webhook_url
			);
		}

		webhook_message_queue.erase(q.first);
	}
	webhook_queue_lock.unlock();
}
