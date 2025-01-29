#include "bot.h"
#include "mob.h"

bool Mob::PlotBotPositionAroundTarget(Mob* target, float& x_dest, float& y_dest, float& z_dest, float min_distance, float max_distance, bool behind_only, bool front_only, bool bypass_los) {
	bool Result = false;

	if (target) {
		float look_heading = 0;

		min_distance = min_distance;
		max_distance = max_distance;
		float temp_x = 0;
		float temp_y = 0;
		float temp_z = target->GetZ();
		float best_z = 0;
		auto offset = GetZOffset();
		const float tar_x = target->GetX();
		const float tar_y = target->GetY();
		float tar_distance = 0;

		glm::vec3 temp_z_Position;
		glm::vec4 temp_m_Position;

		const uint16 max_iterations_allowed = 50;
		uint16 counter = 0;

		while (counter < max_iterations_allowed) {
			temp_x = tar_x + zone->random.Real(-max_distance, max_distance);
			temp_y = tar_y + zone->random.Real(-max_distance, max_distance);

			temp_z_Position.x = temp_z;
			temp_z_Position.y = temp_y;
			temp_z_Position.z = temp_z;
			best_z = GetFixedZ(temp_z_Position);

			if (best_z != BEST_Z_INVALID) {
				temp_z = best_z;
			}
			else {
				counter++;
				continue;
			}

			temp_m_Position.x = temp_x;
			temp_m_Position.y = temp_y;
			temp_m_Position.z = temp_z;

			tar_distance = Distance(target->GetPosition(), temp_m_Position);

			if (tar_distance > max_distance || tar_distance < min_distance) {
				counter++;
				continue;
			}

			if (front_only && !InFrontMob(target, temp_x, temp_y)) {
				counter++;
				continue;
			}
			else if (behind_only && !BehindMob(target, temp_x, temp_y)) {
				counter++;
				continue;
			}

			if (!bypass_los && CastToBot()->RequiresLoSForPositioning() && !CheckPositioningLosFN(target, temp_x, temp_y, temp_z)) {
				counter++;
				continue;
			}

			Result = true;
			break;
		}

		if (Result) {
			x_dest = temp_x;
			y_dest = temp_y;
			z_dest = temp_z;
		}
	}

	return Result;
}

std::vector<Mob*> Mob::GatherSpellTargets(bool entire_raid, Mob* target, bool no_clients, bool no_bots, bool no_pets) {
	std::vector<Mob*> valid_spell_targets;

	auto is_valid_target = [no_clients, no_bots](Mob* member) {
		return member &&
			((member->IsClient() && !no_clients) || (member->IsBot() && !no_bots));
		};

	if (IsRaidGrouped()) {
		Raid* raid = IsBot() ? CastToBot()->GetStoredRaid() : GetRaid();

		if (raid) {
			if (entire_raid) {
				for (const auto& m : raid->members) {
					if (is_valid_target(m.member) && m.group_number != RAID_GROUPLESS) {
						valid_spell_targets.emplace_back(m.member);
					}
				}
			}
			else {
				auto group_name = target ? raid->GetGroup(target->GetName()) : raid->GetGroup(GetName());
				auto raid_group = raid->GetRaidGroupMembers(group_name);

				for (const auto& m : raid_group) {
					if (is_valid_target(m.member) && m.group_number != RAID_GROUPLESS) {
						valid_spell_targets.emplace_back(m.member);
					}
				}
			}
		}
	}
	else if (IsGrouped()) {
		Group* group = GetGroup();

		if (group) {
			for (const auto& m : group->members) {
				if (is_valid_target(m)) {
					valid_spell_targets.emplace_back(m);
				}
			}
		}
	}
	else {
		valid_spell_targets.emplace_back(this);
	}

	return valid_spell_targets;
}

uint16 Mob::GetSpellTypeIDByShortName(std::string spell_type_string) {

	for (int i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
		if (!Strings::ToLower(spell_type_string).compare(GetSpellTypeShortNameByID(i))) {
			return i;
		}
	}

	for (int i = BotSpellTypes::COMMANDED_START; i <= BotSpellTypes::COMMANDED_END; ++i) {
		if (!Strings::ToLower(spell_type_string).compare(GetSpellTypeShortNameByID(i))) {
			return i;
		}
	}

	return UINT16_MAX;
}

