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

/**
 * @param npc 
 */
void NpcScaleManager::ScaleNPC(NPC * npc)
{
	if (npc->IsSkipAutoScale())
		return;

	int8 npc_type       = GetNPCScalingType(npc);
	int  npc_level      = npc->GetLevel();
	bool is_auto_scaled = IsAutoScaled(npc);

	global_npc_scale scale_data = GetGlobalScaleDataForTypeLevel(npc_type, npc_level);

	if (!scale_data.level) {
		Log(Logs::General, Logs::NPCScaling, "NPC: %s - scaling data not found for type: %i level: %i",
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

			Log(Logs::Moderate,
				Logs::NPCScaling,
				"ClassLevelDamageMod::min_dmg base: %i calc: %i",
				scale_data.min_dmg,
				min_dmg);
		}

		npc->ModifyNPCStat("min_hit", std::to_string(min_dmg).c_str());
	}
	if (npc->GetMaxDMG() == 0) {
		int max_dmg = scale_data.max_dmg;
		if (RuleB(Combat, UseNPCDamageClassLevelMods)) {
			int32 class_level_damage_mod = GetClassLevelDamageMod(npc->GetLevel(), npc->GetClass());
			max_dmg = (scale_data.max_dmg * class_level_damage_mod) / 220;

			Log(Logs::Moderate,
				Logs::NPCScaling,
				"ClassLevelDamageMod::max_dmg base: %i calc: %i",
				scale_data.max_dmg,
				max_dmg
			);
		}

		npc->ModifyNPCStat("max_hit", std::to_string(max_dmg).c_str());
	}
	if (npc->GetHPRegen() == 0) {
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

		Log(Logs::General,
			Logs::NPCScaling,
			"(%s) level: %i type: %i Auto: %s Setting: %s",
			npc->GetCleanName(),
			npc_level,
			npc_type,
			(is_auto_scaled ? "true" : "false"),
			scale_log.c_str());
	}
}

