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

void PlayerEventDiscordFormatter::BuildDiscordField(
	std::vector<DiscordField> *f,
	const std::string &name,
	const std::string &value,
	bool is_inline
)
{
	if (value.empty()) {
		return;
	}

	f->emplace_back(
		DiscordField{
			.name = name,
			.value = value,
			.is_inline = is_inline,
		}
	);
}

void PlayerEventDiscordFormatter::BuildBaseFields(
	std::vector<DiscordField> *f,
	const PlayerEvent::PlayerEventContainer &p
)
{
	std::string guild_info;
	if (!p.player_event.guild_name.empty()) {
		guild_info = fmt::format("\n:shield: Guild [{}] ({})", p.player_event.guild_name, p.player_event.guild_id);
	}

	BuildDiscordField(
		f, "Character",
		fmt::format(
			":scales: {} ({}) {}",
			p.player_event.character_name,
			p.player_event.character_id,
			guild_info
		)
	);

	std::string instance_info;
	if (p.player_event.instance_id > 0) {
		instance_info = fmt::format("Instance ID [{}]", p.player_event.instance_id);
	}

	BuildDiscordField(
		f, "Zone", fmt::format(
			":map: [{}] ({}) ({}) {}",
			p.player_event.zone_long_name,
			p.player_event.zone_short_name,
			p.player_event.zone_id,
			instance_info
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
	BuildDiscordField(&f, "Message", e.message);
	BuildDiscordField(&f, "Target", e.target);

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
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatMerchantPurchaseEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::MerchantPurchaseEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(&f, "Merchant", fmt::format("{} ({}) NPC ID ({})", e.merchant_name, e.merchant_type, e.npc_id));
	BuildDiscordField(&f, "Item", fmt::format("{} ({}) x({})", e.item_name, e.item_id, e.charges));
	BuildDiscordField(
		&f,
		"Cost",
		fmt::format(
			":moneybag: {}",
			Strings::Money((e.cost / 1000), (e.cost / 100) % 10, (e.cost / 10) % 10, e.cost % 10)));
	BuildDiscordField(
		&f,
		"Player Balance",
		fmt::format(
			":moneybag: [{}] \n:gem: Currency [{}]",
			Strings::Commify(std::to_string(e.player_money_balance)),
			e.player_currency_balance
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatMerchantSellEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::MerchantSellEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(&f, "Merchant", fmt::format("{} ({}) NPC ID ({})", e.merchant_name, e.merchant_type, e.npc_id));
	BuildDiscordField(&f, "Item", fmt::format("{} ({}) x({})", e.item_name, e.item_id, e.charges));
	BuildDiscordField(
		&f,
		"Cost",
		fmt::format(
			":moneybag: {}",
			Strings::Money((e.cost / 1000), (e.cost / 100) % 10, (e.cost / 10) % 10, e.cost % 10)));
	BuildDiscordField(
		&f,
		"Player Balance",
		fmt::format(
			":moneybag: [{}] \n:gem: Currency [{}]",
			Strings::Commify(std::to_string(e.player_money_balance)),
			e.player_currency_balance
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatZoningEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::ZoningEvent &e
)
{
	std::string instance_id_info;
	std::string instance_version_info;

	if (e.to_instance_id > 0 || e.from_instance_id > 0) {
		instance_id_info = fmt::format("Instance ID ({}) -> ({})", e.from_instance_id, e.to_instance_id);
	}

	if (e.from_instance_version > 0 || e.to_instance_version > 0) {
		instance_version_info = fmt::format(
			"Instance Version ({}) -> ({})",
			e.from_instance_version,
			e.to_instance_version
		);
	}

	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Zoning Information",
		fmt::format(
			"Zone ID ({}) -> ({}) \n{} \n{}",
			e.from_zone_id, e.to_zone_id, instance_id_info, instance_version_info
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatAAGainedEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::AAGainedEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Points Gained",
		fmt::format(
			"AA Gained ({})",
			e.aa_gained
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatAAPurchasedEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::AAPurchasedEvent &e
)
{
	std::string aa_info;
	if (e.aa_previous_id != -1 || e.aa_next_id != -1) {
		aa_info = fmt::format("AA Previous ID ({}) \nAA Next ID ({})", e.aa_previous_id, e.aa_next_id);
	}

	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"AA Purchased",
		fmt::format(
			"AA ID ({}) \nAA Cost ({}) \n{}",
			e.aa_id, e.aa_cost, aa_info
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatForageSuccessEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::ForageSuccessEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Foraged Item",
		fmt::format(
			"Item ID ({}) \nItem Name ({})",
			e.item_id, e.item_name
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatDestroyItemEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::DestroyItemEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Destroyed Item",
		fmt::format(
			"{} ({}) \nCharges ({}) \nReason ({})",
			e.item_name, e.item_id, e.charges, e.reason
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatLevelGainedEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::LevelGainedEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Levels Gained",
		fmt::format(
			"(+{})",
			e.levels_gained
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatLevelLostEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::LevelLostEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Levels Lost",
		fmt::format(
			"(-{})",
			e.levels_lost
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatLootItemEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::LootItemEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Looted Item",
		fmt::format(
			"{} ({}) \nCharges: {} \nCorpse: {}",
			e.item_name, e.item_id, e.charges, e.corpse_name
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}


std::string PlayerEventDiscordFormatter::FormatGroundSpawnPickupEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::GroundSpawnPickupEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Picked Up Item",
		fmt::format(
			"{} ({})",
			e.item_name, e.item_id
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatSkillUpEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::SkillUpEvent &e
)
{
	std::string target_info;
	if (e.against_who.length()) {
		if (e.against_who == c.player_event.character_name) {
			target_info = fmt::format("Target: Self");
		}
		else {
			target_info = fmt::format("Target: {}", e.against_who);
		}
	}

	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Skill Information",
		fmt::format(
			"Skill: {} \nLevel: ({}/{}) \n{}",
			e.skill_id, e.value, e.max_skill, target_info
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTaskAcceptEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TaskAcceptEvent &e
)
{

	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Task Information",
		fmt::format(
			"{} ({}) \n {} ({})",
			e.task_name, e.task_id, e.npc_name, e.npc_id
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTaskCompleteEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TaskCompleteEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Task Information",
		fmt::format(
			"{} ({}) \nActivity ID ({}) \nDone ({})",
			e.task_name, e.task_id, e.activity_id, e.done_count
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTaskUpdateEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TaskUpdateEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Task Information",
		fmt::format(
			"{} ({}) \nActivity ID ({}) \nDone ({})",
			e.task_name, e.task_id, e.activity_id, e.done_count
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatResurrectAcceptEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::ResurrectAcceptEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Resurrect Information",
		fmt::format(
			"From: {} \nSpell ID ({})",
			e.resurrecter_name, e.spell_id
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatCombineEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::CombineEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Combine Information",
		fmt::format(
			"{} ({}) \n Made ({})",
			e.recipe_name, e.recipe_id, e.made_count
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatFishSuccessEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::FishSuccessEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Fishing Information",
		fmt::format(
			"{} ({})",
			e.item_name, e.item_id
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatDeathEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::DeathEvent &e
)
{
	std::string killer_info;
	if (e.killer_id) {
		killer_info = fmt::format("Killer: {} ({})", e.killer_name, e.killer_id);
	}

	std::string spell_info;
	if (e.spell_id) {
		spell_info = fmt::format("Spell: {} ({})", e.spell_name, e.spell_id);
	}
	
	std::string skill_info;
	if (e.skill_id) {
		skill_info = fmt::format("Skill: {} ({})", e.skill_name, e.skill_id);
	}

	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Death Information",
		fmt::format(
			"{} ({}) \nDamage: {} \n {} \n {}",
			killer_info, e.damage, spell_info, skill_info
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTradeItem(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TradeItem &e
)
{
	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f,
		"Trade Information",
		fmt::format(
			"{} ({}) \nSlot ({})",
			e.item_name, e.item_id, e.slot
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	DiscordEmbedRoot  root = DiscordEmbedRoot{
		.embeds = embeds
	};
	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}