bool Mob::IsValidBotSpellCategory(uint8 setting_type) {
	return EQ::ValueWithin(setting_type, BotSettingCategories::START, BotSettingCategories::END_FULL);
}

std::string Mob::GetBotSpellCategoryName(uint8 setting_type) {
	return IsValidBotBaseSetting(setting_type) ? botSpellCategory_names[setting_type] : "UNKNOWN CATEGORY";
}

uint16 Mob::GetBotSpellCategoryIDByShortName(std::string setting_string) {
	for (int i = BotSettingCategories::START; i <= BotSettingCategories::END; ++i) {
		if (!Strings::ToLower(setting_string).compare(Strings::ToLower(GetBotSpellCategoryName(i)))) {
			return i;
		}
	}

	return UINT16_MAX;
}

bool Mob::IsValidBotBaseSetting(uint16 setting_type) {
	return EQ::ValueWithin(setting_type, BotBaseSettings::START_ALL, BotBaseSettings::END);
}

std::string Mob::GetBotSettingCategoryName(uint16 setting_type) {
	return IsValidBotBaseSetting(setting_type) ? botBaseSettings_names[setting_type] : "UNKNOWN SETTING";
}

uint16 Mob::GetBaseSettingIDByShortName(std::string setting_string) {
	for (int i = BotSettingCategories::START; i <= BotSettingCategories::END; ++i) {
		if (!Strings::ToLower(setting_string).compare(Strings::ToLower(GetBotSettingCategoryName(i)))) {
			return i;
		}
	}

	return UINT16_MAX;
}

bool Mob::IsValidSpellType(uint16 spell_type) {
	return (
		EQ::ValueWithin(spell_type, BotSpellTypes::START, BotSpellTypes::END) ||
		EQ::ValueWithin(spell_type, BotSpellTypes::COMMANDED_START, BotSpellTypes::COMMANDED_END)
	);
}

std::string Mob::GetSpellTypeShortNameByID(uint16 spell_type) {
	return IsValidSpellType(spell_type) ? spellType_shortNames[spell_type] : "UNKNOWN SPELLTYPE";
}

std::string Mob::GetSpellTypeNameByID(uint16 spell_type) {
	return IsValidSpellType(spell_type) ? spellType_names[spell_type] : "UNKNOWN SPELLTYPE";
}

bool Mob::IsValidSubType(uint16 sub_type) {
	return EQ::ValueWithin(sub_type, CommandedSubTypes::START, CommandedSubTypes::END);
}

std::string Mob::GetSubTypeNameByID(uint16 sub_type) {
	return IsValidSpellType(sub_type) ? botSubType_names[sub_type] : "UNKNOWN SUBTYPE";
}

bool Mob::GetDefaultSpellHold(uint16 spell_type, uint8 stance) {
	uint8 bot_class = GetClass();

	switch (spell_type) {
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::Pet:
		case BotSpellTypes::Escape:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::InCombatBuff:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::ResistBuffs:
			return false;
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::RegularHeal:
			switch (stance) {
				case Stance::Aggressive:
				case Stance::AEBurn:
				case Stance::Burn:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
			switch (stance) {
				case Stance::Aggressive:
				case Stance::AEBurn:
				case Stance::Burn:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::PreCombatBuffSong:
			if (bot_class == Class::Bard) {
				return false;
			}
			else {
				return true;
			}
		case BotSpellTypes::Nuke:
		case BotSpellTypes::DOT:
		case BotSpellTypes::Stun:
			switch (stance) {
				case Stance::Assist:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::PBAENuke:
			switch (stance) {
				case Stance::AEBurn:
					return false;
				default:
					return true;
			}
		case BotSpellTypes::Mez:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Slow:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::HateRedux:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::Snare:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Assist:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::HateLine:
			if (bot_class == Class::ShadowKnight || bot_class == Class::Paladin) {
				switch (stance) {
					case Stance::Aggressive:
						return false;
					default:
						return true;
				}
			}
			else {
				return true;
			}
		case BotSpellTypes::Charm:
		case BotSpellTypes::Resurrect:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::Fear:
		case BotSpellTypes::AEHateLine:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::PetHoTHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		default:
			return true;
	}
}

uint16 Mob::GetDefaultSpellDelay(uint16 spell_type, uint8 stance) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
			return 1500;
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
			return 2500;
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
			return 4000;
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::PetCompleteHeals:
			return 8000;
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetHoTHeals:
			return 22000;
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
					return 1;
				case Stance::Aggressive:
					return 2000;
				case Stance::Efficient:
					return 8000;
				default:
					return 4000;
			}
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::Snare:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::Stun:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
					return 1;
				case Stance::Aggressive:
					return 3000;
				case Stance::Efficient:
					return 10000;
				default:
					return 6000;
			}
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
			return 8000;
		case BotSpellTypes::Fear:
		case BotSpellTypes::AEFear:
			return 15000;
		default:
			return 1;
	}
}

