/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2018 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "npc_scale_manager.h"
#include "../common/string_util.h"
#include "../common/repositories/npc_scale_global_base_repository.h"
#include "../common/repositories/npc_types_repository.h"

/**
 * @param npc
 */
void NpcScaleManager::ScaleNPC(NPC *npc)
{
	if (npc->IsSkipAutoScale()) {
		return;
	}

	int8 npc_type       = GetNPCScalingType(npc);
	int  npc_level      = npc->GetLevel();
	bool is_auto_scaled = IsAutoScaled(npc);

	global_npc_scale scale_data = GetGlobalScaleDataForTypeLevel(npc_type, npc_level);

	if (!scale_data.level) {
		LogNPCScaling(
			"NPC: [{}] - scaling data not found for type: [{}] level: [{}]",
			npc->GetCleanName(),
			npc_type,
			npc_level
		);

		return;
	}

	if (npc->GetAC() == 0 && is_auto_scaled) {
		npc->ModifyNPCStat("ac", std::to_string(scale_data.ac).c_str());
	}
	if (npc->GetMaxHP() == 0) {
		npc->ModifyNPCStat("max_hp", std::to_string(scale_data.hp).c_str());
		npc->Heal();
	}
	if (npc->GetAccuracyRating() == 0) {
		npc->ModifyNPCStat("accuracy", std::to_string(scale_data.accuracy).c_str());
	}
	if (npc->GetSlowMitigation() == 0) {
		npc->ModifyNPCStat("slow_mitigation", std::to_string(scale_data.slow_mitigation).c_str());
	}
	if (npc->GetATK() == 0) {
		npc->ModifyNPCStat("atk", std::to_string(scale_data.attack).c_str());
	}
	if (npc->GetSTR() == 0) {
		npc->ModifyNPCStat("str", std::to_string(scale_data.strength).c_str());
	}
	if (npc->GetSTA() == 0) {
		npc->ModifyNPCStat("sta", std::to_string(scale_data.stamina).c_str());
	}
	if (npc->GetDEX() == 0) {
		npc->ModifyNPCStat("dex", std::to_string(scale_data.dexterity).c_str());
	}
	if (npc->GetAGI() == 0) {
		npc->ModifyNPCStat("agi", std::to_string(scale_data.agility).c_str());
	}
	if (npc->GetINT() == 0) {
		npc->ModifyNPCStat("int", std::to_string(scale_data.intelligence).c_str());
	}
	if (npc->GetWIS() == 0) {
		npc->ModifyNPCStat("wis", std::to_string(scale_data.wisdom).c_str());
	}
	if (npc->GetCHA() == 0) {
		npc->ModifyNPCStat("cha", std::to_string(scale_data.charisma).c_str());
	}
	if (npc->GetMR() == 0) {
		npc->ModifyNPCStat("mr", std::to_string(scale_data.magic_resist).c_str());
	}
	if (npc->GetCR() == 0) {
		npc->ModifyNPCStat("cr", std::to_string(scale_data.cold_resist).c_str());
	}
	if (npc->GetFR() == 0) {
		npc->ModifyNPCStat("fr", std::to_string(scale_data.fire_resist).c_str());
	}
	if (npc->GetPR() == 0) {
		npc->ModifyNPCStat("pr", std::to_string(scale_data.poison_resist).c_str());
	}
	if (npc->GetDR() == 0) {
		npc->ModifyNPCStat("dr", std::to_string(scale_data.disease_resist).c_str());
	}
	if (npc->GetCorrup() == 0 && is_auto_scaled) {
		npc->ModifyNPCStat("cor", std::to_string(scale_data.corruption_resist).c_str());
	}
	if (npc->GetPhR() == 0 && is_auto_scaled) {
		npc->ModifyNPCStat("phr", std::to_string(scale_data.physical_resist).c_str());
	}
	if (npc->GetMinDMG() == 0 && npc->GetMaxDMG() == 0) {
		int min_dmg = scale_data.min_dmg;
		if (RuleB(Combat, UseNPCDamageClassLevelMods)) {
			int32 class_level_damage_mod = GetClassLevelDamageMod(npc->GetLevel(), npc->GetClass());
			min_dmg = (min_dmg * class_level_damage_mod) / 220;

			LogNPCScaling("ClassLevelDamageMod::min_dmg base: [{}] calc: [{}]", scale_data.min_dmg, min_dmg);
		}

		npc->ModifyNPCStat("min_hit", std::to_string(min_dmg).c_str());
	}
	if (npc->GetMaxDMG() == 0) {
		int max_dmg = scale_data.max_dmg;
		if (RuleB(Combat, UseNPCDamageClassLevelMods)) {
			int32 class_level_damage_mod = GetClassLevelDamageMod(npc->GetLevel(), npc->GetClass());
			max_dmg = (scale_data.max_dmg * class_level_damage_mod) / 220;

			LogNPCScaling("ClassLevelDamageMod::max_dmg base: [{}] calc: [{}]", scale_data.max_dmg, max_dmg);
		}

		npc->ModifyNPCStat("max_hit", std::to_string(max_dmg).c_str());
	}
	if (npc->GetHPRegen() == 0 && is_auto_scaled) {
		npc->ModifyNPCStat("hp_regen", std::to_string(scale_data.hp_regen_rate).c_str());
	}
	if (npc->GetAttackDelay() == 0) {
		npc->ModifyNPCStat("attack_delay", std::to_string(scale_data.attack_delay).c_str());
	}
	if (npc->GetSpellScale() == 0) {
		npc->ModifyNPCStat("spell_scale", std::to_string(scale_data.spell_scale).c_str());
	}
	if (npc->GetHealScale() == 0) {
		npc->ModifyNPCStat("heal_scale", std::to_string(scale_data.heal_scale).c_str());
	}
	if (!npc->HasSpecialAbilities() && is_auto_scaled) {
		npc->ModifyNPCStat("special_abilities", scale_data.special_abilities.c_str());
	}

	if (LogSys.log_settings[Logs::NPCScaling].is_category_enabled == 1) {
		std::string scale_log;

		for (const auto &stat : scaling_stats) {
			std::string variable = StringFormat("modify_stat_%s", stat.c_str());
			if (npc->EntityVariableExists(variable.c_str())) {
				scale_log += stat + ": " + npc->GetEntityVariable(variable.c_str()) + " ";
			}
		}

		LogNPCScaling(
			"([{}]) level: [{}] type: [{}] Auto: [{}] Setting: [{}]",
			npc->GetCleanName(),
			npc_level,
			npc_type,
			(is_auto_scaled ? "true" : "false"),
			scale_log.c_str()
		);
	}
}