bool NpcScaleManager::LoadScaleData()
{
	auto results = database.QueryDatabase(
		"SELECT "
		"type,"
		"level,"
		"ac,"
		"hp,"
		"accuracy,"
		"slow_mitigation,"
		"attack,"
		"strength,"
		"stamina,"
		"dexterity,"
		"agility,"
		"intelligence,"
		"wisdom,"
		"charisma,"
		"magic_resist,"
		"cold_resist,"
		"fire_resist,"
		"poison_resist,"
		"disease_resist,"
		"corruption_resist,"
		"physical_resist,"
		"min_dmg,"
		"max_dmg,"
		"hp_regen_rate,"
		"attack_delay,"
		"spell_scale,"
		"heal_scale,"
		"special_abilities"
		" FROM `npc_scale_global_base`"
	);

	for (auto row = results.begin(); row != results.end(); ++row) {
		global_npc_scale scale_data;

		scale_data.type              = atoi(row[0]);
		scale_data.level             = atoi(row[1]);
		scale_data.ac                = atoi(row[2]);
		scale_data.hp                = atoi(row[3]);
		scale_data.accuracy          = atoi(row[4]);
		scale_data.slow_mitigation   = atoi(row[5]);
		scale_data.attack            = atoi(row[6]);
		scale_data.strength          = atoi(row[7]);
		scale_data.stamina           = atoi(row[8]);
		scale_data.dexterity         = atoi(row[9]);
		scale_data.agility           = atoi(row[10]);
		scale_data.intelligence      = atoi(row[11]);
		scale_data.wisdom            = atoi(row[12]);
		scale_data.charisma          = atoi(row[13]);
		scale_data.magic_resist      = atoi(row[14]);
		scale_data.cold_resist       = atoi(row[15]);
		scale_data.fire_resist       = atoi(row[16]);
		scale_data.poison_resist     = atoi(row[17]);
		scale_data.disease_resist    = atoi(row[18]);
		scale_data.corruption_resist = atoi(row[19]);
		scale_data.physical_resist   = atoi(row[20]);
		scale_data.min_dmg           = atoi(row[21]);
		scale_data.max_dmg           = atoi(row[22]);
		scale_data.hp_regen_rate     = atoi(row[23]);
		scale_data.attack_delay      = atoi(row[24]);
		scale_data.spell_scale       = atoi(row[25]);
		scale_data.heal_scale        = atoi(row[26]);

		if (row[25]) {
			scale_data.special_abilities = row[27];
		}

		npc_global_base_scaling_data.insert(
			std::make_pair(
				std::make_pair(scale_data.type, scale_data.level),
				scale_data
			)
		);
	}

	Log(Logs::General, Logs::NPCScaling, "Global Base Scaling Data Loaded...");

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

	global_npc_scale scale_data = GetGlobalScaleDataForTypeLevel(npc_type, npc_level);

	if (!scale_data.level) {
		Log(
			Logs::General,
			Logs::NPCScaling,
			"NpcScaleManager::ApplyGlobalBaseScalingToNPCStatically NPC: %s - scaling data not found for type: %i level: %i",
			npc->GetCleanName(),
			npc_type,
			npc_level
		);

		return false;
	}

	std::string query = StringFormat(
		"UPDATE `npc_types` SET "
		"AC = %i, "
		"hp = %i, "
		"Accuracy = %i, "
		"slow_mitigation = %i, "
		"ATK = %i, "
		"STR = %i, "
		"STA = %i, "
		"DEX = %i, "
		"AGI = %i, "
		"_INT = %i, "
		"WIS = %i, "
		"CHA = %i, "
		"MR = %i, "
		"CR = %i, "
		"FR = %i, "
		"PR = %i, "
		"DR = %i, "
		"Corrup = %i, "
		"PhR = %i, "
		"mindmg = %i, "
		"maxdmg = %i, "
		"hp_regen_rate = %i, "
		"attack_delay = %i, "
		"spellscale = %i, "
		"healscale = %i, "
		"special_abilities = '%s' "
		"WHERE `id` = %i",
		scale_data.ac,
		scale_data.hp,
		scale_data.accuracy,
		scale_data.slow_mitigation,
		scale_data.attack,
		scale_data.strength,
		scale_data.stamina,
		scale_data.dexterity,
		scale_data.agility,
		scale_data.intelligence,
		scale_data.wisdom,
		scale_data.charisma,
		scale_data.magic_resist,
		scale_data.cold_resist,
		scale_data.fire_resist,
		scale_data.poison_resist,
		scale_data.disease_resist,
		scale_data.corruption_resist,
		scale_data.physical_resist,
		scale_data.min_dmg,
		scale_data.max_dmg,
		scale_data.hp_regen_rate,
		scale_data.attack_delay,
		scale_data.spell_scale,
		scale_data.heal_scale,
		EscapeString(scale_data.special_abilities).c_str(),
		npc->GetNPCTypeID()
	);

	auto results = database.QueryDatabase(query);

	return results.Success();
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

	global_npc_scale scale_data = GetGlobalScaleDataForTypeLevel(npc_type, npc_level);

	if (!scale_data.level) {
		Log(
			Logs::General,
			Logs::NPCScaling,
			"NpcScaleManager::ApplyGlobalBaseScalingToNPCDynamically NPC: %s - scaling data not found for type: %i level: %i",
			npc->GetCleanName(),
			npc_type,
			npc_level
		);

		return false;
	}

	std::string query = StringFormat(
		"UPDATE `npc_types` SET "
		"AC = 0, "
		"hp = 0, "
		"Accuracy = 0, "
		"slow_mitigation = 0, "
		"ATK = 0, "
		"STR = 0, "
		"STA = 0, "
		"DEX = 0, "
		"AGI = 0, "
		"_INT = 0, "
		"WIS = 0, "
		"CHA = 0, "
		"MR = 0, "
		"CR = 0, "
		"FR = 0, "
		"PR = 0, "
		"DR = 0, "
		"Corrup = 0, "
		"PhR = 0, "
		"mindmg = 0, "
		"maxdmg = 0, "
		"hp_regen_rate = 0, "
		"attack_delay = 0, "
		"spellscale = 0, "
		"healscale = 0, "
		"special_abilities = '' "
		"WHERE `id` = %i",
		npc->GetNPCTypeID()
	);

	auto results = database.QueryDatabase(query);

	return results.Success();
}