uint8 Mob::GetDefaultSpellMinThreshold(uint16 spell_type, uint8 stance) {
	switch (spell_type) {
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 0;
				default:
					return 20;
			}
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 0;
				default:
					return 5;
			}
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 0;
				case Stance::Efficient:
					return 40;
				default:
					return 25;
			}
		case BotSpellTypes::Mez:
		case BotSpellTypes::AEMez:
			return 85;
		default:
			return 0;
	}
}

uint8 Mob::GetDefaultSpellMaxThreshold(uint16 spell_type, uint8 stance) {
	uint8 bot_class = GetClass();

	switch (spell_type) {
		case BotSpellTypes::Escape:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 40;
				case Stance::Efficient:
				default:
					return 25;
			}
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 55;
				case Stance::Efficient:
					return 35;
				default:
					return 40;
			}
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 70;
				case Stance::Efficient:
					return 50;
				default:
					return 60;
			}
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::PetCompleteHeals:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 90;
				case Stance::Efficient:
					return 65;
				default:
					return 80;
			}
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::Snare:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::Fear:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::Stun:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
					return 100;
				case Stance::Aggressive:
					return 100;
				case Stance::Efficient:
					return 90;
				default:
					return 99;
			}
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetHoTHeals:
			if (bot_class == Class::Necromancer || bot_class == Class::Shaman) {
				return 60;
			}
			else {
				switch (stance) {
					case Stance::AEBurn:
					case Stance::Burn:
					case Stance::Aggressive:
						return 95;
					case Stance::Efficient:
						return 80;
					default:
						return 90;
				}
			}
		case BotSpellTypes::Buff:
		case BotSpellTypes::Charm:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::HateRedux:
		case BotSpellTypes::InCombatBuff:
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::Mez:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::Pet:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::PreCombatBuffSong:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::Resurrect:
		case BotSpellTypes::HateLine:
		case BotSpellTypes::AEHateLine:
		default:
			return 100;
	}
}

void Mob::SetSpellHold(uint16 spell_type, bool hold_status) {
	m_bot_spell_settings[spell_type].hold = hold_status;
}

void Mob::SetSpellDelay(uint16 spell_type, uint16 delay_value) {
	m_bot_spell_settings[spell_type].delay = delay_value;
}

void Mob::SetSpellMinThreshold(uint16 spell_type, uint8 threshold_value) {
	m_bot_spell_settings[spell_type].min_threshold = threshold_value;
}

void Mob::SetSpellMaxThreshold(uint16 spell_type, uint8 threshold_value) {
	m_bot_spell_settings[spell_type].max_threshold = threshold_value;
}

void Mob::SetSpellTypeRecastTimer(uint16 spell_type, uint32 recast_time) {
	m_bot_spell_settings[spell_type].recast_timer.Start(recast_time);
}

void Mob::StartBotSpellTimers() {
	for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
		m_bot_spell_settings[i].recast_timer.Start();
	}
}

void Mob::DisableBotSpellTimers() {
	for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
		m_bot_spell_settings[i].recast_timer.Disable();
	}
}

bool Mob::GetUltimateSpellHold(uint16 spell_type, Mob* tar) {
	if (!tar) {
		return GetSpellHold(spell_type);
	}

	if (tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot()) {
		return tar->GetOwner()->GetSpellHold(GetPetBotSpellType(spell_type));
	}

	return GetSpellHold(spell_type);
}

