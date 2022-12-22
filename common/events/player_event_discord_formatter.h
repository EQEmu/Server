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
	bool        is_inline;

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(name),
			CEREAL_NVP(value),
			cereal::make_nvp("inline", is_inline)
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

	static std::string FormatAAGainedEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::AAGainedEvent &e
	);
	static std::string FormatAAPurchasedEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::AAPurchasedEvent &e
	);
	static std::string FormatDeathEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::DeathEvent &e
	);
	static std::string FormatFishSuccessEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::FishSuccessEvent &e
	);
	static std::string FormatForageSuccessEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::ForageSuccessEvent &e
	);
	static std::string FormatDestroyItemEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::DestroyItemEvent &e
	);
	static std::string FormatLevelGainedEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::LevelGainedEvent &e
	);
	static std::string FormatLevelLostEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::LevelLostEvent &e
	);
	static std::string FormatLootItemEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::LootItemEvent &e
	);
	static std::string FormatGroundSpawnPickupEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::GroundSpawnPickupEvent &e
	);
	static std::string FormatMerchantPurchaseEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::MerchantPurchaseEvent &e
	);
	static std::string FormatMerchantSellEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::MerchantSellEvent &e
	);
	static std::string FormatNPCHandinEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::HandinEvent &e
	);
	static std::string FormatSkillUpEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::SkillUpEvent &e
	);
	static std::string FormatTaskAcceptEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TaskAcceptEvent &e
	);
	static std::string FormatTaskCompleteEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TaskCompleteEvent &e
	);
	static std::string FormatTaskUpdateEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::TaskUpdateEvent &e
	);
	static std::string FormatResurrectAcceptEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::ResurrectAcceptEvent &e
	);
	static std::string FormatCombineEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::CombineEvent &e
	);
	static std::string FormatZoningEvent(
		const PlayerEvent::PlayerEventContainer &c,
		const PlayerEvent::ZoningEvent &e
	);
};


#endif //EQEMU_PLAYER_EVENT_DISCORD_FORMATTER_H
