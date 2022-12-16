#ifndef EQEMU_PLAYER_EVENT_DISCORD_FORMATTER_H
#define EQEMU_PLAYER_EVENT_DISCORD_FORMATTER_H

#include <string>
#include "player_events.h"
#include "../repositories/base/base_player_event_logs_repository.h"
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

struct DiscordField {
	std::string name;
	std::string value;
	bool        _inline;

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(name),
			CEREAL_NVP(value),
			cereal::make_nvp("inline", _inline)
		);
	}
};

struct DiscordEmbed {
	std::vector<DiscordField> fields;
	std::string               description;
	std::string               timestamp;

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(fields),
			CEREAL_NVP(description),
			CEREAL_NVP(timestamp)
		);
	}
};

struct DiscordEmbedRoot {
	std::vector<DiscordEmbed> embeds;

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(embeds)
		);
	}
};


class PlayerEventDiscordFormatter {
public:
	static std::string GetCurrentTimestamp();
	static std::string FormatEventSay(const PlayerEvent::PlayerEventContainer &c, const PlayerEvent::SayEvent &e);
	static void BuildDiscordField(
		std::vector<DiscordField> *f,
		const std::string &name,
		const std::string &value,
		bool is_inline = true
	);
	static void BuildBaseFields(std::vector<DiscordField> *f, const PlayerEvent::PlayerEventContainer &p);
	static void BuildBaseEmbed(
		std::vector<DiscordEmbed> *e,
		const std::vector<DiscordField> &f,
		PlayerEvent::PlayerEventContainer c
	);
	static std::string FormatWithNodata(const PlayerEvent::PlayerEventContainer &c);
	static std::string
	FormatMerchantPurchaseEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::MerchantPurchaseEvent &e
	);
};


#endif //EQEMU_PLAYER_EVENT_DISCORD_FORMATTER_H