uint16 Mob::GetUltimateSpellDelay(uint16 spell_type, Mob* tar) {
	if (!tar) {
		return GetSpellDelay(spell_type);
	}

	if (tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot()) {
		return tar->GetOwner()->GetSpellDelay(GetPetBotSpellType(spell_type));
	}

	if (IsBotSpellTypeOtherBeneficial(spell_type) && tar->IsOfClientBot()) {
		return tar->GetSpellDelay(spell_type);
	}

	return GetSpellDelay(spell_type);
}

bool Mob::GetUltimateSpellDelayCheck(uint16 spell_type, Mob* tar) {
	if (!tar) {
		return SpellTypeRecastCheck(spell_type);
	}

	if (tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot()) {
		return tar->GetOwner()->SpellTypeRecastCheck(GetPetBotSpellType(spell_type));
	}

	if (IsBotSpellTypeOtherBeneficial(spell_type) && tar->IsOfClientBot()) {
		return tar->SpellTypeRecastCheck(spell_type);
	}

	return SpellTypeRecastCheck(spell_type);
}

uint8 Mob::GetUltimateSpellMinThreshold(uint16 spell_type, Mob* tar) {
	if (!tar) {
		return GetSpellMinThreshold(spell_type);
	}

	if (tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot()) {
		return tar->GetOwner()->GetSpellMinThreshold(GetPetBotSpellType(spell_type));
	}

	if (IsBotSpellTypeOtherBeneficial(spell_type) && tar->IsOfClientBot()) {
		return tar->GetSpellMinThreshold(spell_type);
	}

	return GetSpellMinThreshold(spell_type);
}

uint8 Mob::GetUltimateSpellMaxThreshold(uint16 spell_type, Mob* tar) {
	if (!tar) {
		return GetSpellMaxThreshold(spell_type);
	}

	if (tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot()) {
		return tar->GetOwner()->GetSpellMaxThreshold(GetPetBotSpellType(spell_type));
	}

	if (IsBotSpellTypeOtherBeneficial(spell_type) && tar->IsOfClientBot()) {
		return tar->GetSpellMaxThreshold(spell_type);
	}

	return GetSpellMaxThreshold(spell_type);
}

uint16 Mob::GetPetBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
			return BotSpellTypes::PetVeryFastHeals;
		case BotSpellTypes::FastHeals:
			return BotSpellTypes::PetFastHeals;
		case BotSpellTypes::RegularHeal:
			return BotSpellTypes::PetRegularHeals;
		case BotSpellTypes::CompleteHeal:
			return BotSpellTypes::PetCompleteHeals;
		case BotSpellTypes::HoTHeals:
			return BotSpellTypes::PetHoTHeals;
		case BotSpellTypes::Buff:
			return BotSpellTypes::PetBuffs;
		case BotSpellTypes::Cure:
			return BotSpellTypes::PetCures;
		case BotSpellTypes::DamageShields:
			return BotSpellTypes::PetDamageShields;
		case BotSpellTypes::ResistBuffs:
			return BotSpellTypes::PetResistBuffs;
		default:
			break;
	}

	return spell_type;
}

uint8 Mob::GetHPRatioForSpellType(uint16 spell_type, Mob* tar) {
	switch (spell_type) {
		case BotSpellTypes::Escape:
		case BotSpellTypes::HateRedux:
		case BotSpellTypes::InCombatBuff:
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::Pet:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::PreCombatBuffSong:
			return GetHPRatio();
		default:
			return tar->GetHPRatio();
	}

	return tar->GetHPRatio();
}

void Mob::SetBotSetting(uint8 setting_type, uint16 bot_setting, int setting_value) {
	if (!IsOfClientBot()) {
		return;
	}

	if (IsClient()) {
		CastToClient()->SetBotSetting(setting_type, bot_setting, setting_value);
		return;
	}

	if (IsBot()) {
		CastToBot()->SetBotSetting(setting_type, bot_setting, setting_value);
		return;
	}

	return;
}

void Mob::SetBaseSetting(uint16 base_setting, int setting_value) {
	switch (base_setting) {
		case BotBaseSettings::IllusionBlock:
			SetIllusionBlock(setting_value);
			break;
		default:
			break;
	}
}

bool Mob::TargetValidation(Mob* other) {
	if (!other || GetAppearance() == eaDead) {
		return false;
	}

	return true;
}
