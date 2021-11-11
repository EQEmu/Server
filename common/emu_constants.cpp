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
	if (stance_type >= EQ::constants::stancePassive && stance_type <= EQ::constants::stanceBurnAE)
		return (stance_type - EQ::constants::stancePassive);

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
	if (language_id >= LANG_COMMON_TONGUE && language_id <= LANG_UNKNOWN) {
		auto languages = EQ::constants::GetLanguageMap();
		return languages[language_id];
	}
	return std::string();
}
