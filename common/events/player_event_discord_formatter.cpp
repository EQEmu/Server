#include "player_event_discord_formatter.h"
#include "../repositories/player_event_logs_repository.h"
#include "../repositories/character_data_repository.h"
#include <vector>
#include <fmt/format.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

std::string PlayerEventDiscordFormatter::FormatEventSay(
	const PlayerEventLogsRepository::PlayerEventLogs &p,
	const PlayerEvent::SayEvent &e
)
{
	std::vector<DiscordField> f = {};
//	f.emplace_back(BuildDiscordField("Event", PlayerEvent::EventName[p.event_type_id]));
	f.emplace_back(BuildDiscordField("Character", fmt::format("{}", p.character_id)));
	f.emplace_back(BuildDiscordField("Message", e.message));
	if (!e.target.empty()) {
		f.emplace_back(BuildDiscordField("Target", e.target));
	}

	std::vector<DiscordEmbed> embeds = {};
	embeds.emplace_back(
		DiscordEmbed{
			.fields = f,
			.description = fmt::format("[Player Event] {}", PlayerEvent::EventName[p.event_type_id]),
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

DiscordField PlayerEventDiscordFormatter::BuildDiscordField(
	const std::string &name,
	const std::string &value,
	bool is_inline
)
{
	return DiscordField{
		.name = name,
		.value = value,
		._inline = is_inline,
	};
}
