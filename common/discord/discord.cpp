#include "discord.h"
#include "../http/httplib.h"
#include "../json/json.h"
#include "../string_util.h"
#include "../eqemu_logsys.h"

constexpr int MAX_RETRIES = 10;

void Discord::SendWebhookMessage(const std::string &message, const std::string &webhook_url)
{
	// validate
	if (webhook_url.empty()) {
		LogDiscord("[webhook_url] is empty");
		return;
	}

	// validate
	if (webhook_url.find("http://") == std::string::npos && webhook_url.find("https://") == std::string::npos) {
		LogDiscord("[webhook_url] [{}] does not contain a valid http/s prefix.", webhook_url);
		return;
	}

	// split
	auto s = SplitString(webhook_url, '/');

	// url
	std::string base_url = fmt::format("{}//{}", s[0], s[2]);
	std::string endpoint = replace_string(webhook_url, base_url, "");

	// client
	httplib::Client cli(base_url.c_str());
	cli.set_connection_timeout(0, 15000000); // 15 sec
	cli.set_read_timeout(15, 0); // 15 seconds
	cli.set_write_timeout(15, 0); // 15 seconds
	httplib::Headers headers = {
		{"Content-Type", "application/json"}
	};

	// payload
	Json::Value p;
	p["content"] = message;
	std::stringstream payload;
	payload << p;

	bool retry       = true;
	int  retries     = 0;
	int  retry_timer = 1000;
	while (retry) {
		if (auto res = cli.Post(endpoint.c_str(), payload.str(), "application/json")) {
			if (res->status != 200 && res->status != 204) {
				LogError("[Discord Client] Code [{}] Error [{}]", res->status, res->body);
			}
			if (res->status == 429) {
				if (!res->body.empty()) {
					std::stringstream ss(res->body);
					Json::Value       response;

					try {
						ss >> response;
					}
					catch (std::exception const &ex) {
						LogDiscord("JSON serialization failure [{}] via [{}]", ex.what(), res->body);
					}

					retry_timer = std::stoi(response["retry_after"].asString()) + 500;
				}

				LogDiscord("Rate limited... retrying message in [{}ms]", retry_timer);
				std::this_thread::sleep_for(std::chrono::milliseconds(retry_timer + 500));
			}
			if (res->status == 204) {
				retry = false;
			}
			if (retries > MAX_RETRIES) {
				LogDiscord("Retries exceeded for message [{}]", message);
				retry = false;
			}

			retries++;
		}
	}
}

std::string Discord::FormatDiscordMessage(uint16 category_id, const std::string &message)
{
	if (category_id == Logs::LogCategory::MySQLQuery) {
		return fmt::format("```sql\n{}\n```", message);
	}

	return message + "\n";
}
