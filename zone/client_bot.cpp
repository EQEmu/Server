#include "bot.h"
#include "client.h"

bool Client::GetBotOption(BotOwnerOption boo) const {
	if (boo < _booCount) {
		return bot_owner_options[boo];
	}

	return false;
}

void Client::SetBotOption(BotOwnerOption boo, bool flag) {
	if (boo < _booCount) {
		bot_owner_options[boo] = flag;
	}
}

uint32 Client::GetBotCreationLimit(uint8 class_id) {
	uint32 bot_creation_limit = RuleI(Bots, CreationLimit);

	if (Admin() >= RuleI(Bots, MinStatusToBypassCreateLimit)) {
		return RuleI(Bots, MinStatusBypassCreateLimit);
	}

	const auto bucket_name = fmt::format(
		"bot_creation_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	auto bucket_value = GetBucket(bucket_name);
	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_creation_limit = Strings::ToUnsignedInt(bucket_value);
	}

	return bot_creation_limit;
}

int Client::GetBotRequiredLevel(uint8 class_id) {
	int bot_character_level = RuleI(Bots, BotCharacterLevel);

	const auto bucket_name = fmt::format(
		"bot_required_level{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	auto bucket_value = GetBucket(bucket_name);
	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_character_level = Strings::ToInt(bucket_value);
	}

	return bot_character_level;
}

int Client::GetBotSpawnLimit(uint8 class_id) {
	int bot_spawn_limit = RuleI(Bots, SpawnLimit);

	if (Admin() >= RuleI(Bots, MinStatusToBypassSpawnLimit)) {
		return RuleI(Bots, MinStatusBypassSpawnLimit);
	}

	const auto bucket_name = fmt::format(
		"bot_spawn_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	auto bucket_value = GetBucket(bucket_name);

	if (class_id && !bot_spawn_limit && bucket_value.empty()) {
		const auto new_bucket_name = "bot_spawn_limit";

		bucket_value = GetBucket(new_bucket_name);

		if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
			bot_spawn_limit = Strings::ToInt(bucket_value);

			return bot_spawn_limit;
		}
	}

	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_spawn_limit = Strings::ToInt(bucket_value);
	}

	if (RuleB(Bots, QuestableSpawnLimit)) {
		const auto query = fmt::format(
			"SELECT `value` FROM `quest_globals` WHERE `name` = '{}' AND `charid` = {} LIMIT 1",
			bucket_name,
			CharacterID()
		);

		auto results = database.QueryDatabase(query); // use 'database' for non-bot table calls

		if (!results.Success() || !results.RowCount()) {
			return bot_spawn_limit;
		}

		auto row = results.begin();
		bot_spawn_limit = Strings::ToInt(row[0]);
	}

	const auto& zones_list = Strings::Split(RuleS(Bots, ZonesWithSpawnLimits), ",");
	const auto& zones_limits_list = Strings::Split(RuleS(Bots, ZoneSpawnLimits), ",");
	int i = 0;

	for (const auto& result : zones_list) {
		try {
			if (
				std::stoul(result) == zone->GetZoneID() &&
				std::stoul(zones_limits_list[i]) < bot_spawn_limit
			) {
				bot_spawn_limit = std::stoul(zones_limits_list[i]);

				break;
			}

			++i;
		}

		catch (const std::exception& e) {
			LogInfo("Invalid entry in Rule VegasScaling:SpecialScalingZones or SpecialScalingZonesVersions: [{}]", e.what());
		}
	}

	const auto& zones_forced_list = Strings::Split(RuleS(Bots, ZonesWithForcedSpawnLimits), ",");
	const auto& zones_forced_limits_list = Strings::Split(RuleS(Bots, ZoneForcedSpawnLimits), ",");
	i = 0;

	for (const auto& result : zones_forced_list) {
		try {
			if (
				std::stoul(result) == zone->GetZoneID() &&
				std::stoul(zones_forced_limits_list[i]) != bot_spawn_limit
			) {
				bot_spawn_limit = std::stoul(zones_forced_limits_list[i]);

				break;
			}

			++i;
		}

		catch (const std::exception& e) {
			LogInfo("Invalid entry in Rule VegasScaling:SpecialScalingZones or SpecialScalingZonesVersions: [{}]", e.what());
		}
	}

	return bot_spawn_limit;
}

