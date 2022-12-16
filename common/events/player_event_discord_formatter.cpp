#include "player_event_discord_formatter.h"
#include "../repositories/player_event_logs_repository.h"
#include <vector>
#include <fmt/format.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

std::string PlayerEventDiscordFormatter::FormatEventSay(
	const PlayerEventLogsRepository::PlayerEventLogs &p,
	const PlayerEvent::SayEvent &e
)
{
	std::vector<DiscordField> fields = {};
	fields.emplace_back(
		DiscordField{
			.name = "Event",
			.value = PlayerEvent::EventName[p.event_type_id],
			._inline = false,
		}
	);
	fields.emplace_back(
		DiscordField{
			.name = "Character",
			.value = PlayerEvent::EventName[p.event_type_id],
			._inline = false,
		}
	);

	std::vector<DiscordEmbed> embeds = {};
	embeds.emplace_back(
		DiscordEmbed{
			.fields = fields,
			.description = "",
			.timestamp = GetCurrentTimestamp()
		}
	);

	DiscordEmbedRoot root = DiscordEmbedRoot{
		.embeds = embeds
	};

	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::GetCurrentTimestamp()
{
	time_t now;
	time(&now);
	char buf[sizeof "2011-10-08T07:07:09Z"];
	strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
	std::string timestamp = buf;
	return timestamp;
}
