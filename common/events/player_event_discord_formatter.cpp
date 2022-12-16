#include "player_event_discord_formatter.h"
#include "../repositories/character_data_repository.h"
#include <vector>
#include <fmt/format.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

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

void PlayerEventDiscordFormatter::BuildBaseFields(
	std::vector<DiscordField> *f,
	const PlayerEvent::PlayerEventContainer &p
)
{
	std::string guild_info;
	if (!p.player_event.guild_name.empty()) {
		guild_info = fmt::format(":shield: Guild [{}] ({})", p.player_event.guild_name, p.player_event.guild_id);
	}

	f->emplace_back(
		BuildDiscordField(
			"Character",
			fmt::format(
				":scales: {} ({}) {}",
				p.player_event.character_name,
				p.player_event.character_id,
				guild_info
			)
		)
	);

	std::string instance_info;
	if (p.player_event.instance_id > 0) {
		instance_info = fmt::format("Instance ID [{}]", p.player_event.instance_id);
	}

	f->emplace_back(
		BuildDiscordField(
			"Zone",
			fmt::format(
				":map: [{}] ({}) ({}) {}",
				p.player_event.zone_long_name,
				p.player_event.zone_short_name,
				p.player_event.zone_id,
				instance_info
			)
		)
	);
}

void PlayerEventDiscordFormatter::BuildBaseEmbed(
	std::vector<DiscordEmbed> *e,
	const std::vector<DiscordField> &f,
	const PlayerEvent::PlayerEventContainer c
)
{
	e->emplace_back(
		DiscordEmbed{
			.fields = f,
			.description = fmt::format(
				":small_blue_diamond: [Player Event] {}",
				PlayerEvent::EventName[c.player_event_log.event_type_id]
			),
			.timestamp = GetCurrentTimestamp()
		}
	);
}

std::string PlayerEventDiscordFormatter::FormatEventSay(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::SayEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	f.emplace_back(BuildDiscordField("Message", e.message));
	if (!e.target.empty()) {
		f.emplace_back(BuildDiscordField("Target", e.target));
	}

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);

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

std::string PlayerEventDiscordFormatter::FormatWithNodata(const PlayerEvent::PlayerEventContainer &c)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
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
