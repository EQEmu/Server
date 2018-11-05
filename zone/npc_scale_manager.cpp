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
 * @param mob
 */
void NpcScaleManager::ScaleMob(Mob *mob)
{
	Log(Logs::General, Logs::NPCScaling, "Attempting scale on %s", mob->GetCleanName());

	if (mob->IsClient()) {
		return;
	}

	NPC *npc = mob->CastToNPC();

	int8 mob_type  = 0;
	int  mob_level = npc->GetLevel();

	if (npc->IsRareSpawn()) {
		mob_type = 1;
	}

	if (npc->IsRaidTarget()) {
		mob_type = 2;
	}

	global_npc_scale scale_data = GetGlobalScaleDataForTypeLevel(mob_type, mob_level);

	if (!scale_data.level) {
		Log(Logs::General, Logs::NPCScaling, "NPC: %s - scaling data not found for type: %i level: %i",
			mob->GetCleanName(),
			mob_type,
			mob_level
		);

		return;
	}

	if (npc->GetAC() == 0) {
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
	if (npc->GetCR() == 0) {
		npc->ModifyNPCStat("cr", std::to_string(scale_data.corruption_resist).c_str());
	}
	if (npc->GetPR() == 0) {
		npc->ModifyNPCStat("pr", std::to_string(scale_data.physical_resist).c_str());
	}
	if (npc->GetMinDMG() == 0) {
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
	if (!npc->HasSpecialAbilities()) {
		npc->ModifyNPCStat("special_abilities", scale_data.special_abilities.c_str());
	}

	ListStats(npc);
}

void NpcScaleManager::ListStats(Mob *mob)
{
	std::string stats[] = {
		"ac",
		"max_hp",
		"accuracy",
		"slow_mitigation",
		"atk",
		"str",
		"sta",
		"dex",
		"agi",
		"int",
		"wis",
		"cha",
		"mr",
		"cr",
		"fr",
		"pr",
		"dr",
		"cr",
		"pr",
		"min_hit",
		"max_hit",
		"hp_regen",
		"attack_delay",
		"special_abilities"
	};

	int stat_elements = sizeof(stats) / sizeof(stats[0]);

	for (int i = 0; i < stat_elements; i++) {
		std::string variable = StringFormat("modify_stat_%s", stats[i].c_str());
		if (mob->EntityVariableExists(variable.c_str())) {
			Log(Logs::Detail,
				Logs::NPCScaling,
				"NpcScaleManager::ListStats: %s - %s ",
				stats[i].c_str(),
				mob->GetEntityVariable(variable.c_str()));
		}
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

		if (row[25]) {
			scale_data.special_abilities = row[25];
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
 * @param mob_type
 * @param mob_level
 * @return NpcScaleManager::global_npc_scale
 */
NpcScaleManager::global_npc_scale NpcScaleManager::GetGlobalScaleDataForTypeLevel(int8 mob_type, int mob_level)
{
	auto iter = npc_global_base_scaling_data.find(std::make_pair(mob_type, mob_level));
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