void NpcScaleManager::ResetNPCScaling(NPC *npc)
{
	for (const auto &scaling_stat : scaling_stats) {
		std::string stat_name   = fmt::format("modify_stat_{}", scaling_stat);
		std::string reset_value = "0";
		if (npc->EntityVariableExists(stat_name.c_str())) {
			npc->ModifyNPCStat(scaling_stat.c_str(), reset_value.c_str());
		}
	}
}

bool NpcScaleManager::LoadScaleData()
{
	for (auto &s: NpcScaleGlobalBaseRepository::All(content_db)) {
		global_npc_scale scale_data;

		scale_data.type              = s.type;
		scale_data.level             = s.level;
		scale_data.ac                = s.ac;
		scale_data.hp                = s.hp;
		scale_data.accuracy          = s.accuracy;
		scale_data.slow_mitigation   = s.slow_mitigation;
		scale_data.attack            = s.attack;
		scale_data.strength          = s.strength;
		scale_data.stamina           = s.stamina;
		scale_data.dexterity         = s.dexterity;
		scale_data.agility           = s.agility;
		scale_data.intelligence      = s.intelligence;
		scale_data.wisdom            = s.wisdom;
		scale_data.charisma          = s.charisma;
		scale_data.magic_resist      = s.magic_resist;
		scale_data.cold_resist       = s.cold_resist;
		scale_data.fire_resist       = s.fire_resist;
		scale_data.poison_resist     = s.poison_resist;
		scale_data.disease_resist    = s.disease_resist;
		scale_data.corruption_resist = s.corruption_resist;
		scale_data.physical_resist   = s.physical_resist;
		scale_data.min_dmg           = s.min_dmg;
		scale_data.max_dmg           = s.max_dmg;
		scale_data.hp_regen_rate     = s.hp_regen_rate;
		scale_data.attack_delay      = s.attack_delay;
		scale_data.spell_scale       = s.spell_scale;
		scale_data.heal_scale        = s.heal_scale;

		if (!s.special_abilities.empty()) {
			scale_data.special_abilities = s.special_abilities;
		}

		npc_global_base_scaling_data.insert(
			std::make_pair(
				std::make_pair(scale_data.type, scale_data.level),
				scale_data
			)
		);
	}

	LogNPCScaling("Global Base Scaling Data Loaded");

	return true;
}

