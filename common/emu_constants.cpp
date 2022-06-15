/*	EQEMu:  Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "emu_constants.h"
#include "languages.h"
#include "data_verification.h"
#include "bodytypes.h"

int16 EQ::invtype::GetInvTypeSize(int16 inv_type) {
	static const int16 local_array[] = {
		POSSESSIONS_SIZE,
		BANK_SIZE,
		SHARED_BANK_SIZE,
		TRADE_SIZE,
		WORLD_SIZE,
		LIMBO_SIZE,
		TRIBUTE_SIZE,
		TROPHY_TRIBUTE_SIZE,
		GUILD_TRIBUTE_SIZE,
		MERCHANT_SIZE,
		DELETED_SIZE,
		CORPSE_SIZE,
		BAZAAR_SIZE,
		INSPECT_SIZE,
		REAL_ESTATE_SIZE,
		VIEW_MOD_PC_SIZE,
		VIEW_MOD_BANK_SIZE,
		VIEW_MOD_SHARED_BANK_SIZE,
		VIEW_MOD_LIMBO_SIZE,
		ALT_STORAGE_SIZE,
		ARCHIVED_SIZE,
		MAIL_SIZE,
		GUILD_TROPHY_TRIBUTE_SIZE,
		KRONO_SIZE,
		OTHER_SIZE,
	};

	if (inv_type < TYPE_BEGIN || inv_type > TYPE_END)
		return INULL;

	return local_array[inv_type];
}

const char* EQ::bug::CategoryIDToCategoryName(CategoryID category_id) {
	switch (category_id) {
	case catVideo:
		return "Video";
	case catAudio:
		return "Audio";
	case catPathing:
		return "Pathing";
	case catQuest:
		return "Quest";
	case catTradeskills:
		return "Tradeskills";
	case catSpellStacking:
		return "Spell stacking";
	case catDoorsPortals:
		return "Doors/Portals";
	case catItems:
		return "Items";
	case catNPC:
		return "NPC";
	case catDialogs:
		return "Dialogs";
	case catLoNTCG:
		return "LoN - TCG";
	case catMercenaries:
		return "Mercenaries";
	case catOther:
	default:
		return "Other";
	}
}

EQ::bug::CategoryID EQ::bug::CategoryNameToCategoryID(const char* category_name) {
	if (!category_name)
		return catOther;

	if (!strcmp(category_name, "Video"))
		return catVideo;
	if (!strcmp(category_name, "Audio"))
		return catAudio;
	if (!strcmp(category_name, "Pathing"))
		return catPathing;
	if (!strcmp(category_name, "Quest"))
		return catQuest;
	if (!strcmp(category_name, "Tradeskills"))
		return catTradeskills;
	if (!strcmp(category_name, "Spell stacking"))
		return catSpellStacking;
	if (!strcmp(category_name, "Doors/Portals"))
		return catDoorsPortals;
	if (!strcmp(category_name, "Items"))
		return catItems;
	if (!strcmp(category_name, "NPC"))
		return catNPC;
	if (!strcmp(category_name, "Dialogs"))
		return catDialogs;
	if (!strcmp(category_name, "LoN - TCG"))
		return catLoNTCG;
	if (!strcmp(category_name, "Mercenaries"))
		return catMercenaries;
	
	return catOther;
}

const char *EQ::constants::GetStanceName(StanceType stance_type) {
	switch (stance_type) {
	case stanceUnknown:
		return "Unknown";
	case stancePassive:
		return "Passive";
	case stanceBalanced:
		return "Balanced";
	case stanceEfficient:
		return "Efficient";
	case stanceReactive:
		return "Reactive";
	case stanceAggressive:
		return "Aggressive";
	case stanceAssist:
		return "Assist";
	case stanceBurn:
		return "Burn";
	case stanceEfficient2:
		return "Efficient2";
	case stanceBurnAE:
		return "BurnAE";
	default:
		return "Invalid";
	}
}

int EQ::constants::ConvertStanceTypeToIndex(StanceType stance_type) {
	if (EQ::ValueWithin(stance_type, EQ::constants::stancePassive, EQ::constants::stanceBurnAE)) {
		return (stance_type - EQ::constants::stancePassive);
	}

	return 0;
}

const std::map<int, std::string>& EQ::constants::GetLanguageMap()
{
	static const std::map<int, std::string> language_map = {
		{ LANG_COMMON_TONGUE, "Common Tongue" },
		{ LANG_BARBARIAN, "Barbarian" },
		{ LANG_ERUDIAN, "Erudian" },
		{ LANG_ELVISH, "Elvish" },
		{ LANG_DARK_ELVISH, "Dark Elvish" },
		{ LANG_DWARVISH, "Dwarvish" },
		{ LANG_TROLL, "Troll" },
		{ LANG_OGRE, "Ogre" },
		{ LANG_GNOMISH, "Gnomish" },
		{ LANG_HALFLING, "Halfling" },
		{ LANG_THIEVES_CANT, "Thieves Cant" },
		{ LANG_OLD_ERUDIAN, "Old Erudian" },
		{ LANG_ELDER_ELVISH, "Elder Elvish" },
		{ LANG_FROGLOK, "Froglok" },
		{ LANG_GOBLIN, "Goblin" },
		{ LANG_GNOLL, "Gnoll" },
		{ LANG_COMBINE_TONGUE, "Combine Tongue" },
		{ LANG_ELDER_TEIRDAL, "Elder Teirdal" },
		{ LANG_LIZARDMAN, "Lizardman" },
		{ LANG_ORCISH, "Orcish" },
		{ LANG_FAERIE, "Faerie" },
		{ LANG_DRAGON, "Dragon" },
		{ LANG_ELDER_DRAGON, "Elder Dragon" },
		{ LANG_DARK_SPEECH, "Dark Speech" },
		{ LANG_VAH_SHIR, "Vah Shir" },
		{ LANG_ALARAN, "Alaran" },
		{ LANG_HADAL, "Hadal" },
		{ LANG_UNKNOWN, "Unknown" }
	};

	return language_map;
}

std::string EQ::constants::GetLanguageName(int language_id)
{
	if (EQ::ValueWithin(language_id, LANG_COMMON_TONGUE, LANG_UNKNOWN)) {
		return EQ::constants::GetLanguageMap().find(language_id)->second;
	}

	return std::string();
}

const std::map<uint32, std::string>& EQ::constants::GetLDoNThemeMap()
{
	static const std::map<uint32, std::string> ldon_theme_map = {
		{ LDoNThemes::Unused, "Unused" },
		{ LDoNThemes::GUK, "Deepest Guk" },
		{ LDoNThemes::MIR, "Miragul's Menagerie" },
		{ LDoNThemes::MMC, "Mistmoore Catacombs" },
		{ LDoNThemes::RUJ, "Rujarkian Hills" },
		{ LDoNThemes::TAK, "Takish-Hiz" },
	};

	return ldon_theme_map;
}

std::string EQ::constants::GetLDoNThemeName(uint32 theme_id)
{
	if (EQ::ValueWithin(theme_id, LDoNThemes::Unused, LDoNThemes::TAK)) {
		return EQ::constants::GetLDoNThemeMap().find(theme_id)->second;
	}

	return std::string();	
}

const std::map<int8, std::string>& EQ::constants::GetFlyModeMap()
{
	static const std::map<int8, std::string> flymode_map = {
		{ GravityBehavior::Ground, "Ground" },
		{ GravityBehavior::Flying, "Flying" },
		{ GravityBehavior::Levitating, "Levitating" },
		{ GravityBehavior::Water, "Water" },
		{ GravityBehavior::Floating, "Floating" },
		{ GravityBehavior::LevitateWhileRunning, "Levitating While Running" },
	};

	return flymode_map;
}

std::string EQ::constants::GetFlyModeName(int8 flymode_id)
{
	if (EQ::ValueWithin(flymode_id, GravityBehavior::Ground, GravityBehavior::LevitateWhileRunning)) {
		return EQ::constants::GetFlyModeMap().find(flymode_id)->second;
	}

	return std::string();
}

const std::map<bodyType, std::string>& EQ::constants::GetBodyTypeMap()
{
	static const std::map<bodyType, std::string> bodytype_map = {
		{ BT_Humanoid, "Humanoid" },
		{ BT_Lycanthrope, "Lycanthrope" },
		{ BT_Undead, "Undead" },
		{ BT_Giant, "Giant" },
		{ BT_Construct, "Construct" },
		{ BT_Extraplanar, "Extraplanar" },
		{ BT_Magical, "Magical" },
		{ BT_SummonedUndead, "Summoned Undead" },
		{ BT_RaidGiant, "Raid Giant" },
		{ BT_RaidColdain, "Raid Coldain" },
		{ BT_NoTarget, "Untargetable" },
		{ BT_Vampire, "Vampire" },
		{ BT_Atenha_Ra, "Aten Ha Ra" },
		{ BT_Greater_Akheva, "Greater Akheva" },
		{ BT_Khati_Sha, "Khati Sha" },
		{ BT_Seru, "Seru" },
		{ BT_Grieg_Veneficus, "Grieg Veneficus" },
		{ BT_Draz_Nurakk, "Draz Nurakk" },
		{ BT_Zek, "Zek" },
		{ BT_Luggald, "Luggald" },
		{ BT_Animal, "Animal" },
		{ BT_Insect, "Insect" },
		{ BT_Monster, "Monster" },
		{ BT_Summoned, "Summoned" },
		{ BT_Plant, "Plant" },
		{ BT_Dragon, "Dragon" },
		{ BT_Summoned2, "Summoned 2" },
		{ BT_Summoned3, "Summoned 3" },
		{ BT_Dragon2, "Dragon 2" },
		{ BT_VeliousDragon, "Velious Dragon" },
		{ BT_Familiar, "Familiar" },
		{ BT_Dragon3, "Dragon 3" },
		{ BT_Boxes, "Boxes" },
		{ BT_Muramite, "Muramite" },
		{ BT_NoTarget2, "Untargetable 2" },
		{ BT_SwarmPet, "Swarm Pet" },
		{ BT_MonsterSummon, "Monster Summon" },
		{ BT_InvisMan, "Invisible Man" },
		{ BT_Special, "Special" },
	};

	return bodytype_map;
}

std::string EQ::constants::GetBodyTypeName(bodyType bodytype_id)
{
	if (EQ::constants::GetBodyTypeMap().find(bodytype_id) != EQ::constants::GetBodyTypeMap().end()) {
		return EQ::constants::GetBodyTypeMap().find(bodytype_id)->second;
	}

	return std::string();
}

const std::map<uint8, std::string>& EQ::constants::GetAccountStatusMap()
{
	static const std::map<uint8, std::string> account_status_map = {
		{ AccountStatus::Player, "Player" },
		{ AccountStatus::Steward, "Steward" },
		{ AccountStatus::ApprenticeGuide, "Apprentice Guide" },
		{ AccountStatus::Guide, "Guide" },
		{ AccountStatus::QuestTroupe, "Quest Troupe" },
		{ AccountStatus::SeniorGuide, "Senior Guide" },
		{ AccountStatus::GMTester, "GM Tester" },
		{ AccountStatus::EQSupport, "EQ Support" },
		{ AccountStatus::GMStaff, "GM Staff" },
		{ AccountStatus::GMAdmin, "GM Admin" },
		{ AccountStatus::GMLeadAdmin, "GM Lead Admin" },
		{ AccountStatus::QuestMaster, "Quest Master" },
		{ AccountStatus::GMAreas, "GM Areas" },
		{ AccountStatus::GMCoder, "GM Coder" },
		{ AccountStatus::GMMgmt, "GM Mgmt" },
		{ AccountStatus::GMImpossible, "GM Impossible" },	
		{ AccountStatus::Max, "GM Max" }
	};

	return account_status_map;
}

std::string EQ::constants::GetAccountStatusName(uint8 account_status)
{
	for (
		auto status_level = EQ::constants::GetAccountStatusMap().rbegin();
		status_level != EQ::constants::GetAccountStatusMap().rend();
		++status_level
	) {
		if (account_status >= status_level->first) {
			return status_level->second;
		}
	}

	return std::string();
}

const std::map<uint8, std::string>& EQ::constants::GetConsiderLevelMap()
{
	static const std::map<uint8, std::string> consider_level_map = {
		{ ConsiderLevel::Ally, "Ally" },
		{ ConsiderLevel::Warmly, "Warmly" },
		{ ConsiderLevel::Kindly, "Kindly" },
		{ ConsiderLevel::Amiably, "Amiably" },
		{ ConsiderLevel::Indifferently, "Indifferently" },
		{ ConsiderLevel::Apprehensively, "Apprehensively" },
		{ ConsiderLevel::Dubiously, "Dubiously" },
		{ ConsiderLevel::Threateningly, "Threateningly" },
		{ ConsiderLevel::Scowls, "Scowls" }
	};

	return consider_level_map;
}

std::string EQ::constants::GetConsiderLevelName(uint8 faction_consider_level)
{
	if (EQ::constants::GetConsiderLevelMap().find(faction_consider_level) != EQ::constants::GetConsiderLevelMap().end()) {
		return EQ::constants::GetConsiderLevelMap().find(faction_consider_level)->second;
	}

	return std::string();
}

const std::map<uint8, std::string>& EQ::constants::GetEnvironmentalDamageMap()
{
	static const std::map<uint8, std::string> damage_type_map = {
		{ EnvironmentalDamage::Lava, "Lava" },
		{ EnvironmentalDamage::Drowning, "Drowning" },
		{ EnvironmentalDamage::Falling, "Falling" },
		{ EnvironmentalDamage::Trap, "Trap" }
	};

	return damage_type_map;
}

std::string EQ::constants::GetEnvironmentalDamageName(uint8 damage_type)
{
	if (EQ::ValueWithin(damage_type, EnvironmentalDamage::Lava, EnvironmentalDamage::Trap)) {
		return EQ::constants::GetEnvironmentalDamageMap().find(damage_type)->second;
	}

	return std::string();
}

const std::map<uint8, std::string>& EQ::constants::GetStuckBehaviorMap()
{
	static const std::map<uint8, std::string> stuck_behavior_map = {
		{ StuckBehavior::RunToTarget, "Run To Target" },
		{ StuckBehavior::WarpToTarget, "Warp To Target" },
		{ StuckBehavior::TakeNoAction, "Take No Action" },
		{ StuckBehavior::EvadeCombat, "Evade Combat" }
	};

	return stuck_behavior_map;
}

std::string EQ::constants::GetStuckBehaviorName(uint8 behavior_id)
{
	if (EQ::ValueWithin(behavior_id, StuckBehavior::RunToTarget, StuckBehavior::EvadeCombat)) {
		return EQ::constants::GetStuckBehaviorMap().find(behavior_id)->second;
	}

	return std::string();
}

const std::map<uint8, std::string>& EQ::constants::GetSpawnAnimationMap()
{
	static const std::map<uint8, std::string> spawn_animation_map = {
		{ SpawnAnimations::Standing, "Standing" },
		{ SpawnAnimations::Sitting, "Sitting" },
		{ SpawnAnimations::Crouching, "Crouching" },
		{ SpawnAnimations::Laying, "Laying" },
		{ SpawnAnimations::Looting, "Looting" }
	};

	return spawn_animation_map;
}

std::string EQ::constants::GetSpawnAnimationName(uint8 animation_id)
{
	if (EQ::ValueWithin(animation_id, SpawnAnimations::Standing, SpawnAnimations::Looting)) {
		return EQ::constants::GetSpawnAnimationMap().find(animation_id)->second;
	}

	return std::string();
}