void Client::SetBotCreationLimit(uint32 new_creation_limit, uint8 class_id) {
	const auto bucket_name = fmt::format(
		"bot_creation_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_creation_limit));
}

void Client::SetBotRequiredLevel(int new_required_level, uint8 class_id) {
	const auto bucket_name = fmt::format(
		"bot_required_level{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_required_level));
}

void Client::SetBotSpawnLimit(int new_spawn_limit, uint8 class_id) {
	const auto bucket_name = fmt::format(
		"bot_spawn_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_spawn_limit));
}

void Client::CampAllBots(uint8 class_id) {
	Bot::BotOrderCampAll(this, class_id);
}

void Client::LoadDefaultBotSettings() {
	m_bot_spell_settings.clear();

	// Only illusion block supported currently
	SetBotSetting(BotSettingCategories::BaseSetting, BotBaseSettings::IllusionBlock, GetDefaultBotSettings(BotSettingCategories::BaseSetting, BotBaseSettings::IllusionBlock));
	LogBotSettingsDetail("{} says, 'Setting default {} [{}] to [{}]'", GetCleanName(), CastToBot()->GetBotSettingCategoryName(BotBaseSettings::IllusionBlock), BotBaseSettings::IllusionBlock, GetDefaultBotSettings(BotSettingCategories::BaseSetting, BotBaseSettings::IllusionBlock));

	for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
		BotSpellSettings t;

		t.spellType = i;
		t.shortName = GetSpellTypeShortNameByID(i);
		t.name = GetSpellTypeNameByID(i);
		t.hold = GetDefaultSpellHold(i);
		t.delay = GetDefaultSpellDelay(i);
		t.minThreshold = GetDefaultSpellMinThreshold(i);
		t.maxThreshold = GetDefaultSpellMaxThreshold(i);

		m_bot_spell_settings.push_back(t);

		LogBotSettingsDetail("{} says, 'Setting defaults for {} ({}) [#{}]'", GetCleanName(), t.name, t.shortName, t.spellType);
		LogBotSettingsDetail("{} says, 'Hold = [{}] | Delay = [{}ms] | MinThreshold = [{}\%] | MaxThreshold = [{}\%]'", GetCleanName(), GetDefaultSpellHold(i), GetDefaultSpellDelay(i), GetDefaultSpellMinThreshold(i), GetDefaultSpellMaxThreshold(i));
	}
}

int Client::GetDefaultBotSettings(uint8 setting_type, uint16 bot_setting) {
	switch (setting_type) {
		case BotSettingCategories::BaseSetting:
			return false;
		case BotSettingCategories::SpellHold:
			return GetDefaultSpellHold(bot_setting);
		case BotSettingCategories::SpellDelay:
			return GetDefaultSpellDelay(bot_setting);
		case BotSettingCategories::SpellMinThreshold:
			return GetDefaultSpellMinThreshold(bot_setting);
		case BotSettingCategories::SpellMaxThreshold:
			return GetDefaultSpellMaxThreshold(bot_setting);
	}
}

int Client::GetBotSetting(uint8 setting_type, uint16 bot_setting) {
	switch (setting_type) {
		case BotSettingCategories::SpellHold:
			return GetSpellHold(bot_setting);
		case BotSettingCategories::SpellDelay:
			return GetSpellDelay(bot_setting);
		case BotSettingCategories::SpellMinThreshold:
			return GetSpellMinThreshold(bot_setting);
		case BotSettingCategories::SpellMaxThreshold:
			return GetSpellMaxThreshold(bot_setting);
	}
}

void Client::SetBotSetting(uint8 setting_type, uint16 bot_setting, uint32 setting_value) {
	switch (setting_type) {
		case BotSettingCategories::BaseSetting:
			SetBaseSetting(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellHold:
			SetSpellHold(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellDelay:
			SetSpellDelay(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellMinThreshold:
			SetSpellMinThreshold(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellMaxThreshold:
			SetSpellMaxThreshold(bot_setting, setting_value);
			break;
	}
}

void Client::SendSpellTypePrompts(bool commanded_types, bool client_only_types) {
	if (client_only_types) {
		Message(
			Chat::Yellow,
			fmt::format(
				"You can view client spell types by {} or {}.",
				Saylink::Silent(
					fmt::format("^spelltypeids client"), "ID"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames client"), "Shortname"
				)
			).c_str()
		);
	}
	else {
		Message(
			Chat::Yellow,
			fmt::format(
				"You can view spell types by {}, {}, {} or by {}, {}, {}.",
				Saylink::Silent(
					fmt::format("^spelltypeids 0-19"), "ID 0-19"
				),
				Saylink::Silent(
					fmt::format("^spelltypeids 20-39"), "20-39"
				),
				Saylink::Silent(
					fmt::format("^spelltypeids 40+"), "40+"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames 0-19"), "Shortname 0-19"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames 20-39"), "20-39"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames 40+"), "40+"
				)
			).c_str()
		);
	}

	if (commanded_types) {
		Message(
			Chat::Yellow,
			fmt::format(
				"You can view commanded spell types by {} or {}.",
				Saylink::Silent(
					fmt::format("^spelltypeids commanded"), "ID"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames commanded"), "Shortname"
				)
			).c_str()
		);
	}

	return;
}