/**
 * @param npc_type
 * @param npc_level
 * @return NpcScaleManager::global_npc_scale
 */
NpcScaleManager::global_npc_scale NpcScaleManager::GetGlobalScaleDataForTypeLevel(int8 npc_type, int npc_level)
{
	auto iter = npc_global_base_scaling_data.find(std::make_pair(npc_type, npc_level));
	if (iter != npc_global_base_scaling_data.end()) {
		return iter->second;
	}

	return {};
}

/**
 * @param level
 * @param npc_class
 * @return
 */
uint32 NpcScaleManager::GetClassLevelDamageMod(uint32 level, uint32 npc_class)
{
	uint32 multiplier = 0;

	switch (npc_class) {
		case WARRIOR: {
			if (level < 20) {
				multiplier = 220;
			}
			else if (level < 30) {
				multiplier = 230;
			}
			else if (level < 40) {
				multiplier = 250;
			}
			else if (level < 53) {
				multiplier = 270;
			}
			else if (level < 57) {
				multiplier = 280;
			}
			else if (level < 60) {
				multiplier = 290;
			}
			else if (level < 70) {
				multiplier = 300;
			}
			else {
				multiplier = 311;
			}
			break;
		}
		case DRUID:
		case CLERIC:
		case SHAMAN: {
			if (level < 70) {
				multiplier = 150;
			}
			else {
				multiplier = 157;
			}
			break;
		}
		case BERSERKER:
		case PALADIN:
		case SHADOWKNIGHT: {
			if (level < 35) {
				multiplier = 210;
			}
			else if (level < 45) {
				multiplier = 220;
			}
			else if (level < 51) {
				multiplier = 230;
			}
			else if (level < 56) {
				multiplier = 240;
			}
			else if (level < 60) {
				multiplier = 250;
			}
			else if (level < 68) {
				multiplier = 260;
			}
			else {
				multiplier = 270;
			}
			break;
		}
		case MONK:
		case BARD:
		case ROGUE:
		case BEASTLORD: {
			if (level < 51) {
				multiplier = 180;
			}
			else if (level < 58) {
				multiplier = 190;
			}
			else if (level < 70) {
				multiplier = 200;
			}
			else {
				multiplier = 210;
			}
			break;
		}
		case RANGER: {
			if (level < 58) {
				multiplier = 200;
			}
			else if (level < 70) {
				multiplier = 210;
			}
			else {
				multiplier = 220;
			}
			break;
		}
		case MAGICIAN:
		case WIZARD:
		case NECROMANCER:
		case ENCHANTER: {
			if (level < 70) {
				multiplier = 120;
			}
			else {
				multiplier = 127;
			}
			break;
		}
		default: {
			if (level < 35) {
				multiplier = 210;
			}
			else if (level < 45) {
				multiplier = 220;
			}
			else if (level < 51) {
				multiplier = 230;
			}
			else if (level < 56) {
				multiplier = 240;
			}
			else if (level < 60) {
				multiplier = 250;
			}
			else {
				multiplier = 260;
			}
			break;
		}
	}

	return multiplier;
}

/**
 * @param npc
 * @return int8
 */
int8 NpcScaleManager::GetNPCScalingType(NPC *&npc)
{
	std::string npc_name = npc->GetName();

	if (npc->IsRaidTarget()) {
		return 2;
	}

	if (npc->IsRareSpawn() || npc_name.find('#') != std::string::npos || isupper(npc_name[0])) {
		return 1;
	}

	return 0;
}

/**
 * @param npc
 * @return std::string
 */
std::string NpcScaleManager::GetNPCScalingTypeName(NPC *&npc)
{
	int8 scaling_type = GetNPCScalingType(npc);

	if (scaling_type == 1) {
		return "Named";
	}

	if (npc->IsRaidTarget()) {
		return "Raid";
	}

	return "Trash";
}

/**
 * Determines based on minimum criteria if NPC is auto scaled for certain things to be scaled like
 * special abilities. We use this so we don't blindly assume we want things to be applied
 *
 * @param npc
 * @return
 */
