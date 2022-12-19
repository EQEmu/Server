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
	BuildDiscordField(&f, "Cost", fmt::format(":moneybag: {}", Strings::Money((e.cost / 1000), (e.cost / 100) % 10, (e.cost / 10) % 10, e.cost % 10)));
	BuildDiscordField(
		&f,
		"Player Balance",
		fmt::format(":moneybag: [{}] \n:gem: Currency [{}]", Strings::Commify(std::to_string(e.player_money_balance)), e.player_currency_balance)
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
	BuildDiscordField(&f, "Cost", fmt::format(":moneybag: {}", Strings::Money((e.cost / 1000), (e.cost / 100) % 10, (e.cost / 10) % 10, e.cost % 10)));
	BuildDiscordField(
		&f,
		"Player Balance",
		fmt::format(":moneybag: [{}] \n:gem: Currency [{}]", Strings::Commify(std::to_string(e.player_money_balance)), e.player_currency_balance)
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
		instance_version_info = fmt::format("Instance Version ({}) -> ({})", e.from_instance_version, e.to_instance_version);
	}

	std::vector<DiscordField> f = {};
	BuildBaseFields(&f, c);
	BuildDiscordField(
		&f, 
		"Zoning Information", 
		fmt::format("Zone ID ({}) -> ({}) \n{} \n{}",
		e.from_zone_id, e.to_zone_id, instance_id_info, instance_version_info)
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
		fmt::format("AA Gained ({})",
		e.aa_gained)
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
		fmt::format("AA ID ({}) \nAA Cost ({}) \n{}",
		e.aa_id, e.aa_cost, aa_info)
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
		fmt::format("Item ID ({}) \nItem Name ({})",
		e.item_id, e.item_name)
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
		fmt::format("{} ({}) \nCharges ({}) \nReason ({})",
		e.item_name, e.item_id, e.charges, e.reason)
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