bool NpcScaleManager::IsAutoScaled(NPC *npc)
{
	return
		(npc->GetHP() == 0 &&
		 npc->GetMaxDMG() == 0 &&
		 npc->GetMinDMG() == 0 &&
		 npc->GetSTR() == 0 &&
		 npc->GetSTA() == 0 &&
		 npc->GetDEX() == 0 &&
		 npc->GetAGI() == 0 &&
		 npc->GetINT() == 0 &&
		 npc->GetWIS() == 0 &&
		 npc->GetCHA() == 0 &&
		 npc->GetMR() == 0 &&
		 npc->GetFR() == 0 &&
		 npc->GetCR() == 0 &&
		 npc->GetPR() == 0 &&
		 npc->GetDR() == 0);
}

/**
 * Returns false if scaling data not found
 * @param npc
 * @return
 */
bool NpcScaleManager::ApplyGlobalBaseScalingToNPCStatically(NPC *&npc)
{
	int8 npc_type  = GetNPCScalingType(npc);
	int  npc_level = npc->GetLevel();

	global_npc_scale g = GetGlobalScaleDataForTypeLevel(npc_type, npc_level);

	if (!g.level) {
		LogNPCScaling(
			"NpcScaleManager::ApplyGlobalBaseScalingToNPCStatically NPC: [{}] - scaling data not found for type: [{}] level: [{}]",
			npc->GetCleanName(),
			npc_type,
			npc_level
		);

		return false;
	}

	auto n = NpcTypesRepository::FindOne(content_db, (int) npc->GetNPCTypeID());
	if (n.id > 0) {
		n.AC                = g.ac;
		n.hp                = g.hp;
		n.Accuracy          = g.accuracy;
		n.slow_mitigation   = g.slow_mitigation;
		n.ATK               = g.attack;
		n.STR               = g.strength;
		n.STA               = g.stamina;
		n.DEX               = g.dexterity;
		n.AGI               = g.agility;
		n._INT              = g.intelligence;
		n.WIS               = g.wisdom;
		n.CHA               = g.charisma;
		n.MR                = g.magic_resist;
		n.CR                = g.cold_resist;
		n.FR                = g.fire_resist;
		n.PR                = g.poison_resist;
		n.DR                = g.disease_resist;
		n.Corrup            = g.corruption_resist;
		n.PhR               = g.physical_resist;
		n.mindmg            = g.min_dmg;
		n.maxdmg            = g.max_dmg;
		n.hp_regen_rate     = g.hp_regen_rate;
		n.attack_delay      = g.attack_delay;
		n.spellscale        = (float) g.spell_scale;
		n.healscale         = (float) g.heal_scale;
		n.special_abilities = g.special_abilities;

		return NpcTypesRepository::UpdateOne(content_db, n);
	}

	return false;
}

/**
 * Returns false if scaling data not found
 * @param npc
 * @return
 */
bool NpcScaleManager::ApplyGlobalBaseScalingToNPCDynamically(NPC *&npc)
{
	int8 npc_type  = GetNPCScalingType(npc);
	int  npc_level = npc->GetLevel();

	global_npc_scale d = GetGlobalScaleDataForTypeLevel(npc_type, npc_level);

	if (!d.level) {
		LogNPCScaling(
			"NpcScaleManager::ApplyGlobalBaseScalingToNPCDynamically NPC: [{}] - scaling data not found for type: [{}] level: [{}]",
			npc->GetCleanName(),
			npc_type,
			npc_level
		);

		return false;
	}

	auto n = NpcTypesRepository::FindOne(content_db, (int) npc->GetNPCTypeID());
	if (n.id > 0) {
		n.AC                = 0;
		n.hp                = 0;
		n.Accuracy          = 0;
		n.slow_mitigation   = 0;
		n.ATK               = 0;
		n.STR               = 0;
		n.STA               = 0;
		n.DEX               = 0;
		n.AGI               = 0;
		n._INT              = 0;
		n.WIS               = 0;
		n.CHA               = 0;
		n.MR                = 0;
		n.CR                = 0;
		n.FR                = 0;
		n.PR                = 0;
		n.DR                = 0;
		n.Corrup            = 0;
		n.PhR               = 0;
		n.mindmg            = 0;
		n.maxdmg            = 0;
		n.hp_regen_rate     = 0;
		n.attack_delay      = 0;
		n.spellscale        = 0;
		n.healscale         = 0;
		n.special_abilities = "";

		return NpcTypesRepository::UpdateOne(content_db, n);
	}

	return false;
}
