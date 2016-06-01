/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*

	To add a new bot command 3 things must be done:

	1.	At the bottom of bot_command.h you must add a prototype for it.
	2.	Add the function in this file.
	3.	In the bot_command_init function you must add a call to bot_command_add
		for your function.
		
	Notes: If you want an alias for your bot command, add an entry to the
	`bot_command_settings` table in your database. The access level you
	set with bot_command_add is the default setting if the bot command isn't
	listed in the `bot_command_settings` db table.

*/

#ifdef BOTS

#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <thread>

#ifdef _WINDOWS
#define strcasecmp _stricmp
#endif

#include "../common/global_define.h"
#include "../common/eq_packet.h"
#include "../common/features.h"
#include "../common/guilds.h"
#include "../common/patches/patches.h"
#include "../common/ptimer.h"
#include "../common/rulesys.h"
#include "../common/serverinfo.h"
#include "../common/string_util.h"
#include "../common/say_link.h"
#include "../common/eqemu_logsys.h"


#include "bot_command.h"
#include "bot_database.h"
#include "guild_mgr.h"
#include "map.h"
#include "doors.h"
#include "pathing.h"
#include "qglobals.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "titles.h"
#include "water_map.h"
#include "worldserver.h"

extern QueryServ* QServ;
extern WorldServer worldserver;
extern TaskManager *taskmanager;
void CatchSignal(int sig_num);


/*
 * file-scope helper objects
 */
namespace
{
//#define BCSTSPELLDUMP // only needed if you're adding/tailoring bot command spells and need a file dump

#define m_message CC_WhiteSmoke
#define m_action CC_Yellow
#define m_note CC_Gray
#define m_usage CC_Cyan
#define m_fail CC_Red
#define m_unknown CC_Magenta

#define HP_RATIO_DELTA 5.0f

	enum { EffectIDFirst = 1, EffectIDLast = 12 };

#define CLASSIDTOINDEX(x) ((x >= WARRIOR && x <= BERSERKER) ? (x - 1) : (0))
#define EFFECTIDTOINDEX(x) ((x >= EffectIDFirst && x <= EffectIDLast) ? (x - 1) : (0))
#define AILMENTIDTOINDEX(x) ((x >= BCEnum::AT_Blindness && x <= BCEnum::AT_Corruption) ? (x - 1) : (0))
#define RESISTANCEIDTOINDEX(x) ((x >= BCEnum::RT_Fire && x <= BCEnum::RT_Corruption) ? (x - 1) : (0))

	// ActionableTarget action_type
#define FRIENDLY true
#define ENEMY false
}

bcst_map bot_command_spells;
bcst_required_bot_classes_map required_bots_map;
bcst_required_bot_classes_map_by_class required_bots_map_by_class;


class BCSpells
{
public:
	//
	static void Load() {
		bot_command_spells.clear();
		bcst_levels_map bot_levels_map;
		
		for (int i = BCEnum::SpellTypeFirst; i <= BCEnum::SpellTypeLast; ++i) {
			bot_command_spells[static_cast<BCEnum::SpType>(i)];
			bot_levels_map[static_cast<BCEnum::SpType>(i)];
		}
		
		for (int spell_id = 2; spell_id < SPDAT_RECORDS; ++spell_id) {
			if (spells[spell_id].player_1[0] == '\0')
				continue;
			if (spells[spell_id].targettype != ST_Target && spells[spell_id].CastRestriction != 0) // watch
				continue;
			
			auto target_type = BCEnum::TT_None;
			switch (spells[spell_id].targettype) {
			case ST_GroupTeleport:
				target_type = BCEnum::TT_GroupV1;
				break;
			case ST_AECaster:
				// Disabled until bot code works correctly
				//target_type = BCEnum::TT_AECaster;
				break;
			case ST_AEBard:
				// Disabled until bot code works correctly
				//target_type = BCEnum::TT_AEBard;
				break;
			case ST_Target:
				switch (spells[spell_id].CastRestriction) {
				case 0:
					target_type = BCEnum::TT_Single;
					break;
				case 104:
					target_type = BCEnum::TT_Animal;
					break;
				case 105:
					target_type = BCEnum::TT_Plant;
					break;
				case 118:
					target_type = BCEnum::TT_Summoned;
					break;
				case 120:
					target_type = BCEnum::TT_Undead;
					break;
				default:
					break;
				}
				break;
			case ST_Self:
				target_type = BCEnum::TT_Self;
				break;
			case ST_AETarget:
				// Disabled until bot code works correctly
				//target_type = BCEnum::TT_AETarget;
				break;
			case ST_Animal:
				target_type = BCEnum::TT_Animal;
				break;
			case ST_Undead:
				target_type = BCEnum::TT_Undead;
				break;
			case ST_Summoned:
				target_type = BCEnum::TT_Summoned;
				break;
			case ST_Corpse:
				target_type = BCEnum::TT_Corpse;
				break;
			case ST_Plant:
				target_type = BCEnum::TT_Plant;
				break;
			case ST_Group:
				target_type = BCEnum::TT_GroupV2;
				break;
			default:
				break;
			}
			if (target_type == BCEnum::TT_None)
				continue;
			
			uint8 class_levels[16] = { 0 };
			bool player_spell = false;
			for (int class_type = WARRIOR; class_type <= BERSERKER; ++class_type) {
				int class_index = CLASSIDTOINDEX(class_type);
				if (spells[spell_id].classes[class_index] == 0 || spells[spell_id].classes[class_index] > HARD_LEVEL_CAP)
					continue;

				class_levels[class_index] = spells[spell_id].classes[class_index];
				player_spell = true;
			}
			if (!player_spell)
				continue;
			
			STBaseEntry* entry_prototype = nullptr;
			while (true) {
				switch (spells[spell_id].effectid[EFFECTIDTOINDEX(1)]) {
				case SE_BindAffinity:
					entry_prototype = new STBaseEntry(BCEnum::SpT_BindAffinity);
					break;
				case SE_Charm:
					if (spells[spell_id].SpellAffectIndex != 12)
						break;
					entry_prototype = new STCharmEntry();
					if (spells[spell_id].ResistDiff <= -1000)
						entry_prototype->SafeCastToCharm()->dire = true;
					break;
				case SE_Teleport:
					entry_prototype = new STDepartEntry;
					entry_prototype->SafeCastToDepart()->single = !BCSpells::IsGroupType(target_type);
					break;
				case SE_Succor:
					if (!strcmp(spells[spell_id].teleport_zone, "same")) {
						entry_prototype = new STEscapeEntry;
					}
					else {
						entry_prototype = new STDepartEntry;
						entry_prototype->SafeCastToDepart()->single = !BCSpells::IsGroupType(target_type);
					}
					break;
				case SE_Translocate:
					if (spells[spell_id].teleport_zone[0] == '\0') {
						entry_prototype = new STSendHomeEntry();
						entry_prototype->SafeCastToSendHome()->group = BCSpells::IsGroupType(target_type);
					}
					else {
						entry_prototype = new STDepartEntry;
						entry_prototype->SafeCastToDepart()->single = !BCSpells::IsGroupType(target_type);
					}
					break;
				case SE_ModelSize:
					if (spells[spell_id].base[EFFECTIDTOINDEX(1)] > 100) {
						entry_prototype = new STSizeEntry;
						entry_prototype->SafeCastToSize()->size_type = BCEnum::SzT_Enlarge;
					}
					else if (spells[spell_id].base[EFFECTIDTOINDEX(1)] > 0 && spells[spell_id].base[EFFECTIDTOINDEX(1)] < 100) {
						entry_prototype = new STSizeEntry;
						entry_prototype->SafeCastToSize()->size_type = BCEnum::SzT_Reduce;
					}
					break;
				case SE_Identify:
					entry_prototype = new STBaseEntry(BCEnum::SpT_Identify);
					break;
				case SE_Invisibility:
					if (spells[spell_id].SpellAffectIndex != 9)
						break;
					entry_prototype = new STInvisibilityEntry;
					entry_prototype->SafeCastToInvisibility()->invis_type = BCEnum::IT_Living;
					break;
				case SE_SeeInvis:
					if (spells[spell_id].SpellAffectIndex != 5)
						break;
					entry_prototype = new STInvisibilityEntry;
					entry_prototype->SafeCastToInvisibility()->invis_type = BCEnum::IT_See;
					break;
				case SE_InvisVsUndead:
					if (spells[spell_id].SpellAffectIndex != 9)
						break;
					entry_prototype = new STInvisibilityEntry;
					entry_prototype->SafeCastToInvisibility()->invis_type = BCEnum::IT_Undead;
					break;
				case SE_InvisVsAnimals:
					if (spells[spell_id].SpellAffectIndex != 9)
						break;
					entry_prototype = new STInvisibilityEntry;
					entry_prototype->SafeCastToInvisibility()->invis_type = BCEnum::IT_Animal;
					break;
				case SE_Mez:
					if (spells[spell_id].SpellAffectIndex != 12)
						break;
					entry_prototype = new STBaseEntry(BCEnum::SpT_Mesmerize);
					break;
				case SE_Revive:
					if (spells[spell_id].SpellAffectIndex != 1)
						break;
					entry_prototype = new STResurrectEntry();
					entry_prototype->SafeCastToResurrect()->aoe = BCSpells::IsCasterCentered(target_type);
					break;
				case SE_Rune:
					if (spells[spell_id].SpellAffectIndex != 2)
						break;
					entry_prototype = new STBaseEntry(BCEnum::SpT_Rune);
					break;
				case SE_SummonCorpse:
					entry_prototype = new STBaseEntry(BCEnum::SpT_SummonCorpse);
					break;
				case SE_WaterBreathing:
					entry_prototype = new STBaseEntry(BCEnum::SpT_WaterBreathing);
					break;
				default:
					break;
				}
				if (entry_prototype)
					break;
				
				switch (spells[spell_id].effectid[EFFECTIDTOINDEX(2)]) {
				case SE_Succor:
					entry_prototype = new STEscapeEntry;
					std::string is_lesser = spells[spell_id].name;
					if (is_lesser.find("Lesser") != std::string::npos)
						entry_prototype->SafeCastToEscape()->lesser = true;
					break;
				}
				if (entry_prototype)
					break;

				switch (spells[spell_id].effectid[EFFECTIDTOINDEX(3)]) {
				case SE_Lull:
					entry_prototype = new STBaseEntry(BCEnum::SpT_Lull);
					break;
				case SE_Levitate: // needs more criteria
					entry_prototype = new STBaseEntry(BCEnum::SpT_Levitation);
					break;
				default:
					break;
				}
				if (entry_prototype)
					break;
				
				while (spells[spell_id].typedescnum == 27) {
					if (!spells[spell_id].goodEffect)
						break;
					if (spells[spell_id].skill != EQEmu::skills::SkillOffense && spells[spell_id].skill != EQEmu::skills::SkillDefense)
						break;

					entry_prototype = new STStanceEntry();
					if (spells[spell_id].skill == EQEmu::skills::SkillOffense)
						entry_prototype->SafeCastToStance()->stance_type = BCEnum::StT_Aggressive;
					else
						entry_prototype->SafeCastToStance()->stance_type = BCEnum::StT_Defensive;

					break;
				}
				if (entry_prototype)
					break;
				
				switch (spells[spell_id].SpellAffectIndex) {
				case 1: {
					bool valid_spell = false;
					entry_prototype = new STCureEntry;

					for (int i = EffectIDFirst; i <= EffectIDLast; ++i) {
						int effect_index = EFFECTIDTOINDEX(i);
						if (spells[spell_id].effectid[effect_index] != SE_Blind && spells[spell_id].base[effect_index] >= 0)
							continue;
						else if (spells[spell_id].effectid[effect_index] == SE_Blind && !spells[spell_id].goodEffect)
							continue;

						switch (spells[spell_id].effectid[effect_index]) {
						case SE_Blind:
							entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(BCEnum::AT_Blindness)] += spells[spell_id].base[effect_index];
							break;
						case SE_DiseaseCounter:
							entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(BCEnum::AT_Disease)] += spells[spell_id].base[effect_index];
							break;
						case SE_PoisonCounter:
							entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(BCEnum::AT_Poison)] += spells[spell_id].base[effect_index];
							break;
						case SE_CurseCounter:
							entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(BCEnum::AT_Curse)] += spells[spell_id].base[effect_index];
							break;
						case SE_CorruptionCounter:
							entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(BCEnum::AT_Corruption)] += spells[spell_id].base[effect_index];
							break;
						default:
							continue;
						}
						entry_prototype->SafeCastToCure()->cure_total += spells[spell_id].base[effect_index];
						valid_spell = true;
					}
					if (!valid_spell) {
						safe_delete(entry_prototype);
						entry_prototype = nullptr;
					}

					break;
				}
				case 2: {
					bool valid_spell = false;
					entry_prototype = new STResistanceEntry;

					for (int i = EffectIDFirst; i <= EffectIDLast; ++i) {
						int effect_index = EFFECTIDTOINDEX(i);
						if (spells[spell_id].base[effect_index] <= 0)
							continue;

						switch (spells[spell_id].effectid[effect_index]) {
						case SE_ResistFire:
							entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(BCEnum::RT_Fire)] += spells[spell_id].base[effect_index];
							break;
						case SE_ResistCold:
							entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(BCEnum::RT_Cold)] += spells[spell_id].base[effect_index];
							break;
						case SE_ResistPoison:
							entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(BCEnum::RT_Poison)] += spells[spell_id].base[effect_index];
							break;
						case SE_ResistDisease:
							entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(BCEnum::RT_Disease)] += spells[spell_id].base[effect_index];
							break;
						case SE_ResistMagic:
							entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(BCEnum::RT_Magic)] += spells[spell_id].base[effect_index];
							break;
						case SE_ResistCorruption:
							entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(BCEnum::RT_Corruption)] += spells[spell_id].base[effect_index];
							break;
						default:
							continue;
						}
						entry_prototype->SafeCastToResistance()->resist_total += spells[spell_id].base[effect_index];
						valid_spell = true;
					}
					if (!valid_spell) {
						safe_delete(entry_prototype);
						entry_prototype = nullptr;
					}

					break;
				}
				case 7:
				case 10:
					if (spells[spell_id].effectdescnum != 65)
						break;
					entry_prototype = new STMovementSpeedEntry();
					entry_prototype->SafeCastToMovementSpeed()->group = BCSpells::IsGroupType(target_type);
					break;
				default:
					break;
				}
				if (entry_prototype)
					break;

				break;
			}
			if (!entry_prototype)
				continue;
			
			if (target_type == BCEnum::TT_Self && (entry_prototype->BCST() != BCEnum::SpT_Stance && entry_prototype->BCST() != BCEnum::SpT_SummonCorpse)) {
#ifdef BCSTSPELLDUMP
				Log.Out(Logs::General, Logs::Error, "DELETING entry_prototype (primary clause) - name: %s, target_type: %s, BCST: %s",
					spells[spell_id].name, BCEnum::TargetTypeEnumToString(target_type).c_str(), BCEnum::SpellTypeEnumToString(entry_prototype->BCST()).c_str());
#endif
				safe_delete(entry_prototype);
				continue;
			}
			if (entry_prototype->BCST() == BCEnum::SpT_Stance && target_type != BCEnum::TT_Self) {
#ifdef BCSTSPELLDUMP
				Log.Out(Logs::General, Logs::Error, "DELETING entry_prototype (secondary clause) - name: %s, BCST: %s, target_type: %s",
					spells[spell_id].name, BCEnum::SpellTypeEnumToString(entry_prototype->BCST()).c_str(), BCEnum::TargetTypeEnumToString(target_type).c_str());
#endif
				safe_delete(entry_prototype);
				continue;
			}

			assert(entry_prototype->BCST() != BCEnum::SpT_None);

			entry_prototype->spell_id = spell_id;
			entry_prototype->target_type = target_type;

			bcst_levels& bot_levels = bot_levels_map[entry_prototype->BCST()];
			for (int class_type = WARRIOR; class_type <= BERSERKER; ++class_type) {
				int class_index = CLASSIDTOINDEX(class_type);
				if (!class_levels[class_index])
					continue;

				STBaseEntry* spell_entry = nullptr;
				switch (entry_prototype->BCST()) {
				case BCEnum::SpT_Charm:
					if (entry_prototype->IsCharm())
						spell_entry = new STCharmEntry(entry_prototype->SafeCastToCharm());
					break;
				case BCEnum::SpT_Cure:
					if (entry_prototype->IsCure())
						spell_entry = new STCureEntry(entry_prototype->SafeCastToCure());
					break;
				case BCEnum::SpT_Depart:
					if (entry_prototype->IsDepart())
						spell_entry = new STDepartEntry(entry_prototype->SafeCastToDepart());
					break;
				case BCEnum::SpT_Escape:
					if (entry_prototype->IsEscape())
						spell_entry = new STEscapeEntry(entry_prototype->SafeCastToEscape());
					break;
				case BCEnum::SpT_Invisibility:
					if (entry_prototype->IsInvisibility())
						spell_entry = new STInvisibilityEntry(entry_prototype->SafeCastToInvisibility());
					break;
				case BCEnum::SpT_MovementSpeed:
					if (entry_prototype->IsMovementSpeed())
						spell_entry = new STMovementSpeedEntry(entry_prototype->SafeCastToMovementSpeed());
					break;
				case BCEnum::SpT_Resistance:
					if (entry_prototype->IsResistance())
						spell_entry = new STResistanceEntry(entry_prototype->SafeCastToResistance());
					break;
				case BCEnum::SpT_Resurrect:
					if (entry_prototype->IsResurrect())
						spell_entry = new STResurrectEntry(entry_prototype->SafeCastToResurrect());
					break;
				case BCEnum::SpT_SendHome:
					if (entry_prototype->IsSendHome())
						spell_entry = new STSendHomeEntry(entry_prototype->SafeCastToSendHome());
					break;
				case BCEnum::SpT_Size:
					if (entry_prototype->IsSize())
						spell_entry = new STSizeEntry(entry_prototype->SafeCastToSize());
					break;
				case BCEnum::SpT_Stance:
					if (entry_prototype->IsStance())
						spell_entry = new STStanceEntry(entry_prototype->SafeCastToStance());
					break;
				default:
					spell_entry = new STBaseEntry(entry_prototype);
					break;
				}
				
				assert(spell_entry);

				spell_entry->caster_class = class_type;
				spell_entry->spell_level = class_levels[class_index];
				
				bot_command_spells[spell_entry->BCST()].push_back(spell_entry);

				if (bot_levels.find(class_type) == bot_levels.end() || bot_levels[class_type] > class_levels[class_index])
					bot_levels[class_type] = class_levels[class_index];
			}

			delete(entry_prototype);
		}
		
		remove_inactive();
		order_all();
		load_teleport_zone_names();
		build_strings(bot_levels_map);
		status_report();

#ifdef BCSTSPELLDUMP
		spell_dump();
#endif

	}

	static void Unload() {
		for (auto map_iter : bot_command_spells) {
			if (map_iter.second.empty())
				continue;
			for (auto list_iter : map_iter.second)
				safe_delete(list_iter);
			map_iter.second.clear();
		}
		bot_command_spells.clear();
		required_bots_map.clear();
		required_bots_map_by_class.clear();
	}

	static bool IsCasterCentered(BCEnum::TType target_type) {
		switch (target_type) {
		case BCEnum::TT_AECaster:
		case BCEnum::TT_AEBard:
			return true;
		default:
			return false;
		}
	}

	static bool IsGroupType(BCEnum::TType target_type) {
		switch (target_type) {
		case BCEnum::TT_GroupV1:
		case BCEnum::TT_GroupV2:
			return true;
		default:
			return false;
		}
	}

private:
	static void remove_inactive() {
		if (bot_command_spells.empty())
			return;

		for (auto map_iter = bot_command_spells.begin(); map_iter != bot_command_spells.end(); ++map_iter) {
			if (map_iter->second.empty())
				continue;

			bcst_list* spells_list = &map_iter->second;
			bcst_list* removed_spells_list = new bcst_list;

			spells_list->remove(nullptr);
			spells_list->remove_if([removed_spells_list](STBaseEntry* l) {
				if (l->spell_id < 2 || l->spell_id >= SPDAT_RECORDS || strlen(spells[l->spell_id].name) < 3) {
					removed_spells_list->push_back(l);
					return true;
				}
				else {
					return false;
				}
			});

			for (auto del_iter : *removed_spells_list)
				safe_delete(del_iter);
			removed_spells_list->clear();

			if (RuleI(Bots, CommandSpellRank) == 1) {
				spells_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (spells[l->spell_id].spellgroup < spells[r->spell_id].spellgroup)
						return true;
					if (spells[l->spell_id].spellgroup == spells[r->spell_id].spellgroup && l->caster_class < r->caster_class)
						return true;
					if (spells[l->spell_id].spellgroup == spells[r->spell_id].spellgroup && l->caster_class == r->caster_class && spells[l->spell_id].rank < spells[r->spell_id].rank)
						return true;

					return false;
				});
				spells_list->unique([removed_spells_list](STBaseEntry* l, STBaseEntry* r) {
					std::string r_name = spells[r->spell_id].name;
					if (spells[l->spell_id].spellgroup == spells[r->spell_id].spellgroup && l->caster_class == r->caster_class && spells[l->spell_id].rank < spells[r->spell_id].rank) {
						removed_spells_list->push_back(r);
						return true;
					}

					return false;
				});

				for (auto del_iter : *removed_spells_list)
					safe_delete(del_iter);
				removed_spells_list->clear();
			}

			if (RuleI(Bots, CommandSpellRank) == 2) {
				spells_list->remove_if([removed_spells_list](STBaseEntry* l) {
					std::string l_name = spells[l->spell_id].name;
					if (spells[l->spell_id].rank == 10) {
						removed_spells_list->push_back(l);
						return true;
					}
					if (l_name.find("III") == (l_name.size() - 3)) {
						removed_spells_list->push_back(l);
						return true;
					}
					if (l_name.find("III ") == (l_name.size() - 4)) {
						removed_spells_list->push_back(l);
						return true;
					}

					return false;
				});

				for (auto del_iter : *removed_spells_list)
					safe_delete(del_iter);
				removed_spells_list->clear();
			}

			// needs rework
			if (RuleI(Bots, CommandSpellRank) == 2 || RuleI(Bots, CommandSpellRank) == 3) {
				spells_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (spells[l->spell_id].spellgroup < spells[r->spell_id].spellgroup)
						return true;
					if (spells[l->spell_id].spellgroup == spells[r->spell_id].spellgroup && l->caster_class < r->caster_class)
						return true;
					if (spells[l->spell_id].spellgroup == spells[r->spell_id].spellgroup && l->caster_class == r->caster_class && spells[l->spell_id].rank > spells[r->spell_id].rank)
						return true;

					return false;
				});
				spells_list->unique([removed_spells_list](STBaseEntry* l, STBaseEntry* r) {
					std::string l_name = spells[l->spell_id].name;
					if (spells[l->spell_id].spellgroup == spells[r->spell_id].spellgroup && l->caster_class == r->caster_class && spells[l->spell_id].rank > spells[r->spell_id].rank) {
						removed_spells_list->push_back(r);
						return true;
					}

					return false;
				});

				for (auto del_iter : *removed_spells_list)
					safe_delete(del_iter);
				removed_spells_list->clear();
			}

			safe_delete(removed_spells_list);
		}
	}

	static void order_all() {
		// Example of a macro'd lambda using anonymous property dereference:
		// #define XXX(p) ([](const <_Ty>* l, const <_Ty>* r) { return (l->p < r->p); })


#define LT_STBASE(l, r, p) (l->p < r->p)
#define LT_STCHARM(l, r, p) (l->SafeCastToCharm()->p < r->SafeCastToCharm()->p)
#define LT_STCURE(l, r, p) (l->SafeCastToCure()->p < r->SafeCastToCure()->p)
#define LT_STCURE_VAL_ID(l, r, p, ctid) (l->SafeCastToCure()->p[AILMENTIDTOINDEX(ctid)] < r->SafeCastToCure()->p[AILMENTIDTOINDEX(ctid)])
#define LT_STDEPART(l, r, p) (l->SafeCastToDepart()->p < r->SafeCastToDepart()->p)
#define LT_STESCAPE(l, r, p) (l->SafeCastToEscape()->p < r->SafeCastToEscape()->p)
#define LT_STINVISIBILITY(l, r, p) (l->SafeCastToInvisibility()->p < r->SafeCastToInvisibility()->p)
#define LT_STRESISTANCE(l, r, p) (l->SafeCastToResistance()->p < r->SafeCastToResistance()->p)
#define LT_STRESISTANCE_VAL_ID(l, r, p, rtid) (l->SafeCastToResistance()->p[RESISTANCEIDTOINDEX(rtid)] < r->SafeCastToResistance()->p[RESISTANCEIDTOINDEX(rtid))
#define LT_STSTANCE(l, r, p) (l->SafeCastToStance()->p < r->SafeCastToStance()->p)
#define LT_SPELLS(l, r, p) (spells[l->spell_id].p < spells[r->spell_id].p)
#define LT_SPELLS_EFFECT_ID(l, r, p, eid) (spells[l->spell_id].p[EFFECTIDTOINDEX(eid)] < spells[r->spell_id].p[EFFECTIDTOINDEX(eid)])
#define LT_SPELLS_STR(l, r, s) (strcasecmp(spells[l->spell_id].s, spells[r->spell_id].s) < 0)

#define EQ_STBASE(l, r, p) (l->p == r->p)
#define EQ_STCHARM(l, r, p) (l->SafeCastToCharm()->p == r->SafeCastToCharm()->p)
#define EQ_STCURE(l, r, p) (l->SafeCastToCure()->p == r->SafeCastToCure()->p)
#define EQ_STCURE_VAL_ID(l, r, p, ctid) (l->SafeCastToCure()->p[AILMENTIDTOINDEX(ctid)] == r->SafeCastToCure()->p[AILMENTIDTOINDEX(ctid)])
#define EQ_STDEPART(l, r, p) (l->SafeCastToDepart()->p == r->SafeCastToDepart()->p)
#define EQ_STESCAPE(l, r, p) (l->SafeCastToEscape()->p == r->SafeCastToEscape()->p)
#define EQ_STINVISIBILITY(l, r, p) (l->SafeCastToInvisibility()->p == r->SafeCastToInvisibility()->p)
#define EQ_STRESISTANCE(l, r, p) (l->SafeCastToResistance()->p == r->SafeCastToResistance()->p)
#define EQ_STRESISTANCE_VAL_ID(l, r, p, rtid) (l->SafeCastToResistance()->p[RESISTANCEIDTOINDEX(rtid)] == r->SafeCastToResistance()->p[RESISTANCEIDTOINDEX(rtid))
#define EQ_STSTANCE(l, r, p) (l->SafeCastToStance()->p == r->SafeCastToStance()->p)
#define EQ_SPELLS(l, r, p) (spells[l->spell_id].p == spells[r->spell_id].p)
#define EQ_SPELLS_EFFECT_ID(l, r, p, eid) (spells[l->spell_id].p[EFFECTIDTOINDEX(eid)] == spells[r->spell_id].p[EFFECTIDTOINDEX(eid)])
#define EQ_SPELLS_STR(l, r, s) (strcasecmp(spells[l->spell_id].s, spells[r->spell_id].s) == 0)

#define GT_STBASE(l, r, p) (l->p > r->p)
#define GT_STCHARM(l, r, p) (l->SafeCastToCharm()->p > r->SafeCastToCharm()->p)
#define GT_STCURE(l, r, p) (l->SafeCastToCure()->p > r->SafeCastToCure()->p)
#define GT_STCURE_VAL_ID(l, r, p, ctid) (l->SafeCastToCure()->p[AILMENTIDTOINDEX(ctid)] > r->SafeCastToCure()->p[AILMENTIDTOINDEX(ctid)])
#define GT_STDEPART(l, r, p) (l->SafeCastToDepart()->p > r->SafeCastToDepart()->p)
#define GT_STESCAPE(l, r, p) (l->SafeCastToEscape()->p > r->SafeCastToEscape()->p)
#define GT_STINVISIBILITY(l, r, p) (l->SafeCastToInvisibility()->p > r->SafeCastToInvisibility()->p)
#define GT_STRESISTANCE(l, r, p) (l->SafeCastToResistance()->p > r->SafeCastToResistance()->p)
#define GT_STRESISTANCE_VAL_ID(l, r, p, rtid) (l->SafeCastToResistance()->p[RESISTANCEIDTOINDEX(rtid)] > r->SafeCastToResistance()->p[RESISTANCEIDTOINDEX(rtid))
#define GT_STSTANCE(l, r, p) (l->SafeCastToStance()->p > r->SafeCastToStance()->p)
#define GT_SPELLS(l, r, p) (spells[l->spell_id].p > spells[r->spell_id].p)
#define GT_SPELLS_EFFECT_ID(l, r, p, eid) (spells[l->spell_id].p[EFFECTIDTOINDEX(eid)] > spells[r->spell_id].p[EFFECTIDTOINDEX(eid)])
#define GT_SPELLS_STR(l, r, s) (strcasecmp(spells[l->spell_id].s, spells[r->spell_id].s) > 0)


		for (auto map_iter = bot_command_spells.begin(); map_iter != bot_command_spells.end(); ++map_iter) {
			if (map_iter->second.size() < 2)
				continue;

			auto spell_type = map_iter->first;
			bcst_list* spell_list = &map_iter->second;
			switch (spell_type) {
			case BCEnum::SpT_BindAffinity:
				if (RuleB(Bots, PreferNoManaCommandSpells)) {
					spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
						if (LT_SPELLS(l, r, mana))
							return true;
						if (EQ_SPELLS(l, r, mana) && LT_STBASE(l, r, target_type))
							return true;
						if (EQ_SPELLS(l, r, mana) && EQ_STBASE(l, r, target_type) && LT_STBASE(l, r, spell_level))
							return true;
						if (EQ_SPELLS(l, r, mana) && EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
							return true;

						return false;
					});
				}
				else {
					spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
						if (LT_STBASE(l, r, target_type))
							return true;
						if (EQ_STBASE(l, r, target_type) && LT_STBASE(l, r, spell_level))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
							return true;

						return false;
					});
				}
				continue;
			case BCEnum::SpT_Charm:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_SPELLS(l, r, ResistDiff))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, max, 1))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max, 1) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Cure: // per-use sorting in command handler
				spell_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (l->spell_id < r->spell_id)
						return true;
					if (l->spell_id == r->spell_id && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Depart:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && LT_STBASE(l, r, caster_class))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, caster_class) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, caster_class) && EQ_STBASE(l, r, spell_level) && LT_SPELLS_STR(l, r, name))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Escape:
				spell_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (LT_STESCAPE(l, r, lesser))
						return true;
					if (EQ_STESCAPE(l, r, lesser) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STESCAPE(l, r, lesser) && EQ_STBASE(l, r, target_type) && GT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STESCAPE(l, r, lesser) && EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Identify:
				if (RuleB(Bots, PreferNoManaCommandSpells)) {
					spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
						if (LT_SPELLS(l, r, mana))
							return true;
						if (EQ_SPELLS(l, r, mana) && LT_STBASE(l, r, target_type))
							return true;
						if (EQ_SPELLS(l, r, mana) && EQ_STBASE(l, r, target_type) && LT_STBASE(l, r, spell_level))
							return true;
						if (EQ_SPELLS(l, r, mana) && EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
							return true;

						return false;
					});
				}
				else {
					spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
						if (LT_STBASE(l, r, target_type))
							return true;
						if (EQ_STBASE(l, r, target_type) && LT_STBASE(l, r, spell_level))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
							return true;

						return false;
					});
				}
				continue;
			case BCEnum::SpT_Invisibility:
				spell_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (LT_STINVISIBILITY(l, r, invis_type))
						return true;
					if (EQ_STINVISIBILITY(l, r, invis_type) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STINVISIBILITY(l, r, invis_type) && EQ_STBASE(l, r, target_type) && GT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STINVISIBILITY(l, r, invis_type) && EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;
					return false;
				});
				continue;
			case BCEnum::SpT_Levitation:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && LT_SPELLS(l, r, zonetype))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS(l, r, zonetype) && GT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS(l, r, zonetype) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Lull:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_SPELLS(l, r, ResistDiff))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, max, 3))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max, 3) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max, 3) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Mesmerize:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (GT_SPELLS(l, r, ResistDiff))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, max, 1))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max, 1) && GT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS(l, r, ResistDiff) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_MovementSpeed:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, base, 2))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base, 2) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base, 2) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Resistance: // per-use sorting in command handler
				spell_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (l->spell_id < r->spell_id)
						return true;
					if (l->spell_id == r->spell_id && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Resurrect:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (GT_SPELLS_EFFECT_ID(l, r, base, 1))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base, 1) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base, 1) && EQ_STBASE(l, r, target_type) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base, 1) && EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Rune:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, max, 1))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max, 1) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_SendHome:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && GT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Size:
				spell_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;

					auto l_size_type = l->SafeCastToSize()->size_type;
					auto r_size_type = r->SafeCastToSize()->size_type;
					if (l_size_type < r_size_type)
						return true;
					if (l_size_type == BCEnum::SzT_Enlarge && r_size_type == BCEnum::SzT_Enlarge) {
						if (EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, base, 1))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base, 1) && GT_STBASE(l, r, spell_level))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
							return true;
					}
					if (l_size_type == BCEnum::SzT_Reduce && r_size_type == BCEnum::SzT_Reduce) {
						if (EQ_STBASE(l, r, target_type) && LT_SPELLS_EFFECT_ID(l, r, base, 1))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base, 1) && GT_STBASE(l, r, spell_level))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
							return true;
					}

					return false;
				});
				continue;
			case BCEnum::SpT_Stance:
				spell_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (LT_STSTANCE(l, r, stance_type))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_SummonCorpse:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (GT_SPELLS_EFFECT_ID(l, r, base, 1))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base, 1) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base, 1) && EQ_STBASE(l, r, spell_level) && EQ_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_WaterBreathing:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && GT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			default:
				continue;
			}
		}
	}

	static void load_teleport_zone_names() {
		auto depart_list = &bot_command_spells[BCEnum::SpT_Depart];
		if (depart_list->empty())
			return;

		std::string query = "SELECT `short_name`, `long_name` FROM `zone` WHERE '' NOT IN (`short_name`, `long_name`)";
		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			Log.Out(Logs::General, Logs::Error, "load_teleport_zone_names() - Error in zone names query: %s", results.ErrorMessage().c_str());
			return;
		}

		std::map<std::string, std::string> zone_names;
		for (auto row = results.begin(); row != results.end(); ++row)
			zone_names[row[0]] = row[1];

		for (auto list_iter = depart_list->begin(); list_iter != depart_list->end();) {
			auto test_iter = zone_names.find(spells[(*list_iter)->spell_id].teleport_zone);
			if (test_iter == zone_names.end()) {
				list_iter = depart_list->erase(list_iter);
				continue;
			}

			(*list_iter)->SafeCastToDepart()->long_name = test_iter->second;
			++list_iter;
		}

	}

	static void build_strings(bcst_levels_map& bot_levels_map) {
		for (int i = BCEnum::SpellTypeFirst; i <= BCEnum::SpellTypeLast; ++i)
			helper_bots_string(static_cast<BCEnum::SpType>(i), bot_levels_map[static_cast<BCEnum::SpType>(i)]);
	}

	static void status_report() {
		Log.Out(Logs::General, Logs::Commands, "load_bot_command_spells(): - 'RuleI(Bots, CommandSpellRank)' set to %i.", RuleI(Bots, CommandSpellRank));
		if (bot_command_spells.empty()) {
			Log.Out(Logs::General, Logs::Error, "load_bot_command_spells() - 'bot_command_spells' is empty.");
			return;
		}

		for (int i = BCEnum::SpellTypeFirst; i <= BCEnum::SpellTypeLast; ++i)
			Log.Out(Logs::General, Logs::Commands, "load_bot_command_spells(): - '%s' returned %u spell entries.",
				BCEnum::SpellTypeEnumToString(static_cast<BCEnum::SpType>(i)).c_str(), bot_command_spells[static_cast<BCEnum::SpType>(i)].size());
	}

	static void helper_bots_string(BCEnum::SpType type_index, bcst_levels& bot_levels) {
		for (int i = WARRIOR; i <= BERSERKER; ++i)
			required_bots_map_by_class[type_index][i] = "Unavailable...";

		if (bot_levels.empty()) {
			required_bots_map[type_index] = "This command is currently unavailable...";
			return;
		}

		required_bots_map[type_index] = "";

		auto map_size = bot_levels.size();
		while (bot_levels.size()) {
			bcst_levels::iterator test_iter = bot_levels.begin();
			for (bcst_levels::iterator levels_iter = bot_levels.begin(); levels_iter != bot_levels.end(); ++levels_iter) {
				if (levels_iter->second < test_iter->second)
					test_iter = levels_iter;
				if (strcasecmp(Bot::ClassIdToString(levels_iter->first).c_str(), Bot::ClassIdToString(test_iter->first).c_str()) < 0 && levels_iter->second <= test_iter->second)
					test_iter = levels_iter;
			}

			std::string bot_segment;
			if (bot_levels.size() == map_size)
				bot_segment = "%s(%u)";
			else if (bot_levels.size() > 1)
				bot_segment = ", %s(%u)";
			else
				bot_segment = " or %s(%u)";

			required_bots_map[type_index].append(StringFormat(bot_segment.c_str(), Bot::ClassIdToString(test_iter->first).c_str(), test_iter->second));
			required_bots_map_by_class[type_index][test_iter->first] = StringFormat("%s(%u)", Bot::ClassIdToString(test_iter->first).c_str(), test_iter->second);
			bot_levels.erase(test_iter);
		}
	}

#ifdef BCSTSPELLDUMP
	static void spell_dump() {
		std::ofstream spell_dump;
		spell_dump.open(StringFormat("bcs_dump/spell_dump_%i.txt", getpid()), std::ios_base::app | std::ios_base::out);

		if (bot_command_spells.empty()) {
			spell_dump << "BCSpells::spell_dump() - 'bot_command_spells' map is empty.\n";
			spell_dump.close();
			return;
		}

		int entry_count = 0;
		for (int i = BCEnum::SpellTypeFirst; i <= BCEnum::SpellTypeLast; ++i) {
			auto bcst_id = static_cast<BCEnum::SpType>(i);
			spell_dump << StringFormat("BCSpells::spell_dump(): - '%s' returned %u spells:\n",
				BCEnum::SpellTypeEnumToString(bcst_id).c_str(), bot_command_spells[bcst_id].size());
			
			bcst_list& map_entry = bot_command_spells[bcst_id];
			for (auto list_iter = map_entry.begin(); list_iter != map_entry.end(); ++list_iter) {
				STBaseEntry* list_entry = *list_iter;
				int spell_id = list_entry->spell_id;
				spell_dump << StringFormat("\"%20s\" tt:%02u/cc:%02u/cl:%03u",
					((strlen(spells[spell_id].name) > 20) ? (std::string(spells[spell_id].name).substr(0, 20).c_str()) : (spells[spell_id].name)),
					list_entry->target_type,
					list_entry->caster_class,
					list_entry->spell_level
				);

				spell_dump << StringFormat(" /mn:%05u/RD:%06i/zt:%02i/d#:%06i/td#:%05i/ed#:%05i/SAI:%03u",
					spells[spell_id].mana,
					spells[spell_id].ResistDiff,
					spells[spell_id].zonetype,
					spells[spell_id].descnum,
					spells[spell_id].typedescnum,
					spells[spell_id].effectdescnum,
					spells[spell_id].SpellAffectIndex
				);

				for (int i = EffectIDFirst; i <= 3/*EffectIDLast*/; ++i) {
					int effect_index = EFFECTIDTOINDEX(i);
					spell_dump << StringFormat(" /e%02i:%04i/b%02i:%06i/m%02i:%06i",
						i, spells[spell_id].effectid[effect_index], i, spells[spell_id].base[effect_index], i, spells[spell_id].max[effect_index]);
				}

				switch (list_entry->BCST()) {
				case BCEnum::SpT_Charm:
					spell_dump << StringFormat(" /d:%c", ((list_entry->SafeCastToCharm()->dire) ? ('T') : ('F')));
					break;
				case BCEnum::SpT_Cure:
					spell_dump << ' ';
					for (int i = 0; i < BCEnum::AilmentTypeCount; ++i) {
						spell_dump << StringFormat("/cv%02i:%03i", i, list_entry->SafeCastToCure()->cure_value[i]);
					}
					break;
				case BCEnum::SpT_Depart: {
					std::string long_name = list_entry->SafeCastToDepart()->long_name.c_str();
					spell_dump << StringFormat(" /ln:%20s", ((long_name.size() > 20) ? (long_name.substr(0, 20).c_str()) : (long_name.c_str())));
					break;
				}
				case BCEnum::SpT_Escape:
					spell_dump << StringFormat(" /l:%c", ((list_entry->SafeCastToEscape()->lesser) ? ('T') : ('F')));
					break;
				case BCEnum::SpT_Invisibility:
					spell_dump << StringFormat(" /it:%02i", list_entry->SafeCastToInvisibility()->invis_type);
					break;
				case BCEnum::SpT_MovementSpeed:
					spell_dump << StringFormat(" /g:%c", ((list_entry->SafeCastToMovementSpeed()->group) ? ('T') : ('F')));
					break;
				case BCEnum::SpT_Resistance:
					spell_dump << ' ';
					for (int i = 0; i < BCEnum::ResistanceTypeCount; ++i) {
						spell_dump << StringFormat("/rv%02i:%03i", i, list_entry->SafeCastToResistance()->resist_value[i]);
					}
					break;
				case BCEnum::SpT_Resurrect:
					spell_dump << StringFormat(" /aoe:%c", ((list_entry->SafeCastToResurrect()->aoe) ? ('T') : ('F')));
					break;
				case BCEnum::SpT_SendHome:
					spell_dump << StringFormat(" /g:%c", ((list_entry->SafeCastToSendHome()->group) ? ('T') : ('F')));
					break;
				case BCEnum::SpT_Size:
					spell_dump << StringFormat(" /st:%02i", list_entry->SafeCastToSize()->size_type);
					break;
				case BCEnum::SpT_Stance:
					spell_dump << StringFormat(" /st:%02i", list_entry->SafeCastToStance()->stance_type);
					break;
				default:
					break;
				}

				spell_dump << "\n";
				++entry_count;
			}
			
			spell_dump << StringFormat("required_bots_map[%s] = \"%s\"\n",
				BCEnum::SpellTypeEnumToString(static_cast<BCEnum::SpType>(i)).c_str(), required_bots_map[static_cast<BCEnum::SpType>(i)].c_str());

			spell_dump << "\n";
		}

		spell_dump << StringFormat("Total bcs entry count: %i\n", entry_count);
		spell_dump.close();
	}
#endif
};


//struct bcl_struct *bot_command_list;	// the actual linked list of bot commands
int bot_command_count;					// how many bot commands we have

// this is the pointer to the dispatch function, updated once
// init has been performed to point at the real function
int (*bot_command_dispatch)(Client *,char const *) = bot_command_not_avail;

// TODO: Find out what these are for...
void bot_command_bestz(Client *c, const Seperator *message);
void bot_command_pf(Client *c, const Seperator *message);

std::map<std::string, BotCommandRecord *> bot_command_list;
std::map<std::string, std::string> bot_command_aliases;

// All allocated BotCommandRecords get put in here so they get deleted on shutdown
LinkedList<BotCommandRecord *> cleanup_bot_command_list;


/*
 * bot_command_not_avail
 * This is the default dispatch function when commands aren't loaded.
 *
 * Parameters:
 *	not used
 *
 */
int bot_command_not_avail(Client *c, const char *message)
{
	c->Message(m_fail, "Bot commands not available.");
	return -1;
}


/**************************************************************************
/* the rest below here could be in a dynamically loaded module eventually *
/*************************************************************************/

/*

Access Levels:

0		Normal
10	* Steward *
20	* Apprentice Guide *
50	* Guide *
80	* QuestTroupe *
81	* Senior Guide *
85	* GM-Tester *
90	* EQ Support *
95	* GM-Staff *
100	* GM-Admin *
150	* GM-Lead Admin *
160	* QuestMaster *
170	* GM-Areas *
180	* GM-Coder *
200	* GM-Mgmt *
250	* GM-Impossible *

*/

/*
 * bot_command_init
 * initializes the bot command list, call at startup
 *
 * Parameters:
 *	none
 *
 * When adding a new bot command, only hard-code 'real' bot commands -
 * all command aliases are added later through a database call
 *
 */
int bot_command_init(void)
{
	bot_command_aliases.clear();

	if (
		bot_command_add("actionable", "Lists actionable command arguments and use descriptions", 0, bot_command_actionable) ||
		bot_command_add("aggressive", "Orders a bot to use a aggressive discipline", 0, bot_command_aggressive) ||
		bot_command_add("attack", "Orders bots to attack a designated target", 0, bot_command_attack) ||
		bot_command_add("bindaffinity", "Orders a bot to attempt an affinity binding", 0, bot_command_bind_affinity) ||
		bot_command_add("bot", "Lists the available bot management [subcommands]", 0, bot_command_bot) ||
		bot_command_add("botappearance", "Lists the available bot appearance [subcommands]", 0, bot_subcommand_bot_appearance) ||
		bot_command_add("botbeardcolor", "Changes the beard color of a bot", 0, bot_subcommand_bot_beard_color) ||
		bot_command_add("botbeardstyle", "Changes the beard style of a bot", 0, bot_subcommand_bot_beard_style) ||
		bot_command_add("botcamp", "Orders a bot(s) to camp", 0, bot_subcommand_bot_camp) ||
		bot_command_add("botclone", "Creates a copy of a bot", 200, bot_subcommand_bot_clone) ||
		bot_command_add("botcreate", "Creates a new bot", 0, bot_subcommand_bot_create) ||
		bot_command_add("botdelete", "Deletes all record of a bot", 0, bot_subcommand_bot_delete) ||
		bot_command_add("botdetails", "Changes the Drakkin details of a bot", 0, bot_subcommand_bot_details) ||
		bot_command_add("botdyearmor", "Changes the color of a bot's (bots') armor", 0, bot_subcommand_bot_dye_armor) ||
		bot_command_add("boteyes", "Changes the eye colors of a bot", 0, bot_subcommand_bot_eyes) ||
		bot_command_add("botface", "Changes the facial appearance of your bot", 0, bot_subcommand_bot_face) ||
		bot_command_add("botfollowdistance", "Changes the follow distance(s) of a bot(s)", 0, bot_subcommand_bot_follow_distance) ||
		bot_command_add("botgroup", "Lists the available bot-group [subcommands]", 0, bot_command_botgroup) ||
		bot_command_add("botgroupaddmember", "Adds a member to a bot-group", 0, bot_subcommand_botgroup_add_member) ||
		bot_command_add("botgroupcreate", "Creates a bot-group and designates a leader", 0, bot_subcommand_botgroup_create) ||
		bot_command_add("botgroupdelete", "Deletes a bot-group and releases its members", 0, bot_subcommand_botgroup_delete) ||
		bot_command_add("botgrouplist", "Lists all of your existing bot-groups", 0, bot_subcommand_botgroup_list) ||
		bot_command_add("botgroupload", "Loads all members of a bot-group", 0, bot_subcommand_botgroup_load) ||
		bot_command_add("botgroupremovemember", "Removes a bot from its bot-group", 0, bot_subcommand_botgroup_remove_member) ||
		bot_command_add("bothaircolor", "Changes the hair color of a bot", 0, bot_subcommand_bot_hair_color) ||
		bot_command_add("bothairstyle", "Changes the hairstyle of a bot", 0, bot_subcommand_bot_hairstyle) ||
		bot_command_add("botheritage", "Changes the Drakkin heritage of a bot", 0, bot_subcommand_bot_heritage) ||
		bot_command_add("botinspectmessage", "Changes the inspect message of a bot", 0, bot_subcommand_bot_inspect_message) ||
		bot_command_add("botlist", "Lists the bots that you own", 0, bot_subcommand_bot_list) ||
		bot_command_add("botoutofcombat", "Toggles your bot between standard and out-of-combat spell/skill use - if any specialized behaviors exist", 0, bot_subcommand_bot_out_of_combat) ||
		bot_command_add("botreport", "Orders a bot to report its readiness", 0, bot_subcommand_bot_report) ||
		bot_command_add("botspawn", "Spawns a created bot", 0, bot_subcommand_bot_spawn) ||
		bot_command_add("botstance", "Changes the stance of a bot", 0, bot_subcommand_bot_stance) ||
		bot_command_add("botsummon", "Summons bot(s) to your location", 0, bot_subcommand_bot_summon) ||
		bot_command_add("bottattoo", "Changes the Drakkin tattoo of a bot", 0, bot_subcommand_bot_tattoo) ||
		bot_command_add("bottogglearcher", "Toggles a archer bot between melee and ranged weapon use", 0, bot_subcommand_bot_toggle_archer) ||
		bot_command_add("bottogglehelm", "Toggles the helm visibility of a bot between shown and hidden", 0, bot_subcommand_bot_toggle_helm) ||
		bot_command_add("botupdate", "Updates a bot to reflect any level changes that you have experienced", 0, bot_subcommand_bot_update) ||
		bot_command_add("botwoad", "Changes the Barbarian woad of a bot", 0, bot_subcommand_bot_woad) ||
		bot_command_add("charm", "Attempts to have a bot charm your target", 0, bot_command_charm) ||
		bot_command_add("circle", "Orders a Druid bot to open a magical doorway to a specified destination", 0, bot_subcommand_circle) ||
		bot_command_add("cure", "Orders a bot to remove any ailments", 0, bot_command_cure) ||
		bot_command_add("defensive", "Orders a bot to use a defensive discipline", 0, bot_command_defensive) ||
		bot_command_add("depart", "Orders a bot to open a magical doorway to a specified destination", 0, bot_command_depart) ||
		bot_command_add("escape", "Orders a bot to send a target group to a safe location within the zone", 0, bot_command_escape) ||
		bot_command_add("findaliases", "Find available aliases for a bot command", 0, bot_command_find_aliases) ||
		bot_command_add("follow", "Orders bots to follow a designated target", 0, bot_command_follow) ||
		bot_command_add("guard", "Orders bots to guard their current positions", 0, bot_command_guard) ||
		bot_command_add("healrotation", "Lists the available bot heal rotation [subcommands]", 0, bot_command_heal_rotation) ||
		bot_command_add("healrotationadaptivetargeting", "Enables or disables adaptive targeting within the heal rotation instance", 0, bot_subcommand_heal_rotation_adaptive_targeting) ||
		bot_command_add("healrotationaddmember", "Adds a bot to a heal rotation instance", 0, bot_subcommand_heal_rotation_add_member) ||
		bot_command_add("healrotationaddtarget", "Adds target to a heal rotation instance", 0, bot_subcommand_heal_rotation_add_target) ||
		bot_command_add("healrotationadjustcritical", "Adjusts the critial HP limit of the heal rotation instance's Class Armor Type criteria", 0, bot_subcommand_heal_rotation_adjust_critical) ||
		bot_command_add("healrotationadjustsafe", "Adjusts the safe HP limit of the heal rotation instance's Class Armor Type criteria", 0, bot_subcommand_heal_rotation_adjust_safe) ||
		bot_command_add("healrotationcastingoverride", "Enables or disables casting overrides within the heal rotation instance", 0, bot_subcommand_heal_rotation_casting_override) ||
		bot_command_add("healrotationchangeinterval", "Changes casting interval between members within the heal rotation instance", 0, bot_subcommand_heal_rotation_change_interval) ||
		bot_command_add("healrotationclearhot", "Clears the HOT of a heal rotation instance", 0, bot_subcommand_heal_rotation_clear_hot) ||
		bot_command_add("healrotationcleartargets", "Removes all targets from a heal rotation instance", 0, bot_subcommand_heal_rotation_clear_targets) ||
		bot_command_add("healrotationcreate", "Creates a bot heal rotation instance and designates a leader", 0, bot_subcommand_heal_rotation_create) ||
		bot_command_add("healrotationdelete", "Deletes a bot heal rotation entry by leader", 0, bot_subcommand_heal_rotation_delete) ||
		bot_command_add("healrotationfastheals", "Enables or disables fast heals within the heal rotation instance", 0, bot_subcommand_heal_rotation_fast_heals) ||
		bot_command_add("healrotationlist", "Reports heal rotation instance(s) information", 0, bot_subcommand_heal_rotation_list) ||
		bot_command_add("healrotationremovemember", "Removes a bot from a heal rotation instance", 0, bot_subcommand_heal_rotation_remove_member) ||
		bot_command_add("healrotationremovetarget", "Removes target from a heal rotations instance", 0, bot_subcommand_heal_rotation_remove_target) ||
		bot_command_add("healrotationresetlimits", "Resets all Class Armor Type HP limit criteria in a heal rotation to its default value", 0, bot_subcommand_heal_rotation_reset_limits) ||
		bot_command_add("healrotationsave", "Saves a bot heal rotation entry by leader", 0, bot_subcommand_heal_rotation_save) ||
		bot_command_add("healrotationsethot", "Sets the HOT in a heal rotation instance", 0, bot_subcommand_heal_rotation_set_hot) ||
		bot_command_add("healrotationstart", "Starts a heal rotation", 0, bot_subcommand_heal_rotation_start) ||
		bot_command_add("healrotationstop", "Stops a heal rotation", 0, bot_subcommand_heal_rotation_stop) ||
		bot_command_add("help", "List available commands and their description - specify partial command as argument to search", 0, bot_command_help) ||
		bot_command_add("hold", "Suspends a bot's AI processing until released", 0, bot_command_hold) ||
		bot_command_add("identify", "Orders a bot to cast an item identification spell", 0, bot_command_identify) ||
		bot_command_add("inventory", "Lists the available bot inventory [subcommands]", 0, bot_command_inventory) ||
		bot_command_add("inventorygive", "Gives the item on your cursor to a bot", 0, bot_subcommand_inventory_give) ||
		bot_command_add("inventorylist", "Lists all items in a bot's inventory", 0, bot_subcommand_inventory_list) ||
		bot_command_add("inventoryremove", "Removes an item from a bot's inventory", 0, bot_subcommand_inventory_remove) ||
		bot_command_add("inventorywindow", "Displays all items in a bot's inventory in a pop-up window", 0, bot_subcommand_inventory_window) ||
		bot_command_add("invisibility", "Orders a bot to cast a cloak of invisibility, or allow them to be seen", 0, bot_command_invisibility) ||
		bot_command_add("levitation", "Orders a bot to cast a levitation spell", 0, bot_command_levitation) ||
		bot_command_add("lull", "Orders a bot to cast a pacification spell", 0, bot_command_lull) ||
		bot_command_add("mesmerize", "Orders a bot to cast a mesmerization spell", 0, bot_command_mesmerize) ||
		bot_command_add("movementspeed", "Orders a bot to cast a movement speed enhancement spell", 0, bot_command_movement_speed) ||
		bot_command_add("pet", "Lists the available bot pet [subcommands]", 0, bot_command_pet) ||
		bot_command_add("petremove", "Orders a bot to remove its pet", 0, bot_subcommand_pet_remove) ||
		bot_command_add("petsettype", "Orders a Magician bot to use a specified pet type", 0, bot_subcommand_pet_set_type) ||
		bot_command_add("picklock", "Orders a capable bot to pick the lock of the closest door", 0, bot_command_pick_lock) ||
		bot_command_add("portal", "Orders a Wizard bot to open a magical doorway to a specified destination", 0, bot_subcommand_portal) ||
		bot_command_add("pull", "Orders a designated bot to 'pull' an enemy", 0, bot_command_pull) ||
		bot_command_add("release", "Releases a suspended bot's AI processing (with hate list wipe)", 0, bot_command_release) ||
		bot_command_add("resistance", "Orders a bot to cast a specified resistance buff", 0, bot_command_resistance) ||
		bot_command_add("resurrect", "Orders a bot to resurrect a player's (players') corpse(s)", 0, bot_command_resurrect) ||
		bot_command_add("rune", "Orders a bot to cast a rune of protection", 0, bot_command_rune) ||
		bot_command_add("sendhome", "Orders a bot to open a magical doorway home", 0, bot_command_send_home) ||
		bot_command_add("size", "Orders a bot to change a player's size", 0, bot_command_size) ||
		bot_command_add("summoncorpse", "Orders a bot to summon a corpse to its feet", 0, bot_command_summon_corpse) ||
		bot_command_add("taunt", "Toggles taunt use by a bot", 0, bot_command_taunt) ||
		bot_command_add("track", "Orders a capable bot to track enemies", 0, bot_command_track) ||
		bot_command_add("waterbreathing", "Orders a bot to cast a water breathing spell", 0, bot_command_water_breathing)
	) {
		bot_command_deinit();
		return -1;
	}

	std::map<std::string, std::pair<uint8, std::vector<std::string>>> bot_command_settings;
	botdb.LoadBotCommandSettings(bot_command_settings);

	auto working_bcl = bot_command_list;
	for (auto working_bcl_iter : working_bcl) {
		auto bot_command_settings_iter = bot_command_settings.find(working_bcl_iter.first);
		if (bot_command_settings_iter == bot_command_settings.end()) {
			if (working_bcl_iter.second->access == 0)
				Log.Out(Logs::General, Logs::Commands, "bot_command_init(): Warning: Bot Command '%s' defaulting to access level 0!", working_bcl_iter.first.c_str());
			continue;
		}

		working_bcl_iter.second->access = bot_command_settings_iter->second.first;
		Log.Out(Logs::General, Logs::Commands, "bot_command_init(): - Bot Command '%s' set to access level %d.", working_bcl_iter.first.c_str(), bot_command_settings_iter->second.first);
		if (bot_command_settings_iter->second.second.empty())
			continue;

		for (auto alias_iter : bot_command_settings_iter->second.second) {
			if (alias_iter.empty())
				continue;
			if (bot_command_list.find(alias_iter) != bot_command_list.end()) {
				Log.Out(Logs::General, Logs::Commands, "bot_command_init(): Warning: Alias '%s' already exists as a bot command - skipping!", alias_iter.c_str());
				continue;
			}

			bot_command_list[alias_iter] = working_bcl_iter.second;
			bot_command_aliases[alias_iter] = working_bcl_iter.first;

			Log.Out(Logs::General, Logs::Commands, "bot_command_init(): - Alias '%s' added to bot command '%s'.", alias_iter.c_str(), bot_command_aliases[alias_iter].c_str());
		}
	}

	bot_command_dispatch = bot_command_real_dispatch;

	BCSpells::Load();
	
	return bot_command_count;
}


/*
 * bot_command_deinit
 * clears the bot command list, freeing resources
 *
 * Parameters:
 *	none
 *
 */
void bot_command_deinit(void)
{
	bot_command_list.clear();
	bot_command_aliases.clear();

	bot_command_dispatch = bot_command_not_avail;
	bot_command_count = 0;

	BCSpells::Unload();
}


/*
 * bot_command_add
 * adds a bot command to the bot command list; used by bot_command_init
 *
 * Parameters:
 *	bot_command_string	- the command ex: "spawn"
 *	desc				- text description of bot command for #help
 *	access				- default access level required to use command
 *	function			- pointer to function that handles command
 *
 */
int bot_command_add(std::string bot_command_name, const char *desc, int access, BotCmdFuncPtr function)
{
	if (bot_command_name.empty()) {
		Log.Out(Logs::General, Logs::Error, "bot_command_add() - Bot command added with empty name string - check bot_command.cpp.");
		return -1;
	}
	if (function == nullptr) {
		Log.Out(Logs::General, Logs::Error, "bot_command_add() - Bot command '%s' added without a valid function pointer - check bot_command.cpp.", bot_command_name.c_str());
		return -1;
	}
	if (bot_command_list.count(bot_command_name) != 0) {
		Log.Out(Logs::General, Logs::Error, "bot_command_add() - Bot command '%s' is a duplicate bot command name - check bot_command.cpp.", bot_command_name.c_str());
		return -1;
	}
	for (auto iter : bot_command_list) {
		if (iter.second->function != function)
			continue;
		Log.Out(Logs::General, Logs::Error, "bot_command_add() - Bot command '%s' equates to an alias of '%s' - check bot_command.cpp.", bot_command_name.c_str(), iter.first.c_str());
		return -1;
	}

	BotCommandRecord *bcr = new BotCommandRecord;
	bcr->access = access;
	bcr->desc = desc;
	bcr->function = function;

	bot_command_list[bot_command_name] = bcr;
	bot_command_aliases[bot_command_name] = bot_command_name;
	cleanup_bot_command_list.Append(bcr);
	bot_command_count++;

	return 0;
}


/*
 *
 * bot_command_real_dispatch
 * Calls the correct function to process the client's command string.
 * Called from Client::ChannelMessageReceived if message starts with
 * bot command character (^).
 *
 * Parameters:
 *	c			- pointer to the calling client object
 *	message		- what the client typed
 *
 */
int bot_command_real_dispatch(Client *c, const char *message)
{
	Seperator sep(message, ' ', 10, 100, true); // "three word argument" should be considered 1 arg

	bot_command_log_command(c, message);

	std::string cstr(sep.arg[0]+1);

	if(bot_command_list.count(cstr) != 1) {
		return(-2);
	}

	BotCommandRecord *cur = bot_command_list[cstr];
	if(c->Admin() < cur->access){
		c->Message(m_fail, "Your access level is not high enough to use this bot command.");
		return(-1);
	}

	/* QS: Player_Log_Issued_Commands */
	if (RuleB(QueryServ, PlayerLogIssuedCommandes)){
		std::string event_desc = StringFormat("Issued bot command :: '%s' in zoneid:%i instid:%i",  message, c->GetZoneID(), c->GetInstanceID());
		QServ->PlayerLogEvent(Player_Log_Issued_Commands, c->CharacterID(), event_desc);
	}

	if(cur->access >= COMMANDS_LOGGING_MIN_STATUS) {
		Log.Out(Logs::General, Logs::Commands, "%s (%s) used bot command: %s (target=%s)",  c->GetName(), c->AccountName(), message, c->GetTarget()?c->GetTarget()->GetName():"NONE");
	}

	if(cur->function == nullptr) {
		Log.Out(Logs::General, Logs::Error, "Bot command '%s' has a null function\n",  cstr.c_str());
		return(-1);
	} else {
		//dispatch C++ bot command
		cur->function(c, &sep);	// dispatch bot command
	}
	return 0;

}

void bot_command_log_command(Client *c, const char *message)
{
	int admin = c->Admin();

	bool continueevents = false;
	switch (zone->loglevelvar) { //catch failsafe
	case 9: // log only LeadGM
		if ((admin >= 150) && (admin <200))
			continueevents = true;
		break;
	case 8: // log only GM
		if ((admin >= 100) && (admin <150))
			continueevents = true;
		break;
	case 1:
		if ((admin >= 200))
			continueevents = true;
		break;
	case 2:
		if ((admin >= 150))
			continueevents = true;
		break;
	case 3:
		if ((admin >= 100))
			continueevents = true;
		break;
	case 4:
		if ((admin >= 80))
			continueevents = true;
		break;
	case 5:
		if ((admin >= 20))
			continueevents = true;
		break;
	case 6:
		if ((admin >= 10))
			continueevents = true;
		break;
	case 7:
		continueevents = true;
		break;
	default:
		break;
	}

	if (continueevents)
		database.logevents(c->AccountName(), c->AccountID(), admin,c->GetName(), c->GetTarget()?c->GetTarget()->GetName():"None", "BotCommand", message, 1);
}


/*
 * helper functions by use
 */
namespace MyBots
{
	static bool IsMyBot(Client *bot_owner, Mob *my_bot) {
		if (!bot_owner || !my_bot || !my_bot->IsBot())
			return false;

		auto test_bot = my_bot->CastToBot();
		if (!test_bot->GetOwner() || !test_bot->GetOwner()->IsClient() || test_bot->GetOwner()->CastToClient() != bot_owner)
			return false;
		
		return true;
	}
	
	static bool IsMyBotInTargetsGroup(Client *bot_owner, Mob *grouped_bot) {
		if (!bot_owner || !grouped_bot || !grouped_bot->GetGroup() || !IsMyBot(bot_owner, grouped_bot))
			return false;

		auto target_mob = bot_owner->GetTarget();
		if (!target_mob)
			return false;

		if (!target_mob->GetGroup() || (!target_mob->IsClient() && !target_mob->IsBot()))
			return false;
		
		return (grouped_bot->GetGroup() == target_mob->GetGroup());
	}

	static bool IsMyBotInPlayerGroup(Client *bot_owner, Mob *grouped_bot, Client *grouped_player) {
		if (!bot_owner || !grouped_player || !grouped_player->GetGroup() || !grouped_bot || !grouped_bot->GetGroup() || !IsMyBot(bot_owner, grouped_bot))
			return false;

		return (grouped_player->GetGroup() == grouped_bot->GetGroup());
	}

	static void UniquifySBL(std::list<Bot*> &sbl) {
		sbl.remove(nullptr);
		sbl.sort();
		sbl.unique();
	}

	static void PopulateSBL_ByTargetedBot(Client *bot_owner, std::list<Bot*> &sbl, bool clear_list = true) {
		if (clear_list)
			sbl.clear();

		if (IsMyBot(bot_owner, bot_owner->GetTarget()))
			sbl.push_back(bot_owner->GetTarget()->CastToBot());

		if (!clear_list)
			UniquifySBL(sbl);
	}

	static void PopulateSBL_ByNamedBot(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner || !name)
			return;
		
		auto selectable_bot_list = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		for (auto bot_iter : selectable_bot_list) {
			if (!strcasecmp(bot_iter->GetCleanName(), name)) {
				sbl.push_back(bot_iter);
				return;
			}
		}
		
		if (!clear_list)
			UniquifySBL(sbl);
	}

	static void PopulateSBL_ByMyGroupedBots(Client *bot_owner, std::list<Bot*> &sbl, bool clear_list = true) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner)
			return;

		if (!bot_owner->GetGroup())
			return;

		std::list<Bot*> group_list;
		bot_owner->GetGroup()->GetBotList(group_list);
		for (auto member_iter : group_list) {
			if (IsMyBot(bot_owner, member_iter))
				sbl.push_back(member_iter);
		}
		
		if (!clear_list)
			UniquifySBL(sbl);
	}

	static void PopulateSBL_ByTargetsGroupedBots(Client *bot_owner, std::list<Bot*> &sbl, bool clear_list = true) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner)
			return;

		auto target_mob = bot_owner->GetTarget();
		if (!target_mob || !target_mob->GetGroup() || (!target_mob->IsClient() && !target_mob->IsBot()))
			return;

		std::list<Bot*> group_list;
		target_mob->GetGroup()->GetBotList(group_list);
		for (auto member_iter : group_list) {
			if (IsMyBot(bot_owner, member_iter))
				sbl.push_back(member_iter);
		}
		
		if (!clear_list)
			UniquifySBL(sbl);
	}

	static void PopulateSBL_ByNamesGroupedBots(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner || !name)
			return;

		Mob* named_mob = nullptr;
		std::list<Mob*> l_mob_list;
		entity_list.GetMobList(l_mob_list);
		for (auto mob_iter : l_mob_list) {
			if (!strcasecmp(mob_iter->GetCleanName(), name)) {
				named_mob = mob_iter;
				break;
			}
		}
		if (!named_mob || !named_mob->GetGroup() || (!named_mob->IsClient() && !named_mob->IsBot()))
			return;

		std::list<Bot*> group_list;
		named_mob->GetGroup()->GetBotList(group_list);
		for (auto member_iter : group_list) {
			if (IsMyBot(bot_owner, member_iter))
				sbl.push_back(member_iter);
		}
		
		if (!clear_list)
			UniquifySBL(sbl);
	}
	
	static void PopulateSBL_ByHealRotation(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner || (!name && !bot_owner->GetTarget()))
			return;

		std::list<Bot*> selectable_bot_list;
		if (name)
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		else
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember())
			return;

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->MemberList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter))
				sbl.push_back(hrm_iter);
		}

		auto hrt = (*selectable_bot_list.front()->MemberOfHealRotation())->TargetList();
		for (auto hrt_iter : *hrt) {
			if (IsMyBot(bot_owner, hrt_iter))
				sbl.push_back(hrt_iter->CastToBot());
		}

		UniquifySBL(sbl);
	}

	static void PopulateSBL_ByHealRotationMembers(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner || (!name && !bot_owner->GetTarget()))
			return;

		std::list<Bot*> selectable_bot_list;
		if (name)
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		else
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember())
			return;

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->MemberList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter))
				sbl.push_back(hrm_iter);
		}
		
		if (!clear_list)
			UniquifySBL(sbl);
	}
	
	static void PopulateSBL_ByHealRotationTargets(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner || (!name && !bot_owner->GetTarget()))
			return;

		std::list<Bot*> selectable_bot_list;
		if (name)
			PopulateSBL_ByNamedBot(bot_owner, selectable_bot_list, name);
		else
			PopulateSBL_ByTargetedBot(bot_owner, selectable_bot_list);

		if (selectable_bot_list.empty() || !selectable_bot_list.front()->IsHealRotationMember())
			return;

		auto hrm = (*selectable_bot_list.front()->MemberOfHealRotation())->TargetList();
		for (auto hrm_iter : *hrm) {
			if (IsMyBot(bot_owner, hrm_iter))
				sbl.push_back(static_cast<Bot*>(hrm_iter));
		}
		
		if (!clear_list)
			UniquifySBL(sbl);
	}

	static void PopulateSBL_BySpawnedBots(Client *bot_owner, std::list<Bot*> &sbl) { // should be used for most spell casting commands
		sbl.clear();
		if (!bot_owner)
			return;

		sbl = entity_list.GetBotsByBotOwnerCharacterID(bot_owner->CharacterID());
		sbl.remove(nullptr);
	}

	static void PopulateSBL_ByBotGroup(Client *bot_owner, std::list<Bot*> &sbl, const char* name, bool clear_list = true) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner || !name)
			return;

		std::string group_name = name;

		uint32 botgroup_id = 0;
		if (!botdb.LoadBotGroupIDForLoadBotGroup(bot_owner->CharacterID(), group_name, botgroup_id) || !botgroup_id)
			return;

		std::map<uint32, std::list<uint32>> botgroup_list;
		if (!botdb.LoadBotGroup(group_name, botgroup_list) || botgroup_list.find(botgroup_id) == botgroup_list.end() || !botgroup_list[botgroup_id].size())
			return;

		std::list<Bot*> selectable_bot_list;
		PopulateSBL_BySpawnedBots(bot_owner, selectable_bot_list);
		if (selectable_bot_list.empty())
			return;

		selectable_bot_list.remove(nullptr);
		for (auto group_iter : botgroup_list[botgroup_id]) {
			for (auto bot_iter : selectable_bot_list) {
				if (bot_iter->GetBotID() != group_iter)
					continue;

				if (IsMyBot(bot_owner, bot_iter)) {
					sbl.push_back(bot_iter);
					break;
				}
			}
		}

		if (!clear_list)
			UniquifySBL(sbl);
	}
};

namespace ActionableTarget
{
	static bool AmIInPlayerGroup(Client *bot_owner, Client *grouped_player) {
		if (!bot_owner || !grouped_player || !bot_owner->GetGroup() || !grouped_player->GetGroup())
			return false;

		return (bot_owner->GetGroup() == grouped_player->GetGroup());
	}

	static bool IsAttackable(Client *bot_owner, Mob* target_mob) {
		if (!bot_owner || !target_mob || bot_owner == target_mob)
			return false;

		return bot_owner->IsAttackAllowed(target_mob);
	}

	static Client* AsSingle_ByPlayer(Client *bot_owner, bool return_me_on_null_target = true) {
		if (!bot_owner)
			return nullptr;

		if (!bot_owner->GetTarget()) {
			if (return_me_on_null_target)
				return bot_owner;
			else
				return nullptr;
		}

		if (!bot_owner->GetTarget()->IsClient())
			return nullptr;

		return bot_owner->GetTarget()->CastToClient();
	}

	static Client* AsGroupMember_ByPlayer(Client *bot_owner, bool return_me_on_null_target = true) {
		if (!bot_owner)
			return nullptr;

		if (!bot_owner->GetTarget()) {
			if (return_me_on_null_target)
				return bot_owner;
			else
				return nullptr;
		}

		if (!bot_owner->GetTarget()->IsClient() || !AmIInPlayerGroup(bot_owner, bot_owner->GetTarget()->CastToClient()))
			return nullptr;

		return bot_owner->GetTarget()->CastToClient();
	}

	static Corpse* AsCorpse_ByPlayer(Client *bot_owner) {
		if (!bot_owner || !bot_owner->GetTarget() || !bot_owner->GetTarget()->IsPlayerCorpse())
			return nullptr;

		return bot_owner->GetTarget()->CastToCorpse();
	}

	static Mob* AsSingle_ByAttackable(Client *bot_owner) {
		if (!IsAttackable(bot_owner, bot_owner->GetTarget()))
			return nullptr;

		return bot_owner->GetTarget();
	}

	static bool IsFriendlyAllowed(Mob* target_mob) {
		if (!target_mob || target_mob->IsClient() || target_mob->IsBot() || (target_mob->IsPet() && target_mob->GetOwner() && (target_mob->GetOwner()->IsClient() || target_mob->GetOwner()->IsBot())) || target_mob->IsPlayerCorpse())
			return true;

		return false;
	}

	static Mob* VerifyFriendly(Client* bot_owner, BCEnum::TType target_type, bool return_me_on_null_target = true) {
		if (IsAttackable(bot_owner, bot_owner->GetTarget()) || target_type == BCEnum::TT_None)
			return nullptr;

		auto target_mob = bot_owner->GetTarget();
		Mob* verified_friendly = nullptr;
		switch (target_type) {
		case BCEnum::TT_Single:
		case BCEnum::TT_GroupV1:
		case BCEnum::TT_GroupV2:
		case BCEnum::TT_AETarget:
			verified_friendly = target_mob;
			break;
		case BCEnum::TT_Animal:
			if (target_mob && target_mob->GetBodyType() == BT_Animal)
				verified_friendly = target_mob;
			break;
		case BCEnum::TT_Undead:
			if (target_mob && target_mob->GetBodyType() == BT_Undead)
				verified_friendly = target_mob;
			break;
		case BCEnum::TT_Summoned:
			if (target_mob && target_mob->GetBodyType() == BT_Summoned)
				verified_friendly = target_mob;
			break;
		case BCEnum::TT_Plant:
			if (target_mob && target_mob->GetBodyType() == BT_Plant)
				verified_friendly = target_mob;
			break;
		case BCEnum::TT_Corpse:
			if (target_mob && target_mob->IsCorpse())
				verified_friendly = target_mob;
			break;
		default:
			return nullptr;
		}

		if (return_me_on_null_target && !target_mob && !verified_friendly) {
			switch (target_type) {
			case BCEnum::TT_Single:
			case BCEnum::TT_GroupV1:
			case BCEnum::TT_GroupV2:
			case BCEnum::TT_AETarget:
				verified_friendly = bot_owner;
				break;
			default:
				break;
			}
		}

		return verified_friendly;
	}

	static Mob* VerifyEnemy(Client* bot_owner, BCEnum::TType target_type) {
		if (!IsAttackable(bot_owner, bot_owner->GetTarget()) || target_type == BCEnum::TT_None)
			return nullptr;

		auto target_mob = bot_owner->GetTarget();
		Mob* verified_enemy = nullptr;
		switch (target_type) {
		case BCEnum::TT_Animal:
			if (target_mob->GetBodyType() == BT_Animal)
				verified_enemy = target_mob;
			break;
		case BCEnum::TT_Undead:
			if (target_mob->GetBodyType() == BT_Undead)
				verified_enemy = target_mob;
			break;
		case BCEnum::TT_Summoned:
			if (target_mob->GetBodyType() == BT_Summoned)
				verified_enemy = target_mob;
			break;
		case BCEnum::TT_Plant:
			if (target_mob->GetBodyType() == BT_Plant)
				verified_enemy = target_mob;
			break;
		case BCEnum::TT_Single:
		case BCEnum::TT_GroupV1:
		case BCEnum::TT_GroupV2:
		case BCEnum::TT_AETarget:
			verified_enemy = target_mob;
			break;
		case BCEnum::TT_Corpse:
			if (target_mob->IsCorpse())
				verified_enemy = target_mob;
			break;
		default:
			return nullptr;
		}

		return verified_enemy;
	}

	class Types {
		Mob* target[BCEnum::TargetTypeCount];
		bool target_set[BCEnum::TargetTypeCount];

	public:
		Types() { Clear(); }

		void Clear() {
			for (int i = BCEnum::TT_None; i <= BCEnum::TargetTypeLast; ++i) {
				target[i] = nullptr;
				target_set[i] = false;
			}
			target_set[BCEnum::TT_None] = true;
		}

		Mob* Select(Client* bot_owner, BCEnum::TType target_type, bool action_type, bool return_me_on_null_target = true) {
			if (target_set[target_type])
				return target[target_type];

			if (action_type == FRIENDLY)
				target[target_type] = VerifyFriendly(bot_owner, target_type, return_me_on_null_target);
			else
				target[target_type] = VerifyEnemy(bot_owner, target_type);
			target_set[target_type] = true;

			return target[target_type];
		}
	};
};

namespace ActionableBots
{
	enum ABType {
		ABT_None = 0,
		ABT_Target,
		ABT_ByName,
		ABT_OwnerGroup,
		ABT_BotGroup,
		ABT_TargetGroup,
		ABT_NamesGroup,
		ABT_HealRotation,
		ABT_HealRotationMembers,
		ABT_HealRotationTargets,
		ABT_Spawned,
		ABT_All
	};

	enum ABMask {
		ABM_None = 0,
		ABM_Target = (1 << (ABT_Target - 1)),
		ABM_ByName = (1 << (ABT_ByName - 1)),
		ABM_OwnerGroup = (1 << (ABT_OwnerGroup - 1)),
		ABM_BotGroup = (1 << (ABT_BotGroup - 1)),
		ABM_TargetGroup = (1 << (ABT_TargetGroup - 1)),
		ABM_NamesGroup = (1 << (ABT_NamesGroup - 1)),
		ABM_HealRotation = (1 << (ABT_HealRotation - 1)),
		ABM_HealRotationMembers = (1 << (ABT_HealRotationMembers - 1)),
		ABM_HealRotationTargets = (1 << (ABT_HealRotationTargets - 1)),
		ABM_Spawned = (1 << (ABT_Spawned - 1)),
		ABM_All = (1 << (ABT_All - 1)),
		ABM_Spawned_All = (3 << (ABT_Spawned - 1)),
		ABM_NoFilter = ~0,
		// grouped values
		ABM_Type1 = (ABM_Target | ABM_ByName | ABM_OwnerGroup | ABM_BotGroup | ABM_TargetGroup | ABM_NamesGroup | ABM_HealRotationTargets | ABM_Spawned),
		ABM_Type2 = (ABM_ByName | ABM_OwnerGroup | ABM_BotGroup | ABM_NamesGroup | ABM_HealRotation | ABM_Spawned)
	};

	// Populates 'sbl'
	static ABType PopulateSBL(Client* bot_owner, std::string ab_type_arg, std::list<Bot*> &sbl, int ab_mask, const char* name = nullptr, bool clear_list = true, bool suppress_message = false) {
		if (clear_list)
			sbl.clear();
		if (!bot_owner)
			return ABT_None;

		auto ab_type = ABT_None;
		if (!ab_type_arg.compare("target") || ab_type_arg.empty())
			ab_type = ABT_Target;
		else if (!ab_type_arg.compare("byname"))
			ab_type = ABT_ByName;
		else if (!ab_type_arg.compare("ownergroup"))
			ab_type = ABT_OwnerGroup;
		else if (!ab_type_arg.compare("botgroup"))
			ab_type = ABT_BotGroup;
		else if (!ab_type_arg.compare("targetgroup"))
			ab_type = ABT_TargetGroup;
		else if (!ab_type_arg.compare("namesgroup"))
			ab_type = ABT_NamesGroup;
		else if (!ab_type_arg.compare("healrotation"))
			ab_type = ABT_HealRotation;
		else if (!ab_type_arg.compare("healrotationmembers"))
			ab_type = ABT_HealRotationMembers;
		else if (!ab_type_arg.compare("healrotationtargets"))
			ab_type = ABT_HealRotationTargets;
		else if (!ab_type_arg.compare("spawned"))
			ab_type = ABT_Spawned;
		else if (!ab_type_arg.compare("all"))
			ab_type = ABT_All;

		if (ab_type_arg.empty())
			ab_type_arg = "target";

		switch (ab_type) {
		case ABT_Target:
			if (ab_mask & ABM_Target)
				MyBots::PopulateSBL_ByTargetedBot(bot_owner, sbl, clear_list);
			break;
		case ABT_ByName:
			if (ab_mask & ABM_ByName)
				MyBots::PopulateSBL_ByNamedBot(bot_owner, sbl, name, clear_list);
			break;
		case ABT_OwnerGroup:
			if (ab_mask & ABM_OwnerGroup)
				MyBots::PopulateSBL_ByMyGroupedBots(bot_owner, sbl, clear_list);
			break;
		case ABT_BotGroup:
			if (ab_mask & ABM_BotGroup)
				MyBots::PopulateSBL_ByBotGroup(bot_owner, sbl, name, clear_list);
			break;
		case ABT_TargetGroup:
			if (ab_mask & ABM_TargetGroup)
				MyBots::PopulateSBL_ByTargetsGroupedBots(bot_owner, sbl, clear_list);
			break;
		case ABT_NamesGroup:
			if (ab_mask & ABM_NamesGroup)
				MyBots::PopulateSBL_ByNamesGroupedBots(bot_owner, sbl, name, clear_list);
			break;
		case ABT_HealRotation:
			if (ab_mask & ABM_HealRotation)
				MyBots::PopulateSBL_ByHealRotation(bot_owner, sbl, name, clear_list);
			break;
		case ABT_HealRotationMembers:
			if (ab_mask & ABM_HealRotationMembers)
				MyBots::PopulateSBL_ByHealRotationMembers(bot_owner, sbl, name, clear_list);
			break;
		case ABT_HealRotationTargets:
			if (ab_mask & ABM_HealRotationTargets)
				MyBots::PopulateSBL_ByHealRotationTargets(bot_owner, sbl, name, clear_list);
			break;
		case ABT_Spawned:
		case ABT_All:
			if (ab_mask & ABM_Spawned_All)
				MyBots::PopulateSBL_BySpawnedBots(bot_owner, sbl);
			break;
		default:
			break;
		}
		if (sbl.empty() && ab_type != ABT_All) {
			if (suppress_message)
				return ABT_None;

			if (!ab_mask)
				bot_owner->Message(m_fail, "Command passed null 'ActionableBot' criteria");
			else if (ab_mask & ab_type)
				bot_owner->Message(m_fail, "You have no spawned bots meeting this criteria - type: '%s', name: '%s'", ab_type_arg.c_str(), ((name) ? (name) : ("")));
			else
				bot_owner->Message(m_fail, "This command does not allow 'ActionableBot' criteria '%s'", ab_type_arg.c_str());
			return ABT_None;
		}

		return ab_type;
	}

	// Returns single, scoped bot
	static Bot* AsGroupMember_ByClass(Client *bot_owner, Client *bot_grouped_player, uint8 cls, bool petless = false) {
		if (!bot_owner || !bot_grouped_player)
			return nullptr;
		if (!bot_grouped_player->GetGroup())
			return nullptr;

		std::list<Mob*> group_list;
		bot_grouped_player->GetGroup()->GetMemberList(group_list);
		for (auto member_iter : group_list) {
			if (!MyBots::IsMyBot(bot_owner, member_iter))
				continue;
			if (member_iter->GetClass() != cls)
				continue;
			if (petless && member_iter->GetPet())
				continue;

			return static_cast<Bot*>(member_iter);
		}

		return nullptr;
	}

	static Bot* AsGroupMember_ByMinLevelAndClass(Client *bot_owner, Client *bot_grouped_player, uint8 minlvl, uint8 cls, bool petless = false) {
		// This function can be nixed if we can enforce bot level as owner level..and the level check can then be moved to the spell loop in the command function
		if (!bot_owner || !bot_grouped_player)
			return nullptr;
		if (!bot_grouped_player->GetGroup())
			return nullptr;

		std::list<Mob*> group_list;
		bot_grouped_player->GetGroup()->GetMemberList(group_list);
		for (auto member_iter : group_list) {
			if (!MyBots::IsMyBot(bot_owner, member_iter))
				continue;
			if (member_iter->GetLevel() < minlvl || member_iter->GetClass() != cls)
				continue;
			if (petless && member_iter->GetPet())
				continue;

			return static_cast<Bot*>(member_iter);
		}

		return nullptr;
	}

	static Bot* AsSpawned_ByClass(Client *bot_owner, std::list<Bot*> &sbl, uint8 cls, bool petless = false) {
		if (!bot_owner)
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* AsSpawned_ByMinLevelAndClass(Client *bot_owner, std::list<Bot*> &sbl, uint8 minlvl, uint8 cls, bool petless = false) {
		// This function can be nixed if we can enforce bot level as owner level..and the level check can then be moved to the spell loop in the command function
		if (!bot_owner)
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetLevel() < minlvl || bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* AsTarget_ByBot(Client *bot_owner) {
		if (!bot_owner || !MyBots::IsMyBot(bot_owner, bot_owner->GetTarget()))
			return nullptr;

		return bot_owner->GetTarget()->CastToBot();
	}

	static Bot* AsNamed_ByBot(Client *bot_owner, std::string bot_name) {
		if (!bot_owner || bot_name.empty())
			return nullptr;

		std::list<Bot*> selectable_bot_list;
		MyBots::PopulateSBL_BySpawnedBots(bot_owner, selectable_bot_list);
		for (auto bot_iter : selectable_bot_list) {
			if (!bot_name.compare(bot_iter->GetCleanName()))
				return bot_iter;
		}

		return nullptr;
	}

	static Bot* Select_ByClass(Client* bot_owner, BCEnum::TType target_type, std::list<Bot*>& sbl, uint8 cls, Mob* target_mob = nullptr, bool petless = false) {
		if (!bot_owner || sbl.empty())
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;
			if (target_type == BCEnum::TT_GroupV1) {
				if (!target_mob)
					return nullptr;
				else if (bot_iter->GetGroup() != target_mob->GetGroup())
					continue;
			}

			return bot_iter;
		}

		return nullptr;
	}

	static Bot* Select_ByMinLevelAndClass(Client* bot_owner, BCEnum::TType target_type, std::list<Bot*>& sbl, uint8 minlvl, uint8 cls, Mob* target_mob = nullptr, bool petless = false) {
		if (!bot_owner || sbl.empty())
			return nullptr;

		for (auto bot_iter : sbl) {
			if (!MyBots::IsMyBot(bot_owner, bot_iter))
				continue;
			if (bot_iter->GetLevel() < minlvl || bot_iter->GetClass() != cls)
				continue;
			if (petless && bot_iter->GetPet())
				continue;
			if (target_type == BCEnum::TT_GroupV1) {
				if (!target_mob)
					return nullptr;
				else if (bot_iter->GetGroup() != target_mob->GetGroup())
					continue;
			}

			return bot_iter;
		}

		return nullptr;
	}

	// Filters actual 'sbl' list
	static void Filter_ByClasses(Client* bot_owner, std::list<Bot*>& sbl, uint16 class_mask) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		sbl.remove_if([class_mask](const Bot* l) { return (GetPlayerClassBit(l->GetClass()) & (~class_mask)); });
	}

	static void Filter_ByMinLevel(Client* bot_owner, std::list<Bot*>& sbl, uint8 min_level) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		sbl.remove_if([min_level](const Bot* l) { return (l->GetLevel() < min_level); });
	}

	static void Filter_ByArcher(Client* bot_owner, std::list<Bot*>& sbl) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		sbl.remove_if([bot_owner](Bot* l) { return (!l->IsBotArcher()); });
	}
	
	static void Filter_ByHighestSkill(Client* bot_owner, std::list<Bot*>& sbl, EQEmu::skills::SkillType skill_type, float& skill_value) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		skill_value = 0.0f;

		float mod_skill_value = 0.0f;
		const Bot* skilled_bot = nullptr;
		for (auto bot_iter : sbl) {
			float base_skill_value = bot_iter->GetSkill(skill_type);
			if (base_skill_value == 0.0f)
				continue;

			mod_skill_value = base_skill_value;
			for (int16 index = EQEmu::legacy::EQUIPMENT_BEGIN; index <= EQEmu::legacy::EQUIPMENT_END; ++index) {
				const ItemInst* indexed_item = bot_iter->GetBotItem(index);
				if (indexed_item && indexed_item->GetItem()->SkillModType == skill_type)
					mod_skill_value += (base_skill_value * (((float)indexed_item->GetItem()->SkillModValue) / 100.0f));
			}

			if (!skilled_bot) {
				skill_value = mod_skill_value;
				skilled_bot = bot_iter;
			}
			else if (mod_skill_value > skill_value) {
				skill_value = mod_skill_value;
				skilled_bot = bot_iter;
			}
		}

		sbl.remove_if([skilled_bot](const Bot* l) { return (l != skilled_bot); });
	}

	static void Filter_ByHighestPickLock(Client* bot_owner, std::list<Bot*>& sbl, float& pick_lock_value) {
		sbl.remove_if([bot_owner](Bot* l) { return (!MyBots::IsMyBot(bot_owner, l)); });
		sbl.remove_if([bot_owner](const Bot* l) { return (l->GetClass() != ROGUE && l->GetClass() != BARD); });
		sbl.remove_if([bot_owner](const Bot* l) { return (l->GetClass() == ROGUE && l->GetLevel() < 5); });
		sbl.remove_if([bot_owner](const Bot* l) { return (l->GetClass() == BARD && l->GetLevel() < 40); });

		ActionableBots::Filter_ByHighestSkill(bot_owner, sbl, EQEmu::skills::SkillPickLock, pick_lock_value);
	}
};


/*
 * bot commands go below here
 */
void bot_command_actionable(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_actionable", sep->arg[0], "actionable"))
		return;

	c->Message(m_message, "Actionable command arguments:");

	c->Message(m_usage, "target - selects target as single bot .. use ^command [target] or imply by empty actionable argument");
	c->Message(m_usage, "byname [name] - selects single bot by name");
	c->Message(m_usage, "ownergroup - selects all bots in the owner's group");
	c->Message(m_usage, "botgroup [name] - selects members of a bot-group by its name");
	c->Message(m_usage, "targetgroup - selects all bots in target's group");
	c->Message(m_usage, "namesgroup [name] - selects all bots in name's group");
	c->Message(m_usage, "healrotation [name] - selects all member and target bots of a heal rotation where name is a member");
	c->Message(m_usage, "healrotationmembers [name] - selects all member bots of a heal rotation where name is a member");
	c->Message(m_usage, "healrotationtargets [name] - selects all target bots of a heal rotation where name is a member");
	c->Message(m_usage, "spawned - selects all spawned bots");
	c->Message(m_usage, "all - selects all spawned bots .. argument use indicates en masse database updating");

	c->Message(m_message, "You may only select your bots as actionable");
}

void bot_command_aggressive(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Stance];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Stance) || helper_command_alias_fail(c, "bot_command_aggressive", sep->arg[0], "aggressive"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotationtargets | spawned] ([actionable_name]))", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Stance);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type1;

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;
	sbl.remove(nullptr);

	int success_count = 0;
	int candidate_count = sbl.size();
	for (auto list_iter : *local_list) {
		if (sbl.empty())
			break;

		auto local_entry = list_iter->SafeCastToStance();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->stance_type != BCEnum::StT_Aggressive)
			continue;

		for (auto bot_iter = sbl.begin(); bot_iter != sbl.end(); ) {
			Bot* my_bot = *bot_iter;
			if (local_entry->caster_class != my_bot->GetClass()) {
				++bot_iter;
				continue;
			}
			if (local_entry->spell_level > my_bot->GetLevel()) {
				++bot_iter;
				continue;
			}

			my_bot->InterruptSpell();
			if (candidate_count == 1)
				Bot::BotGroupSay(my_bot, "Using '%s'", spells[local_entry->spell_id].name);
			my_bot->UseDiscipline(local_entry->spell_id, my_bot->GetID());
			++success_count;

			bot_iter = sbl.erase(bot_iter);
		}
	}

	c->Message(m_action, "%i of %i bots have used aggressive disciplines", success_count, candidate_count);
}

void bot_command_attack(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_attack", sep->arg[0], "attack"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <enemy_target> %s [actionable: byname | ownergroup | botgroup | namesgroup | healrotation | spawned] ([actionable_name])", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type2;

	Mob* target_mob = ActionableTarget::AsSingle_ByAttackable(c);
	if (!target_mob) {
		c->Message(m_fail, "You must <target> an enemy to use this command");
		return;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	sbl.remove(nullptr);
	for (auto bot_iter : sbl) {
		bot_iter->WipeHateList();
		bot_iter->AddToHateList(target_mob, 1);
		if (!bot_iter->GetPet())
			continue;

		bot_iter->GetPet()->WipeHateList();
		bot_iter->GetPet()->AddToHateList(target_mob, 1);
	}
	if (sbl.size() == 1)
		Bot::BotGroupSay(sbl.front(), "Attacking %s", target_mob->GetCleanName());
	else
		c->Message(m_action, "%i of your bots are attacking %s", sbl.size(), target_mob->GetCleanName());
}

void bot_command_bind_affinity(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_BindAffinity];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_BindAffinity) || helper_command_alias_fail(c, "bot_command_bind_affinity", sep->arg[0], "bindaffinity"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_BindAffinity);
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		// Cast effect message is not being generated
		if (helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id))
			c->Message(m_action, "Successfully bound %s to this location", target_mob->GetCleanName());
		else
			c->Message(m_fail, "Failed to bind %s to this location", target_mob->GetCleanName());
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_bot(Client *c, const Seperator *sep)
{
	/* VS2012 code - begin */
	std::list<const char*> subcommand_list;
	subcommand_list.push_back("botappearance");
	subcommand_list.push_back("botcamp");
	subcommand_list.push_back("botclone");
	subcommand_list.push_back("botcreate");
	subcommand_list.push_back("botdelete");
	subcommand_list.push_back("botdetails");
	subcommand_list.push_back("botdyearmor");
	subcommand_list.push_back("botinspectmessage");
	subcommand_list.push_back("botfollowdistance");
	subcommand_list.push_back("botlist");
	subcommand_list.push_back("botoutofcombat");
	subcommand_list.push_back("botreport");
	subcommand_list.push_back("botspawn");
	subcommand_list.push_back("botstance");
	subcommand_list.push_back("botsummon");
	subcommand_list.push_back("bottogglearcher");
	subcommand_list.push_back("bottogglehelm");
	subcommand_list.push_back("botupdate");
	/* VS2012 code - end */

	/* VS2013 code
	const std::list<const char*> subcommand_list = {
		"botappearance", "botcamp", "botclone", "botcreate", "botdelete", "botdetails", "botdyearmor", "botinspectmessage", "botfollowdistance",
		"botlist", "botoutofcombat", "botreport", "botspawn", "botstance", "botsummon", "bottogglearcher", "bottogglehelm", "botupdate"
	};
	*/
	
	if (helper_command_alias_fail(c, "bot_command_bot", sep->arg[0], "bot"))
		return;

	helper_send_available_subcommands(c, "bot", subcommand_list);
}

void bot_command_botgroup(Client *c, const Seperator *sep)
{
	/* VS2012 code - begin */
	std::list<const char*> subcommand_list;
	subcommand_list.push_back("botgroupaddmember");
	subcommand_list.push_back("botgroupcreate");
	subcommand_list.push_back("botgroupdelete");
	subcommand_list.push_back("botgrouplist");
	subcommand_list.push_back("botgroupload");
	subcommand_list.push_back("botgroupremovemember");
	/* VS2012 code - end */

	/* VS2013 code
	const std::list<const char*> subcommand_list = {
		"botgroupaddmember", "botgroupcreate", "botgroupdelete", "botgrouplist", "botgroupload", "botgroupremovemember"
	};
	*/
	
	if (helper_command_alias_fail(c, "bot_command_botgroup", sep->arg[0], "botgroup"))
		return;

	helper_send_available_subcommands(c, "bot-group", subcommand_list);
}

void bot_command_charm(Client *c, const Seperator *sep)
{
	auto local_list = &bot_command_spells[BCEnum::SpT_Charm];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Charm) || helper_command_alias_fail(c, "bot_command_charm", sep->arg[0], "charm"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <enemy_target> %s ([option: dire])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Charm);
		return;
	}

	bool dire = false;
	std::string dire_arg = sep->arg[1];
	if (!dire_arg.compare("dire"))
		dire = true;

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToCharm();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->dire != dire)
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, ENEMY);
		if (!target_mob)
			continue;
		if (target_mob->IsCharmed()) {
			c->Message(m_fail, "Your <target> is already charmed");
			return;
		}

		if (spells[local_entry->spell_id].max[EFFECTIDTOINDEX(1)] < target_mob->GetLevel())
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob, true);
		if (!my_bot)
			continue;

		uint32 dont_root_before = 0;
		if (helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id, true, &dont_root_before))
			target_mob->SetDontRootMeBefore(dont_root_before);

		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_cure(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Cure];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Cure) || helper_command_alias_fail(c, "bot_command_cure", sep->arg[0], "cure"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s [ailment: blindness | disease | poison | curse | corruption]", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Cure);
		return;
	}

	std::string ailment_arg = sep->arg[1];

	auto ailment_type = BCEnum::AT_None;
	if (!ailment_arg.compare("blindness"))
		ailment_type = BCEnum::AT_Blindness;
	else if (!ailment_arg.compare("disease"))
		ailment_type = BCEnum::AT_Disease;
	else if (!ailment_arg.compare("poison"))
		ailment_type = BCEnum::AT_Poison;
	else if (!ailment_arg.compare("curse"))
		ailment_type = BCEnum::AT_Curse;
	else if (!ailment_arg.compare("corruption"))
		ailment_type = BCEnum::AT_Corruption;

	if (ailment_type == BCEnum::AT_None) {
		c->Message(m_fail, "You must specify a cure [ailment] to use this command");
		return;
	}

	local_list->sort([ailment_type](STBaseEntry* l, STBaseEntry* r) {
		auto _l = l->SafeCastToCure(), _r = r->SafeCastToCure();
		if (_l->cure_value[AILMENTIDTOINDEX(ailment_type)] < _r->cure_value[AILMENTIDTOINDEX(ailment_type)])
			return true;
		if (_l->cure_value[AILMENTIDTOINDEX(ailment_type)] == _r->cure_value[AILMENTIDTOINDEX(ailment_type)] && spells[_l->spell_id].mana < spells[_r->spell_id].mana)
			return true;
		if (_l->cure_value[AILMENTIDTOINDEX(ailment_type)] == _r->cure_value[AILMENTIDTOINDEX(ailment_type)] && spells[_l->spell_id].mana == spells[_r->spell_id].mana && _l->cure_total < _r->cure_total)
			return true;

		return false;
	});

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToCure();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (!local_entry->cure_value[AILMENTIDTOINDEX(ailment_type)])
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_defensive(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Stance];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Stance) || helper_command_alias_fail(c, "bot_command_defensive", sep->arg[0], "defensive"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotationtargets | spawned] ([actionable_name]))", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Stance);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type1;
	
	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;
	sbl.remove(nullptr);

	int success_count = 0;
	int candidate_count = sbl.size();
	for (auto list_iter : *local_list) {
		if (sbl.empty())
			break;

		auto local_entry = list_iter->SafeCastToStance();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->stance_type != BCEnum::StT_Aggressive)
			continue;

		for (auto bot_iter = sbl.begin(); bot_iter != sbl.end(); ) {
			Bot* my_bot = *bot_iter;
			if (local_entry->caster_class != my_bot->GetClass()) {
				++bot_iter;
				continue;
			}
			if (local_entry->spell_level > my_bot->GetLevel()) {
				++bot_iter;
				continue;
			}

			my_bot->InterruptSpell();
			if (candidate_count == 1)
				Bot::BotGroupSay(my_bot, "Using '%s'", spells[local_entry->spell_id].name);
			my_bot->UseDiscipline(local_entry->spell_id, my_bot->GetID());
			++success_count;

			bot_iter = sbl.erase(bot_iter);
		}
	}

	c->Message(m_action, "%i of %i bots have used defensive disciplines", success_count, candidate_count);
}

void bot_command_depart(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Depart];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Depart) || helper_command_alias_fail(c, "bot_command_depart", sep->arg[0], "depart"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [list | destination] ([option: single])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Depart);
		return;
	}

	bool single = false;
	std::string single_arg = sep->arg[2];
	if (!single_arg.compare("single"))
		single = true;

	std::string destination = sep->arg[1];
	if (!destination.compare("list")) {
		Bot* my_druid_bot = ActionableBots::AsGroupMember_ByClass(c, c, DRUID);
		Bot* my_wizard_bot = ActionableBots::AsGroupMember_ByClass(c, c, WIZARD);
		helper_command_depart_list(c, my_druid_bot, my_wizard_bot, local_list, single);
		return;
	}
	else if (destination.empty()) {
		c->Message(m_fail, "A [destination] or [list] argument is required to use this command");
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToDepart();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->single != single)
			continue;
		if (destination.compare(spells[local_entry->spell_id].teleport_zone))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_escape(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Escape];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Escape) || helper_command_alias_fail(c, "bot_command_escape", sep->arg[0], "escape"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s ([option: lesser])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Escape);
		return;
	}

	bool use_lesser = false;
	if (!strcasecmp(sep->arg[1], "lesser"))
		use_lesser = true;

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToEscape();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->lesser != use_lesser)
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_find_aliases(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_find_aliases", sep->arg[0], "findaliases"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [alias | command]", sep->arg[0]);
		return;
	}
	
	auto find_iter = bot_command_aliases.find(sep->arg[1]);
	if (find_iter == bot_command_aliases.end()) {
		c->Message(m_fail, "No bot commands or aliases match '%s'", sep->arg[1]);
		return;
	}

	auto command_iter = bot_command_list.find(find_iter->second);
	if (find_iter->second.empty() || command_iter == bot_command_list.end()) {
		c->Message(m_unknown, "An unknown condition has occurred...");
		return;
	}

	c->Message(m_message, "Available bot command aliases for '%s':", command_iter->first.c_str());

	int bot_command_aliases_shown = 0;
	for (auto alias_iter : bot_command_aliases) {
		if (strcasecmp(find_iter->second.c_str(), alias_iter.second.c_str()) || c->Admin() < command_iter->second->access)
			continue;

		c->Message(m_usage, "%c%s", BOT_COMMAND_CHAR, alias_iter.first.c_str());
		++bot_command_aliases_shown;
	}
	c->Message(m_message, "%d bot command alias%s listed.", bot_command_aliases_shown, bot_command_aliases_shown != 1 ? "es" : "");
}

void bot_command_follow(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_follow", sep->arg[0], "follow"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s ([option: reset]) [actionable: byname | ownergroup | botgroup | namesgroup | healrotation | spawned] ([actionable_name])", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type2;

	bool reset = false;
	int ab_arg = 1;
	int name_arg = 2;
	Mob* target_mob = nullptr;

	std::string reset_arg = sep->arg[1];
	if (!reset_arg.compare("reset")) {
		reset = true;
		ab_arg = 2;
		name_arg = 3;
	}
	else {
		target_mob = ActionableTarget::VerifyFriendly(c, BCEnum::TT_Single);
		if (!target_mob) {
			c->Message(m_fail, "You must <target> a friendly mob to use this command");
			return;
		}
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[name_arg]) == ActionableBots::ABT_None)
		return;
	
	sbl.remove(nullptr);
	for (auto bot_iter : sbl) {
		bot_iter->WipeHateList();
		auto my_group = bot_iter->GetGroup();
		if (my_group) {
			if (reset) {
				if (!my_group->GetLeader() || my_group->GetLeader() == bot_iter)
					bot_iter->SetFollowID(c->GetID());
				else
					bot_iter->SetFollowID(my_group->GetLeader()->GetID());
			}
			else {
				if (bot_iter == target_mob)
					bot_iter->SetFollowID(c->GetID());
				else
					bot_iter->SetFollowID(target_mob->GetID());
			}
		}
		else {
			bot_iter->SetFollowID(0);
		}
		if (!bot_iter->GetPet())
			continue;

		bot_iter->GetPet()->WipeHateList();
		bot_iter->GetPet()->SetFollowID(bot_iter->GetID());
	}
	if (sbl.size() == 1) {
		Mob* follow_mob = entity_list.GetMob(sbl.front()->GetFollowID());
		Bot::BotGroupSay(sbl.front(), "Following %s", ((follow_mob) ? (follow_mob->GetCleanName()) : ("'nullptr'")));
	}
	else {
		if (reset)
			c->Message(m_action, "%i of your bots are following their default assignments", sbl.size());
		else
			c->Message(m_action, "%i of your bots are following %s", sbl.size(), target_mob->GetCleanName());
	}
}

void bot_command_guard(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_guard", sep->arg[0], "guard"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [actionable: target | byname | ownergroup | botgroup | namesgroup | healrotation | spawned] ([actionable_name])", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_Type2);

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	sbl.remove(nullptr);
	for (auto bot_iter : sbl) {
		bot_iter->WipeHateList();
		bot_iter->SetFollowID(0);
		if (!bot_iter->GetPet())
			continue;

		bot_iter->GetPet()->WipeHateList();
		bot_iter->GetPet()->SetFollowID(0);
	}
	if (sbl.size() == 1)
		Bot::BotGroupSay(sbl.front(), "Guarding this position");
	else
		c->Message(m_action, "%i of your bots are guarding their positions", sbl.size());
}

void bot_command_heal_rotation(Client *c, const Seperator *sep)
{
	/* VS2012 code - begin */
	std::list<const char*> subcommand_list;
	subcommand_list.push_back("healrotationadaptivetargeting");
	subcommand_list.push_back("healrotationaddmember");
	subcommand_list.push_back("healrotationaddtarget");
	subcommand_list.push_back("healrotationadjustcritical");
	subcommand_list.push_back("healrotationadjustsafe");
	subcommand_list.push_back("healrotationcastoverride");
	subcommand_list.push_back("healrotationchangeinterval");
	subcommand_list.push_back("healrotationclearhot");
	subcommand_list.push_back("healrotationcleartargets");
	subcommand_list.push_back("healrotationcreate");
	subcommand_list.push_back("healrotationdelete");
	subcommand_list.push_back("healrotationfastheals");
	subcommand_list.push_back("healrotationlist");
	subcommand_list.push_back("healrotationremovemember");
	subcommand_list.push_back("healrotationremovetarget");
	subcommand_list.push_back("healrotationresetlimits");
	subcommand_list.push_back("healrotationsave");
	subcommand_list.push_back("healrotationsethot");
	subcommand_list.push_back("healrotationstart");
	subcommand_list.push_back("healrotationstop");
	/* VS2012 code - end */
	
	/* VS2013 code
	const std::list<const char*> subcommand_list = {
		"healrotationadaptivetargeting", "healrotationaddmember", "healrotationaddtarget", "healrotationadjustcritical", "healrotationadjustsafe",
		"healrotationcastoverride", "healrotationchangeinterval", "healrotationclearhot", "healrotationcleartargets", "healrotationcreate",
		"healrotationdelete", "healrotationfastheals", "healrotationlist", "healrotationremovemember", "healrotationremovetarget", "healrotationsave",
		"healrotationresetlimits", "healrotationsethot", "healrotationstart", "healrotationstop"
	};
	*/
	
	if (helper_command_alias_fail(c, "bot_command_heal_rotation", sep->arg[0], "healrotation"))
		return;

#if (EQDEBUG >= 12)
	while (c->Admin() >= 250) {
		if (strcasecmp(sep->arg[1], "shone")) { break; }
		Bot* my_bot = ActionableBots::AsTarget_ByBot(c);
		if (!my_bot || !(my_bot->IsHealRotationMember())) { break; }
		auto tlist = (*my_bot->MemberOfHealRotation())->TargetList();
		if (tlist->empty()) { break; }
		for (auto tlist_iter : *tlist) {
			if (tlist_iter)
				tlist_iter->SetHP((tlist_iter->GetMaxHP() / 100 + 1));
		}
		return;
	}
#endif

	helper_send_available_subcommands(c, "bot heal rotation", subcommand_list);
}

void bot_command_help(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_help", sep->arg[0], "help"))
		return;
	
	c->Message(m_message, "Available EQEMu bot commands:");

	int bot_commands_shown = 0;
	for (auto command_iter : bot_command_list) {
		if (sep->arg[1][0] && command_iter.first.find(sep->arg[1]) == std::string::npos)
			continue;
		if (c->Admin() < command_iter.second->access)
			continue;

		c->Message(m_usage, "%c%s - %s", BOT_COMMAND_CHAR, command_iter.first.c_str(), command_iter.second->desc == nullptr ? "[no description]" : command_iter.second->desc);
		++bot_commands_shown;
	}
	c->Message(m_message, "%d bot command%s listed.", bot_commands_shown, bot_commands_shown != 1 ? "s" : "");
	c->Message(m_note, "type %ccommand [help | usage] for more information", BOT_COMMAND_CHAR);
}

void bot_command_hold(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_hold", sep->arg[0], "hold"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: <any>] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	sbl.remove(nullptr);
	for (auto bot_iter : sbl)
		bot_iter->SetPauseAI(true);
	
	c->Message(m_action, "%i of your bots %s suspended", sbl.size(), ((sbl.size() != 1) ? ("are") : ("is")));
}

void bot_command_identify(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Identify];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Identify) || helper_command_alias_fail(c, "bot_command_identify", sep->arg[0], "identify"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Identify);
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_inventory(Client *c, const Seperator *sep)
{
	/* VS2012 code - begin */
	std::list<const char*> subcommand_list;
	subcommand_list.push_back("inventorygive");
	subcommand_list.push_back("inventorylist");
	subcommand_list.push_back("inventoryremove");
	subcommand_list.push_back("inventorywindow");
	/* VS2012 code - end */

	/* VS2013 code
	const std::list<const char*> subcommand_list = { "inventorygive", "inventorylist", "inventoryremove", "inventorywindow" };
	*/

	if (helper_command_alias_fail(c, "bot_command_inventory", sep->arg[0], "inventory"))
		return;

	helper_send_available_subcommands(c, "bot inventory", subcommand_list);
}

void bot_command_invisibility(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Invisibility];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Invisibility) || helper_command_alias_fail(c, "bot_command_invisibility", sep->arg[0], "invisibility"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s [invisibility: living | undead | animal | see]", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Invisibility);
		return;
	}

	std::string invisibility = sep->arg[1];

	BCEnum::IType invisibility_type = BCEnum::IT_None;
	if (!invisibility.compare("living"))
		invisibility_type = BCEnum::IT_Living;
	else if (!invisibility.compare("undead"))
		invisibility_type = BCEnum::IT_Undead;
	else if (!invisibility.compare("animal"))
		invisibility_type = BCEnum::IT_Animal;
	else if (!invisibility.compare("see"))
		invisibility_type = BCEnum::IT_See;

	if (invisibility_type == BCEnum::IT_None) {
		c->Message(m_fail, "You must specify an [invisibility]");
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToInvisibility();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->invis_type != invisibility_type)
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_levitation(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Levitation];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Levitation) || helper_command_alias_fail(c, "bot_command_levitation", sep->arg[0], "levitation"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Levitation);
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_lull(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Lull];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Lull) || helper_command_alias_fail(c, "bot_command_lull", sep->arg[0], "lull"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <enemy_target> %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Lull);
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, ENEMY);
		if (!target_mob)
			continue;

		//if (spells[local_entry->spell_id].max[EFFECTIDTOINDEX(3)] && spells[local_entry->spell_id].max[EFFECTIDTOINDEX(3)] < target_mob->GetLevel())
		//	continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		uint32 dont_root_before = 0;
		if (helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id, true, &dont_root_before))
			target_mob->SetDontRootMeBefore(dont_root_before);

		break;
	}

	helper_no_available_bots(c, my_bot);
}

void bot_command_mesmerize(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Mesmerize];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Mesmerize) || helper_command_alias_fail(c, "bot_command_mesmerize", sep->arg[0], "mesmerize"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <enemy_target> %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Mesmerize);
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, ENEMY);
		if (!target_mob)
			continue;
		
		if (spells[local_entry->spell_id].max[EFFECTIDTOINDEX(1)] < target_mob->GetLevel())
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		uint32 dont_root_before = 0;
		if (helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id, true, &dont_root_before))
			target_mob->SetDontRootMeBefore(dont_root_before);

		break;
	}

	helper_no_available_bots(c, my_bot);
}

void bot_command_movement_speed(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_MovementSpeed];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_MovementSpeed) || helper_command_alias_fail(c, "bot_command_movement_speed", sep->arg[0], "movementspeed"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s ([group])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_MovementSpeed);
		return;
	}

	bool group = false;
	std::string group_arg = sep->arg[1];
	if (!group_arg.compare("group"))
		group = true;

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToMovementSpeed();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->group != group)
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_pet(Client *c, const Seperator *sep)
{
	/* VS2012 code - begin */
	std::list<const char*> subcommand_list;
	subcommand_list.push_back("petremove");
	subcommand_list.push_back("petsettype");
	/* VS2012 code - end */
	
	/* VS2013 code
	const std::list<const char*> subcommand_list = { "petremove", "petsettype" };
	*/

	if (helper_command_alias_fail(c, "bot_command_pet", sep->arg[0], "pet"))
		return;

	helper_send_available_subcommands(c, "bot pet", subcommand_list);
}

void bot_command_pick_lock(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pick_lock", sep->arg[0], "picklock"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s", sep->arg[0]);
		c->Message(m_note, "requires one of the following bot classes:");
		c->Message(m_message, "Rogue(5) or Bard(40)");
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	float pick_lock_value = 0.0f;
	ActionableBots::Filter_ByHighestPickLock(c, sbl, pick_lock_value);
	if (sbl.empty()) {
		c->Message(m_fail, "No bots are capable of performing this action");
		return;
	}

	Bot* my_bot = sbl.front();
	
	my_bot->InterruptSpell();
	Bot::BotGroupSay(my_bot, "Attempting to pick the lock..");

	std::list<Doors*> door_list;
	entity_list.GetDoorsList(door_list);

	int door_count = 0, open_count = 0;
	for (auto door_iter : door_list) {
		if (!door_iter || door_iter->IsDoorOpen())
			continue;

		glm::tvec4<float, glm::highp> diff = (c->GetPosition() - door_iter->GetPosition());

		float curdist = ((diff.x * diff.x) + (diff.y * diff.y));
		float curelev = (diff.z * diff.z);
#if (EQDEBUG >= 11)
		if (curdist <= 130 && curelev <= 65 && curelev >= 25) // 2D limit is '130' (x^2 + y^2), 1D theoretically should be '65' (z^2)
			Log.Out(Logs::Detail, Logs::Doors, "bot_command_pick_lock(): DoorID: %i - Elevation difference failure within theoretical limit (%f <= 65.0)", door_iter->GetDoorID(), curelev);
#endif
		if (curelev >= 25 || curdist > 130) // changed curelev from '10' to '25' - requiring diff.z to be less than '5'
			continue;

		++door_count;
		if (pick_lock_value >= door_iter->GetLockpick()) {
			door_iter->ForceOpen(my_bot);
			++open_count;
		}
		else {
			Bot::BotGroupSay(my_bot, "I am not skilled enough for this lock...");
		}
	}
	c->Message(m_action, "%i door%s attempted - %i door%s successful", door_count, ((door_count != 1) ? ("s") : ("")), open_count, ((open_count != 1) ? ("s") : ("")));
}

void bot_command_pull(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pull", sep->arg[0], "pull"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <enemy_target> %s", sep->arg[0]);
		return;
	}
	int ab_mask = ActionableBots::ABM_OwnerGroup; // existing behavior - need to add c->IsGrouped() check and modify code if different behavior is desired

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, "ownergroup", sbl, ab_mask) == ActionableBots::ABT_None)
		return;
	sbl.remove(nullptr);

	auto target_mob = ActionableTarget::VerifyEnemy(c, BCEnum::TT_Single);
	if (!target_mob) {
		c->Message(m_fail, "Your current target is not attackable");
		return;
	}

	Bot* bot_puller = nullptr;
	for (auto bot_iter : sbl) {
		if (!bot_iter->IsArcheryRange(target_mob))
			continue;
		
		Bot::BotGroupSay(bot_iter, "Attempting to pull %s..", target_mob->GetCleanName());
		bot_iter->InterruptSpell();
		bot_iter->BotRangedAttack(target_mob);
		bot_puller = bot_iter;
		break;
	}
	
	helper_no_available_bots(c, bot_puller);
}

void bot_command_release(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_release", sep->arg[0], "release"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: <any>] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	sbl.remove(nullptr);
	for (auto bot_iter : sbl) {
		bot_iter->WipeHateList();
		bot_iter->SetPauseAI(false);
	}

	c->Message(m_action, "%i of your bots %s unsuspended", sbl.size(), ((sbl.size() != 1) ? ("are") : ("is")));
}

void bot_command_resistance(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Resistance];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Resistance) || helper_command_alias_fail(c, "bot_command_resistance", sep->arg[0], "resistance"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s [resistance: fire | cold | poison | disease | magic | corruption]", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Resistance);
		return;
	}

	std::string resistance_arg = sep->arg[1];

	auto resistance_type = BCEnum::RT_None;
	if (!resistance_arg.compare("fire"))
		resistance_type = BCEnum::RT_Fire;
	else if (!resistance_arg.compare("cold"))
		resistance_type = BCEnum::RT_Cold;
	else if (!resistance_arg.compare("poison"))
		resistance_type = BCEnum::RT_Poison;
	else if (!resistance_arg.compare("disease"))
		resistance_type = BCEnum::RT_Disease;
	else if (!resistance_arg.compare("magic"))
		resistance_type = BCEnum::RT_Magic;
	else if (!resistance_arg.compare("corruption"))
		resistance_type = BCEnum::RT_Corruption;

	if (resistance_type == BCEnum::RT_None) {
		c->Message(m_fail, "You must specify a [resistance]");
		return;
	}

	local_list->sort([resistance_type](STBaseEntry* l, STBaseEntry* r) {
		auto _l = l->SafeCastToResistance(), _r = r->SafeCastToResistance();
		if (_l->resist_value[RESISTANCEIDTOINDEX(resistance_type)] > _r->resist_value[RESISTANCEIDTOINDEX(resistance_type)])
			return true;
		if (_l->resist_value[RESISTANCEIDTOINDEX(resistance_type)] == _r->resist_value[RESISTANCEIDTOINDEX(resistance_type)] && spells[_l->spell_id].mana < spells[_r->spell_id].mana)
			return true;
		if (_l->resist_value[RESISTANCEIDTOINDEX(resistance_type)] == _r->resist_value[RESISTANCEIDTOINDEX(resistance_type)] && spells[_l->spell_id].mana == spells[_r->spell_id].mana && _l->resist_total > _r->resist_total)
			return true;
		
		return false;
	});

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToResistance();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (!local_entry->resist_value[RESISTANCEIDTOINDEX(resistance_type)])
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_resurrect(Client *c, const Seperator *sep)
{
	// Obscure bot spell code prohibits the aoe portion from working correctly...
	
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Resurrect];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Resurrect) || helper_command_alias_fail(c, "bot_command_resurrect", sep->arg[0], "resurrect"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		//c->Message(m_usage, "usage: <corpse_target> %s ([option: aoe])", sep->arg[0]);
		c->Message(m_usage, "usage: <corpse_target> %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Resurrect);
		return;
	}

	bool aoe = false;
	//std::string aoe_arg = sep->arg[1];
	//if (!aoe_arg.compare("aoe"))
	//	aoe = true;

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToResurrect();
		if (helper_spell_check_fail(local_entry))
			continue;
		//if (local_entry->aoe != aoe)
		//	continue;
		if (local_entry->aoe)
			continue;
		
		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		//if (!target_mob && !local_entry->aoe)
		//	continue;
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		//if (local_entry->aoe)
		//	target_mob = my_bot;

		uint32 dont_root_before = 0;
		if (helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id, true, &dont_root_before))
			target_mob->SetDontRootMeBefore(dont_root_before);

		break;
	}

	helper_no_available_bots(c, my_bot);
}

void bot_command_rune(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Rune];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Rune) || helper_command_alias_fail(c, "bot_command_rune", sep->arg[0], "rune"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Rune);
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_send_home(Client *c, const Seperator *sep)
{
	// Obscure bot spell code prohibits the aoe portion from working correctly...
	
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_SendHome];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_SendHome) || helper_command_alias_fail(c, "bot_command_send_home", sep->arg[0], "sendhome"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s ([option: group])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_SendHome);
		return;
	}

	bool group = false;
	std::string group_arg = sep->arg[1];
	if (!group_arg.compare("group"))
		group = true;

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToSendHome();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->group != group)
			continue;
		
		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}

	helper_no_available_bots(c, my_bot);
}

void bot_command_size(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Size];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Size) || helper_command_alias_fail(c, "bot_command_size", sep->arg[0], "size"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s [grow | shrink]", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Size);
		return;
	}

	std::string size_arg = sep->arg[1];
	auto size_type = BCEnum::SzT_Reduce;
	if (!size_arg.compare("grow")) {
		size_type = BCEnum::SzT_Enlarge;
	}
	else if (size_arg.compare("shrink")) {
		c->Message(m_fail, "This command requires a [grow | shrink] argument");
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToSize();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->size_type != size_type)
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_command_summon_corpse(Client *c, const Seperator *sep)
{
	// Same methodology as old command..but, does not appear to work... (note: didn't work there, either...)

	// temp
	c->Message(m_fail, "This command is currently unavailable...");
	return;
	
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_SummonCorpse];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_SummonCorpse) || helper_command_alias_fail(c, "bot_command_summon_corpse", sep->arg[0], "summoncorpse"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <friendly_target> %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_SummonCorpse);
		return;
	}

	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;
		
		auto target_mob = ActionableTarget::AsSingle_ByPlayer(c);
		if (!target_mob)
			continue;

		if (spells[local_entry->spell_id].base[EFFECTIDTOINDEX(1)] < target_mob->GetLevel())
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		
		break;
	}

	helper_no_available_bots(c, my_bot);
}

void bot_command_taunt(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_taunt", sep->arg[0], "taunt"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([option: on | off]) ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotationtargets | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type1;

	std::string arg1 = sep->arg[1];

	bool taunt_state = false;
	bool toggle_taunt = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		taunt_state = true;
		toggle_taunt = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_taunt = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]) == ActionableBots::ABT_None)
		return;
	sbl.remove(nullptr);
	
	int taunting_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter->GetSkill(EQEmu::skills::SkillTaunt))
			continue;
		
		if (toggle_taunt)
			bot_iter->SetTaunting(!bot_iter->IsTaunting());
		else
			bot_iter->SetTaunting(taunt_state);

		if (sbl.size() == 1)
			Bot::BotGroupSay(bot_iter, "I am %s taunting", bot_iter->IsTaunting() ? "now" : "no longer");

		++taunting_count;
	}

	if (taunting_count) {
		if (toggle_taunt)
			c->Message(m_action, "%i of your bots %s toggled their taunting state", taunting_count, ((taunting_count != 1) ? ("have") : ("has")));
		else
			c->Message(m_action, "%i of your bots %s %s taunting", taunting_count, ((taunting_count != 1) ? ("have") : ("has")), ((taunt_state) ? ("started") : ("stopped")));
	}
	else {
		c->Message(m_fail, "None of your bots are capable of taunting");
	}
}

void bot_command_track(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_track", sep->arg[0], "track"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s (Ranger: [option=all: all | rare | local])", sep->arg[0]);
		c->Message(m_note, "requires one of the following bot classes:");
		c->Message(m_message, "Ranger(1), Druid(20) or Bard(35)");
		return;
	}

	std::string tracking_scope = sep->arg[1];

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);
	
	uint16 class_mask = (PLAYER_CLASS_RANGER_BIT | PLAYER_CLASS_DRUID_BIT | PLAYER_CLASS_BARD_BIT);
	ActionableBots::Filter_ByClasses(c, sbl, class_mask);
	
	Bot* my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 1, RANGER);
	if (tracking_scope.empty()) {
		if (!my_bot)
			my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 20, DRUID);
		if (!my_bot)
			my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 35, BARD);
	}
	if (!my_bot) {
		c->Message(m_fail, "No bots are capable of performing this action");
		return;
	}

	int base_distance = 0;
	bool track_named = false;
	std::string tracking_msg;
	switch (my_bot->GetClass()) {
	case RANGER:
		if (!tracking_scope.compare("local")) {
			base_distance = 30;
			tracking_msg = "Local tracking...";
		}
		else if (!tracking_scope.compare("rare")) {
			base_distance = 80;
			bool track_named = true;
			tracking_msg = "Master tracking...";
		}
		else { // default to 'all'
			base_distance = 80;
			tracking_msg = "Advanced tracking...";
		}
		break;
	case DRUID:
		base_distance = 30;
		tracking_msg = "Local tracking...";
		break;
	case BARD:
		base_distance = 20;
		tracking_msg = "Near tracking...";
		break;
	default:
		return;
	}
	if (!base_distance) {
		c->Message(m_unknown, "An unknown codition has occurred");
		return;
	}

	my_bot->InterruptSpell();
	Bot::BotGroupSay(my_bot, tracking_msg.c_str());
	entity_list.ShowSpawnWindow(c, (c->GetLevel() * base_distance), track_named);
}

void bot_command_water_breathing(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_WaterBreathing];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_WaterBreathing) || helper_command_alias_fail(c, "bot_command_water_breathing", sep->arg[0], "waterbreathing"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<friendly_target>) %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_WaterBreathing);
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}


/*
 * bot subcommands go below here
 */
void bot_subcommand_bot_appearance(Client *c, const Seperator *sep)
{
	/* VS2012 code - begin */
	std::list<const char*> subcommand_list;
	subcommand_list.push_back("botbeardcolor");
	subcommand_list.push_back("botbeardstyle");
	subcommand_list.push_back("botdetails");
	subcommand_list.push_back("boteyes");
	subcommand_list.push_back("botface");
	subcommand_list.push_back("bothaircolor");
	subcommand_list.push_back("bothairstyle");
	subcommand_list.push_back("botheritage");
	subcommand_list.push_back("bottattoo");
	subcommand_list.push_back("botwoad");
	/* VS2012 code - end */
	
	/* VS2013 code
	const std::list<const char*> subcommand_list = {
		"botbeardcolor", "botbeardstyle", "botdetails", "boteyes", "botface",
		"bothaircolor", "bothairstyle", "botheritage", "bottattoo", "botwoad"
	};
	*/
	
	if (helper_command_alias_fail(c, "bot_subcommand_bot_appearance", sep->arg[0], "botappearance"))
		return;

	helper_send_available_subcommands(c, "bot appearance", subcommand_list);
}

void bot_subcommand_bot_beard_color(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_beard_color", sep->arg[0], "botbeardcolor"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n] (Dwarves or male bots only)", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetGender() != MALE && my_bot->GetRace() != DWARF)
		fail_type = BCEnum::AFT_GenderRace;
	else if (!PlayerAppearance::IsValidBeardColor(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetBeardColor(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "beard color"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_beard_style(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_beard_style", sep->arg[0], "botbeardstyle"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n] (Dwarves or male bots only)", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetGender() != MALE && my_bot->GetRace() != DWARF)
		fail_type = BCEnum::AFT_GenderRace;
	else if (!PlayerAppearance::IsValidBeard(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetBeard(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "beard style"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_camp(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_camp", sep->arg[0], "botcamp"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl)
		bot_iter->Camp();
}

void bot_subcommand_bot_clone(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_clone", sep->arg[0], "botclone"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [clone_name]", sep->arg[0]);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}
	if (!my_bot->GetBotID()) {
		c->Message(m_unknown, "An unknown error has occured - BotName: %s, BotID: %u", my_bot->GetCleanName(), my_bot->GetBotID());
		Log.Out(Logs::General, Logs::Commands, "bot_command_clone(): - Error: Active bot reported invalid ID (BotName: %s, BotID: %u, OwnerName: %s, OwnerID: %u, AcctName: %s, AcctID: %u)",
			my_bot->GetCleanName(), my_bot->GetBotID(), c->GetCleanName(), c->CharacterID(), c->AccountName(), c->AccountID());
		return;
	}

	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(m_fail, "You must [name] your bot clone");
		return;
	}
	std::string bot_name = sep->arg[1];

	if (!Bot::IsValidName(bot_name)) {
		c->Message(m_fail, "'%s' is an invalid name. You may only use characters 'A-Z', 'a-z' and '_'", bot_name.c_str());
		return;
	}

	std::string error_message;

	bool available_flag = false;
	if (!botdb.QueryNameAvailablity(bot_name, available_flag)) {
		c->Message(m_fail, "%s", BotDatabase::fail::QueryNameAvailablity());
		return;
	}
	if (!available_flag) {
		c->Message(m_fail, "The name %s is already being used. Please choose a different name", bot_name.c_str());
		return;
	}

	uint32 max_bot_count = RuleI(Bots, CreationLimit);

	uint32 bot_count = 0;
	if (!botdb.QueryBotCount(c->CharacterID(), bot_count)) {
		c->Message(m_fail, "%s", BotDatabase::fail::QueryBotCount());
		return;
	}
	if (bot_count >= max_bot_count) {
		c->Message(m_fail, "You have reached the maximum limit of %i bots", max_bot_count);
		return;
	}

	uint32 clone_id = 0;
	if (!botdb.CreateCloneBot(c->CharacterID(), my_bot->GetBotID(), bot_name, clone_id) || !clone_id) {
		c->Message(m_fail, "%s '%s'", BotDatabase::fail::CreateCloneBot(), bot_name.c_str());
		return;
	}

	int clone_stance = BotStancePassive;
	if (!botdb.LoadStance(my_bot->GetBotID(), clone_stance))
		c->Message(m_fail, "%s for bot '%s'", BotDatabase::fail::LoadStance(), my_bot->GetCleanName());
	if (!botdb.SaveStance(clone_id, clone_stance))
		c->Message(m_fail, "%s for clone '%s'", BotDatabase::fail::SaveStance(), bot_name.c_str());

	if (!botdb.CreateCloneBotInventory(c->CharacterID(), my_bot->GetBotID(), clone_id))
		c->Message(m_fail, "%s for clone '%s'", BotDatabase::fail::CreateCloneBotInventory(), bot_name.c_str());
	
	c->Message(m_action, "Bot '%s' was successfully cloned to bot '%s'", my_bot->GetCleanName(), bot_name.c_str());
}

void bot_subcommand_bot_create(Client *c, const Seperator *sep)
{
	const std::string msg_class = StringFormat("class: %u(WAR), %u(CLR), %u(PAL), %u(RNG), %u(SHD), %u(DRU), %u(MNK), %u(BRD), %u(ROG), %u(SHM), %u(NEC), %u(WIZ), %u(MAG), %u(ENC), %u(BST), %u(BER)",
		WARRIOR, CLERIC, PALADIN, RANGER, SHADOWKNIGHT, DRUID, MONK, BARD, ROGUE, SHAMAN, NECROMANCER, WIZARD, MAGICIAN, ENCHANTER, BEASTLORD, BERSERKER);
	const std::string msg_race = StringFormat("race: %u(HUM), %u(BAR), %u(ERU), %u(ELF), %u(HIE), %u(DEF), %u(HEF), %u(DWF), %u(TRL), %u(OGR), %u(HFL), %u(GNM), %u(IKS), %u(VAH), %u(FRG), %u(DRK)",
		HUMAN, BARBARIAN, ERUDITE, WOOD_ELF, HIGH_ELF, DARK_ELF, HALF_ELF, DWARF, TROLL, OGRE, HALFLING, GNOME, IKSAR, VAHSHIR, FROGLOK, DRAKKIN);
	const std::string msg_gender = StringFormat("gender: %u(M), %u(F)", MALE, FEMALE);
	
	if (helper_command_alias_fail(c, "bot_subcommand_bot_create", sep->arg[0], "botcreate"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [bot_name] [bot_class] [bot_race] [bot_gender]", sep->arg[0]);
		c->Message(m_note, msg_class.c_str());
		c->Message(m_note, msg_race.c_str());
		c->Message(m_note, msg_gender.c_str());
		return;
	}

	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(m_fail, "You must [name] your bot");
		return;
	}
	std::string bot_name = sep->arg[1];

	if (sep->arg[2][0] == '\0' || !sep->IsNumber(2)) {
		c->Message(m_fail, msg_class.c_str());
		return;
	}
	uint8 bot_class = atoi(sep->arg[2]);

	if (sep->arg[3][0] == '\0' || !sep->IsNumber(3)) {
		c->Message(m_fail, msg_race.c_str());
		return;
	}
	uint16 bot_race = atoi(sep->arg[3]);

	if (sep->arg[4][0] == '\0') {
		c->Message(m_fail, msg_gender.c_str());
		return;
	}
	uint8 bot_gender = atoi(sep->arg[4]);

	helper_bot_create(c, bot_name, bot_class, bot_race, bot_gender);
}

void bot_subcommand_bot_delete(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_delete", sep->arg[0], "botdelete"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s", sep->arg[0]);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	std::string error_message;

	if (!my_bot->DeleteBot()) {
		c->Message(m_unknown, "Failed to delete '%s' due to database error", my_bot->GetCleanName());
		return;
	}

	auto bid = my_bot->GetBotID();
	std::string bot_name = my_bot->GetCleanName();

	my_bot->Camp(false);

	c->Message(m_action, "Successfully deleted bot '%s' (id: %i)", bot_name.c_str(), bid);
}

void bot_subcommand_bot_details(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue
	
	if (helper_command_alias_fail(c, "bot_subcommand_bot_details", sep->arg[0], "botdetails"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetRace() != DRAKKIN)
		fail_type = BCEnum::AFT_Race;
	else if (!PlayerAppearance::IsValidDetail(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetDrakkinDetails(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "details"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_dye_armor(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue
	
	const std::string msg_matslot = StringFormat("mat_slot: %c(All), %i(Head), %i(Chest), %i(Arms), %i(Wrists), %i(Hands), %i(Legs), %i(Feet)",
		'*', EQEmu::textures::TextureHead, EQEmu::textures::TextureChest, EQEmu::textures::TextureArms, EQEmu::textures::TextureWrist, EQEmu::textures::TextureHands, EQEmu::textures::TextureLegs, EQEmu::textures::TextureFeet);
	
	if (helper_command_alias_fail(c, "bot_subcommand_bot_dye_armor", sep->arg[0], "botdyearmor"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [mat_slot] [red: 0-255] [green: 0-255] [blue: 0-255] ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(m_note, msg_matslot.c_str());
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	uint8 material_slot = EQEmu::textures::TextureInvalid;
	int16 slot_id = INVALID_INDEX;

	bool dye_all = (sep->arg[1][0] == '*');
	if (!dye_all) {
		material_slot = atoi(sep->arg[1]);
		slot_id = Inventory::CalcSlotFromMaterial(material_slot);

		if (!sep->IsNumber(1) || slot_id == INVALID_INDEX || material_slot > EQEmu::textures::TextureFeet) {
			c->Message(m_fail, "Valid [mat_slot]s for this command are:");
			c->Message(m_fail, msg_matslot.c_str());
			return;
		}
	}

	uint32 red_value = atoi(sep->arg[2]);
	if (!sep->IsNumber(2) || red_value > 255) {
		c->Message(m_fail, "Valid [red] values for this command are [0-255]");
		return;
	}

	uint32 green_value = atoi(sep->arg[3]);
	if (!sep->IsNumber(3) || green_value > 255) {
		c->Message(m_fail, "Valid [green] values for this command are [0-255]");
		return;
	}

	uint32 blue_value = atoi(sep->arg[4]);
	if (!sep->IsNumber(4) || blue_value > 255) {
		c->Message(m_fail, "Valid [blue] values for this command are [0-255]");
		return;
	}

	uint32 rgb_value = ((0xFF) | (red_value << 16) | (green_value << 8) | (blue_value)); // watch the leading '(0xFF) | '

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[5], sbl, ab_mask);
	if (ab_type == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (!bot_iter->DyeArmor(slot_id, rgb_value, dye_all, (ab_type != ActionableBots::ABT_All))) {
			c->Message(m_fail, "Failed to change armor color for '%s' due to unknown cause", bot_iter->GetCleanName());
			return;
		}

		//if (dye_all)
		//	helper_bot_appearance_form_update(bot_iter);
	}

	if (ab_type == ActionableBots::ABT_All) {
		if (dye_all) {
			if (!botdb.SaveAllArmorColors(c->CharacterID(), rgb_value))
				c->Message(m_fail, "%s", BotDatabase::fail::SaveAllArmorColors());
		}
		else {
			if (!botdb.SaveAllArmorColorBySlot(c->CharacterID(), slot_id, rgb_value))
				c->Message(m_fail, "%s", BotDatabase::fail::SaveAllArmorColorBySlot());
		}
	}
}

void bot_subcommand_bot_eyes(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue
	
	// not sure if left/right bias is allowed in pc-type entities (something is keeping them from being different)
	if (helper_command_alias_fail(c, "bot_subcommand_bot_eyes", sep->arg[0], "boteyes"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		//c->Message(m_usage, "usage: <target_bot> %s [value:0-n] ([option: left | right])", sep->arg[0]);
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = atoi(sep->arg[1]);

	//uint8 eye_bias = 0;
	//std::string arg2 = sep->arg[2];
	//if (!arg2.compare("left"))
	//	eye_bias = 1;
	//else if (!arg2.compare("right"))
	//	eye_bias = 2;

	auto fail_type = BCEnum::AFT_None;
	if (!PlayerAppearance::IsValidEyeColor(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = BCEnum::AFT_Value;
	}
	else {
		//if (eye_bias == 1) {
		//	my_bot->SetEyeColor1(uvalue);
		//}
		//else if (eye_bias == 2) {
		//	my_bot->SetEyeColor2(uvalue);
		//}
		//else {
			my_bot->SetEyeColor1(uvalue);
			my_bot->SetEyeColor2(uvalue);
		//}
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "eyes"))
		return;

	c->Message(m_action, "This feature will update the next time your bot is spawned");
	//helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_face(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_face", sep->arg[0], "botface"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (!PlayerAppearance::IsValidFace(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = BCEnum::AFT_Value;
	}
	else {
		uint8 old_woad = 0;
		if (my_bot->GetRace() == BARBARIAN)
			old_woad = ((my_bot->GetLuclinFace() / 10) * 10);
		my_bot->SetLuclinFace((old_woad + uvalue));
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "face"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_follow_distance(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_follow_distance", sep->arg[0], "botfollowdistance"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [set] [distance] ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(m_usage, "usage: %s [clear] ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	uint32 bfd = BOT_DEFAULT_FOLLOW_DISTANCE;
	bool set_flag = false;
	int ab_arg = 2;

	if (!strcasecmp(sep->arg[1], "set")) {
		if (!sep->IsNumber(2)) {
			c->Message(m_fail, "A numeric [distance] is required to use this command");
			return;
		}

		bfd = atoi(sep->arg[2]);
		set_flag = true;
		ab_arg = 3;
	}
	else if (strcasecmp(sep->arg[1], "clear")) {
		c->Message(m_fail, "This command requires a [set | clear] argument");
		return;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		bot_iter->SetFollowDistance(bfd);
		if (ab_type != ActionableBots::ABT_All && !botdb.SaveFollowDistance(c->CharacterID(), bot_iter->GetBotID(), bfd)) {
			c->Message(m_fail, "%s for '%s'", BotDatabase::fail::SaveFollowDistance(), bot_iter->GetCleanName());
			return;
		}

		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		if (!botdb.SaveAllFollowDistances(c->CharacterID(), bfd)) {
			c->Message(m_fail, "%s", BotDatabase::fail::SaveAllFollowDistances());
			return;
		}

		c->Message(m_action, "%s all of your bot follow distances", set_flag ? "Set" : "Cleared");
	}
	else {
		c->Message(m_action, "%s %i of your spawned bot follow distances", (set_flag ? "Set" : "Cleared"), bot_count);
	}
}

void bot_subcommand_bot_hair_color(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_hair_color", sep->arg[0], "bothaircolor"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (!PlayerAppearance::IsValidHairColor(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetHairColor(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "hair color"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_hairstyle(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_hairstyle", sep->arg[0], "bothairstyle"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n]", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (!PlayerAppearance::IsValidHair(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetHairStyle(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "hair style"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_heritage(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue
	
	if (helper_command_alias_fail(c, "bot_subcommand_bot_heritage", sep->arg[0], "botheritage"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetRace() != DRAKKIN)
		fail_type = BCEnum::AFT_Race;
	else if (!PlayerAppearance::IsValidHeritage(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetDrakkinHeritage(uvalue);
	
	if (helper_bot_appearance_fail(c, my_bot, fail_type, "heritage"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_inspect_message(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_inspect_message", sep->arg[0], "botinspectmessage"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [set | clear] ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(m_note, "Notes:");
		if (c->ClientVersion() >= EQEmu::versions::ClientVersion::SoF) {
			c->Message(m_message, "- Self-inspect and type your bot's inspect message");
			c->Message(m_message, "- Close the self-inspect window to update the server");
			c->Message(m_message, "- Type '%s set' to set the bot's inspect message", sep->arg[0]);
		}
		else {
			c->Message(m_message, "- Self-inspect and type your bot's inspect message");
			c->Message(m_message, "- Close the self-inspect window");
			c->Message(m_message, "- Self-inspect again to update the server");
			c->Message(m_message, "- Type '%s set' to set the bot's inspect message", sep->arg[0]);
		}
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	bool set_flag = false;
	if (!strcasecmp(sep->arg[1], "set")) {
		set_flag = true;
	}
	else if (strcasecmp(sep->arg[1], "clear")) {
		c->Message(15, "This command requires a [set | clear] argument");
		return;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	const auto client_message_struct = &c->GetInspectMessage();

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		auto bot_message_struct = &bot_iter->GetInspectMessage();
		memset(bot_message_struct, 0, sizeof(InspectMessage_Struct));
		if (set_flag)
			memcpy(bot_message_struct, client_message_struct, sizeof(InspectMessage_Struct));

		if (ab_type != ActionableBots::ABT_All && !botdb.SaveInspectMessage(bot_iter->GetBotID(), *bot_message_struct)) {
			c->Message(m_fail, "%s for '%s'", BotDatabase::fail::SaveInspectMessage(), bot_iter->GetCleanName());
			return;
		}

		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		InspectMessage_Struct bot_message_struct;
		memset(&bot_message_struct, 0, sizeof(InspectMessage_Struct));
		if (set_flag)
			memcpy(&bot_message_struct, client_message_struct, sizeof(InspectMessage_Struct));

		if (!botdb.SaveAllInspectMessages(c->CharacterID(), bot_message_struct)) {
			c->Message(m_fail, "%s", BotDatabase::fail::SaveAllInspectMessages());
			return;
		}

		c->Message(m_action, "%s all of your bot inspect messages", set_flag ? "Set" : "Cleared");
	}
	else {
		c->Message(m_action, "%s %i of your spawned bot inspect messages", set_flag ? "Set" : "Cleared", bot_count);
	}
}

void bot_subcommand_bot_list(Client *c, const Seperator *sep)
{
	// TODO: Consider re-working to use player race values instead of actual race

	enum { FilterClass, FilterRace, FilterName, FilterCount, MaskClass = (1 << FilterClass), MaskRace = (1 << FilterRace), MaskName = (1 << FilterName) };

	if (helper_command_alias_fail(c, "bot_subcommand_bot_list", sep->arg[0], "botlist"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([class] [value]) ([race] [value]) ([name] [partial-full])", sep->arg[0]);
		c->Message(m_note, "Note: filter criteria is orderless and optional");
		return;
	}

	uint32 filter_value[FilterCount];
	int name_criteria_arg = 0;
	memset(&filter_value, 0, sizeof(uint32) * FilterCount);

	int filter_mask = 0;
	for (int i = 1; i < (FilterCount * 2); i += 2) {
		if (sep->arg[i][0] == '\0')
			break;

		if (!strcasecmp(sep->arg[i], "class")) {
			filter_mask |= MaskClass;
			filter_value[FilterClass] = atoi(sep->arg[i + 1]);
			continue;
		}
		if (!strcasecmp(sep->arg[i], "race")) {
			filter_mask |= MaskRace;
			filter_value[FilterRace] = atoi(sep->arg[i + 1]);
			continue;
		}
		if (!strcasecmp(sep->arg[i], "name")) {
			filter_mask |= MaskName;
			name_criteria_arg = (i + 1);
			continue;
		}

		c->Message(m_fail, "A numeric value or name is required to use the filter property of this command (f: '%s', v: '%s')", sep->arg[i], sep->arg[i + 1]);
		return;
	}

	std::list<BotsAvailableList> bots_list;
	if (!botdb.LoadBotsList(c->CharacterID(), bots_list)) {
		c->Message(m_fail, "%s", BotDatabase::fail::LoadBotsList());
		return;
	}
	if (bots_list.empty()) {
		c->Message(m_fail, "You have no bots");
		return;
	}

	int bot_count = 0;
	for (auto bots_iter : bots_list) {
		if (filter_mask) {
			if ((filter_mask & MaskClass) && filter_value[FilterClass] != bots_iter.Class)
				continue;
			if ((filter_mask & MaskRace) && filter_value[FilterRace] != bots_iter.Race)
				continue;
			if (filter_mask & MaskName) {
				std::string name_criteria = sep->arg[name_criteria_arg];
				std::transform(name_criteria.begin(), name_criteria.end(), name_criteria.begin(), ::tolower);
				std::string name_check = bots_iter.Name;
				std::transform(name_check.begin(), name_check.end(), name_check.begin(), ::tolower);
				if (name_check.find(name_criteria) == std::string::npos)
					continue;
			}
		}

		c->Message(m_message, "%s is a level %u %s %s %s",
			bots_iter.Name,
			bots_iter.Level,
			Bot::RaceIdToString(bots_iter.Race).c_str(),
			((bots_iter.Gender == FEMALE) ? ("Female") : ((bots_iter.Gender == MALE) ? ("Male") : ("Neuter"))),
			Bot::ClassIdToString(bots_iter.Class).c_str()
		);

		++bot_count;
	}
	if (!bot_count) {
		c->Message(m_fail, "You have no bots meeting this criteria");
	}
	else {
		c->Message(m_action, "%i of %i bot%s shown", bot_count, bots_list.size(), ((bot_count != 1) ? ("s") : ("")));
		c->Message(m_message, "Your limit is %i bot%s", RuleI(Bots, CreationLimit), ((RuleI(Bots, CreationLimit) != 1) ? ("s") : ("")));
	}
}

void bot_subcommand_bot_out_of_combat(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_out_of_combat", sep->arg[0], "botoutofcombat"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([option: on | off]) ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::string arg1 = sep->arg[1];

	bool behavior_state = false;
	bool toggle_behavior = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		behavior_state = true;
		toggle_behavior = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_behavior = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (toggle_behavior)
			bot_iter->SetAltOutOfCombatBehavior(!bot_iter->GetAltOutOfCombatBehavior());
		else
			bot_iter->SetAltOutOfCombatBehavior(behavior_state);
		
		helper_bot_out_of_combat(c, bot_iter);
	}
}

void bot_subcommand_bot_report(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_report", sep->arg[0], "botreport"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::string ab_type_arg = sep->arg[1];
	if (ab_type_arg.empty()) {
		if (c->GetTarget()) {
			if (c->GetTarget()->IsClient() && c->GetTarget()->CastToClient() == c)
				ab_type_arg = "ownergroup";
			else if (c->GetTarget()->IsClient() && c->GetTarget()->CastToClient() != c)
				ab_type_arg = "targetgroup";
		}
		else {
			ab_type_arg = "spawned";
		}
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, ab_type_arg.c_str(), sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		std::string report_msg = StringFormat("%s %s reports", Bot::ClassIdToString(bot_iter->GetClass()).c_str(), bot_iter->GetCleanName());
		report_msg.append(StringFormat(": %3.1f%% health", bot_iter->GetHPRatio()));
		if (!IsNonSpellFighterClass(bot_iter->GetClass()))
			report_msg.append(StringFormat(": %3.1f%% mana", bot_iter->GetManaRatio()));

		c->Message(m_message, "%s", report_msg.c_str());
	}
}

void bot_subcommand_bot_spawn(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_spawn", sep->arg[0], "botspawn"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [bot_name]", sep->arg[0]);
		return;
	}

	int rule_level = RuleI(Bots, BotCharacterLevel);
	if (c->GetLevel() < rule_level) {
		c->Message(m_fail, "You must be level %i to use bots", rule_level);
		return;
	}

	if (c->GetFeigned()) {
		c->Message(m_fail, "You can not spawn a bot while feigned");
		return;
	}

	int spawned_bot_count = Bot::SpawnedBotCount(c->CharacterID());

	int rule_limit = RuleI(Bots, SpawnLimit);
	if (spawned_bot_count >= rule_limit && !c->GetGM()) {
		c->Message(m_fail, "You can not have more than %i spawned bots", rule_limit);
		return;
	}

	if (RuleB(Bots, QuestableSpawnLimit) && !c->GetGM()) {
		int allowed_bot_count = 0;
		if (!botdb.LoadQuestableSpawnCount(c->CharacterID(), allowed_bot_count)) {
			c->Message(m_fail, "%s", BotDatabase::fail::LoadQuestableSpawnCount());
			return;
		}
		if (!allowed_bot_count) {
			c->Message(m_fail, "You are not currently allowed any spawned bots");
			return;
		}
		if (spawned_bot_count >= allowed_bot_count) {
			c->Message(m_fail, "You have reached your current limit of %i spawned bots", allowed_bot_count);
			return;
		}
	}

	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(m_fail, "You must specify a [name] to use this command");
		return;
	}
	std::string bot_name = sep->arg[1];

	uint32 bot_id = 0;
	if (!botdb.LoadBotID(c->CharacterID(), bot_name, bot_id)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadBotID(), bot_name.c_str());
		return;
	}
	if (!bot_id) {
		c->Message(m_fail, "You don't own a bot named '%s'", bot_name.c_str());
		return;
	}

	if (entity_list.GetMobByBotID(bot_id)) {
		c->Message(m_fail, "'%s' is already spawned in zone", bot_name.c_str());
		return;
	}

	// this probably needs work...
	if (c->GetGroup()) {
		std::list<Mob*> group_list;
		c->GetGroup()->GetMemberList(group_list);
		for (auto member_iter : group_list) {
			if (!member_iter)
				continue;
			if (member_iter->qglobal) // what is this?? really should have had a message to describe failure... (can't spawn bots if you are assigned to a task/instance?)
				return;
			if (!member_iter->qglobal && (member_iter->GetAppearance() != eaDead) && (member_iter->IsEngaged() || (member_iter->IsClient() && member_iter->CastToClient()->GetAggroCount()))) {
				c->Message(m_fail, "You can't summon bots while you are engaged.");
				return;
			}
		}
	}
	else if (c->GetAggroCount() > 0) {
		c->Message(m_fail, "You can't spawn bots while you are engaged.");
		return;
	}
	
	//if (c->IsEngaged()) {
	//	c->Message(m_fail, "You can't spawn bots while you are engaged.");
	//	return;
	//}

	auto my_bot = Bot::LoadBot(bot_id);
	if (!my_bot) {
		c->Message(m_fail, "No valid bot '%s' (id: %i) exists", bot_name.c_str(), bot_id);
		return;
	}

	my_bot->Spawn(c);

	static const char* bot_spawn_message[16] = {
		"A solid weapon is my ally!", // WARRIOR / 'generic'
		"The pious shall never die!", // CLERIC
		"I am the symbol of Light!", // PALADIN
		"There are enemies near!", // RANGER
		"Out of the shadows, I step!", // SHADOWKNIGHT
		"Nature's fury shall be wrought!", // DRUID
		"Your punishment will be my fist!", // MONK
		"Music is the overture of battle! ", // BARD
		"Daggers into the backs of my enemies!", // ROGUE
		"More bones to grind!", // SHAMAN
		"Death is only the beginning!", // NECROMANCER
		"I am the harbinger of demise!", // WIZARD
		"The elements are at my command!", // MAGICIAN
		"No being can resist my charm!", // ENCHANTER
		"Battles are won by hand and paw!", // BEASTLORD
		"My bloodthirst shall not be quenched!" // BERSERKER
	};

	Bot::BotGroupSay(my_bot, "%s", bot_spawn_message[CLASSIDTOINDEX(my_bot->GetClass())]);
}

void bot_subcommand_bot_stance(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_stance", sep->arg[0], "botstance"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [current | value: 0-6] ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		c->Message(m_note, "value: %u(%s), %u(%s), %u(%s), %u(%s), %u(%s), %u(%s), %u(%s)",
			BotStancePassive, GetBotStanceName(BotStancePassive),
			BotStanceBalanced, GetBotStanceName(BotStanceBalanced),
			BotStanceEfficient, GetBotStanceName(BotStanceEfficient),
			BotStanceReactive, GetBotStanceName(BotStanceReactive),
			BotStanceAggressive, GetBotStanceName(BotStanceAggressive),
			BotStanceBurn, GetBotStanceName(BotStanceBurn),
			BotStanceBurnAE, GetBotStanceName(BotStanceBurnAE)
		);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	bool current_flag = false;
	auto bst = BotStanceUnknown;
	
	if (!strcasecmp(sep->arg[1], "current"))
		current_flag = true;
	else if (sep->IsNumber(1))
		bst = VALIDBOTSTANCE(atoi(sep->arg[1]));

	if (!current_flag && bst == BotStanceUnknown) {
		c->Message(m_fail, "A [current] argument or valid numeric [value] is required to use this command");
		return;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (!current_flag) {
			bot_iter->SetBotStance(bst);
			bot_iter->CalcChanceToCast();
			bot_iter->Save();
		}

		Bot::BotGroupSay(bot_iter, "My current stance is '%s' (%u)", GetBotStanceName(bot_iter->GetBotStance()), bot_iter->GetBotStance());
	}
}

void bot_subcommand_bot_summon(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_summon", sep->arg[0], "botsummon"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		Bot::BotGroupSay(bot_iter, "Whee!");

		bot_iter->WipeHateList();
		bot_iter->SetTarget(bot_iter->GetBotOwner());
		bot_iter->Warp(glm::vec3(c->GetPosition()));

		if (!bot_iter->HasPet())
			continue;

		bot_iter->GetPet()->WipeHateList();
		bot_iter->GetPet()->SetTarget(bot_iter);
		bot_iter->GetPet()->Warp(glm::vec3(c->GetPosition()));
	}

	if (sbl.size() == 1)
		c->Message(m_action, "Summoned %s to you", ((sbl.front()) ? (sbl.front()->GetCleanName()) : ("'nullptr'")));
	else
		c->Message(m_action, "Summoned %i bots to you", sbl.size());
}

void bot_subcommand_bot_tattoo(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue
	
	if (helper_command_alias_fail(c, "bot_subcommand_bot_tattoo", sep->arg[0], "bottattoo"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n] (Drakkin bots only)", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint32 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetRace() != DRAKKIN)
		fail_type = BCEnum::AFT_Race;
	else if (!PlayerAppearance::IsValidTattoo(my_bot->GetRace(), my_bot->GetGender(), uvalue))
		fail_type = BCEnum::AFT_Value;
	else
		my_bot->SetDrakkinTattoo(uvalue);

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "tattoo"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_bot_toggle_archer(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_toggle_archer", sep->arg[0], "bottogglearcher"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([option: on | off]) ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::string arg1 = sep->arg[1];

	bool archer_state = false;
	bool toggle_archer = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		archer_state = true;
		toggle_archer = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_archer = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;
		
		if (toggle_archer)
			bot_iter->SetBotArcher(!bot_iter->IsBotArcher());
		else
			bot_iter->SetBotArcher(archer_state);
		bot_iter->ChangeBotArcherWeapons(bot_iter->IsBotArcher());

		if (bot_iter->GetClass() == RANGER && bot_iter->GetLevel() >= 61)
			bot_iter->SetRangerAutoWeaponSelect(bot_iter->IsBotArcher());
	}
}

void bot_subcommand_bot_toggle_helm(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_toggle_helm", sep->arg[0], "bottogglehelm"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([option: on | off]) ([actionable: target | byname | ownergroup | botgroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::string arg1 = sep->arg[1];

	bool helm_state = false;
	bool toggle_helm = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		helm_state = true;
		toggle_helm = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_helm = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (toggle_helm)
			bot_iter->SetShowHelm(!bot_iter->GetShowHelm());
		else
			bot_iter->SetShowHelm(helm_state);

		if (ab_type != ActionableBots::ABT_All) {
			if (!botdb.SaveHelmAppearance(c->CharacterID(), bot_iter->GetBotID(), bot_iter->GetShowHelm())) {
				c->Message(m_unknown, "%s for '%s'", bot_iter->GetCleanName());
				return;
			}

			EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
			SpawnAppearance_Struct* saptr = (SpawnAppearance_Struct*)outapp->pBuffer;
			saptr->spawn_id = bot_iter->GetID();
			saptr->type = AT_ShowHelm;
			saptr->parameter = bot_iter->GetShowHelm();

			entity_list.QueueClients(bot_iter, outapp);
			safe_delete(outapp);

			//helper_bot_appearance_form_update(bot_iter);
		}
		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		std::string query;
		if (toggle_helm) {
			if (!botdb.ToggleAllHelmAppearances(c->CharacterID()))
				c->Message(m_fail, "%s", BotDatabase::fail::ToggleAllHelmAppearances());
		}
		else {
			if (!botdb.SaveAllHelmAppearances(c->CharacterID(), helm_state))
				c->Message(m_fail, "%s", BotDatabase::fail::SaveAllHelmAppearances());
		}

		c->Message(m_action, "%s all of your bot show helm flags", toggle_helm ? "Toggled" : (helm_state ? "Set" : "Cleared"));
	}
	else {
		c->Message(m_action, "%s %i of your spawned bot show helm flags", toggle_helm ? "Toggled" : (helm_state ? "Set" : "Cleared"), bot_count);
	}

	// Notes:
	/*
	[CLIENT OPCODE TEST]
	[10-16-2015 :: 14:57:56] [Packet :: Client -> Server (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 01 00 00 00 - showhelm = true (client)
	[10-16-2015 :: 14:57:56] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 01 00 00 00 - showhelm = true (client)

	[10-16-2015 :: 14:58:02] [Packet :: Client -> Server (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 00 00 00 00 - showhelm = false (client)
	[10-16-2015 :: 14:58:02] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 00 00 00 00 - showhelm = false (client)

	[BOT OPCODE TEST]
	[10-16-2015 :: 22:15:34] [Packet :: Client -> Server (Dump)] [OP_ChannelMessage - 0x0045] [Size: 167]
	0: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	64: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	128: 00 00 00 00 08 00 00 00 - CD CD CD CD CD CD CD CD  | ................
	144: 64 00 00 00 23 62 6F 74 - 20 73 68 6F 77 68 65 6C  | d...#bot showhel
	160: 6D 20 6F 6E 00                                     | m on.

	[10-16-2015 :: 22:15:34] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A2 02 2B 00 01 00 00 00 - showhelm = true

	[10-16-2015 :: 22:15:40] [Packet :: Client -> Server (Dump)] [OP_ChannelMessage - 0x0045] [Size: 168]
	0: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	64: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	128: 00 00 00 00 08 00 00 00 - CD CD CD CD CD CD CD CD  | ................
	144: 64 00 00 00 23 62 6F 74 - 20 73 68 6F 77 68 65 6C  | d...#bot showhel
	160: 6D 20 6F 66 66 00                                  | m off.

	[10-16-2015 :: 22:15:40] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A2 02 2B 00 00 00 00 00 - showhelm = false

	*** Bot did not update using the OP_SpawnAppearance packet with AT_ShowHelm appearance type ***
	*/
}

void bot_subcommand_bot_update(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_update", sep->arg[0], "botupdate"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You currently have no spawned bots");
		return;
	}

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter || bot_iter->IsEngaged() || bot_iter->GetLevel() == c->GetLevel())
			continue;

		bot_iter->SetPetChooser(false);
		bot_iter->CalcBotStats((sbl.size() == 1));
		++bot_count;
	}

	c->Message(m_action, "Updated %i of your %i spawned bots", bot_count, sbl.size());
}

void bot_subcommand_bot_woad(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_bot_woad", sep->arg[0], "botwoad"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: <target_bot> %s [value: 0-n] (Barbarian bots only)", sep->arg[0]);
		c->Message(m_note, "note: Actual limit is filter-based");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(m_fail, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(m_fail, "A numeric [value] is required to use this command");
		return;
	}

	uint8 uvalue = atoi(sep->arg[1]);

	auto fail_type = BCEnum::AFT_None;
	if (my_bot->GetRace() != BARBARIAN) {
		fail_type = BCEnum::AFT_Race;
	}
	else if (!PlayerAppearance::IsValidWoad(my_bot->GetRace(), my_bot->GetGender(), uvalue)) {
		fail_type = BCEnum::AFT_Value;
	}
	else {
		uint8 old_face = (my_bot->GetLuclinFace() % 10);
		my_bot->SetLuclinFace(((uvalue * 10) + old_face));
	}

	if (helper_bot_appearance_fail(c, my_bot, fail_type, "woad"))
		return;

	helper_bot_appearance_form_final(c, my_bot);
}

void bot_subcommand_botgroup_add_member(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_botgroup_add_member", sep->arg[0], "botgroupaddmember"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_leader>) %s [member_name] ([leader_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		c->Message(m_fail, "You must [name] a new member as a bot that you own to use this command");
		return;
	}

	auto new_member = sbl.front();
	if (!new_member) {
		c->Message(m_unknown, "Error: New member bot dereferenced to nullptr");
		return;
	}

	if (new_member->HasGroup()) {
		c->Message(m_fail, "%s is already a current member of a group and can not join another one", new_member->GetCleanName());
		return;
	}

	uint32 botgroup_id = 0;
	if (!botdb.LoadBotGroupIDByMemberID(new_member->GetBotID(), botgroup_id)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadBotGroupIDByMemberID(), new_member->GetCleanName());
		return;
	}
	if (botgroup_id) {
		c->Message(m_fail, "%s is already a current member of a bot-group", new_member->GetCleanName());
		return;
	}

	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a group leader as a bot that you own to use this command");
		return;
	}

	auto botgroup_leader = sbl.front();
	if (!botgroup_leader) {
		c->Message(m_unknown, "Error: Group leader bot dereferenced to nullptr");
		return;
	}

	Group* group_inst = botgroup_leader->GetGroup();
	if (!group_inst || group_inst->GetLeader() != botgroup_leader) {
		c->Message(m_fail, "%s is not the current leader of a group", botgroup_leader->GetCleanName());
		return;
	}

	botgroup_id = 0;
	if (!botdb.LoadBotGroupIDByLeaderID(botgroup_leader->GetBotID(), botgroup_id)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadBotGroupIDByLeaderID(), botgroup_leader->GetCleanName());
		return;
	}
	if (!botgroup_id) {
		c->Message(m_fail, "%s is not the current leader of a bot-group", botgroup_leader->GetCleanName());
		return;
	}

	if (!Bot::AddBotToGroup(new_member, group_inst)) {
		c->Message(m_fail, "Could not add %s as a new member to %s's group", new_member->GetCleanName(), botgroup_leader->GetCleanName());
		return;
	}

	database.SetGroupID(new_member->GetName(), group_inst->GetID(), new_member->GetBotID());

	if (!botdb.AddMemberToBotGroup(botgroup_leader->GetBotID(), new_member->GetBotID())) {
		c->Message(m_fail, "%s - %s->%s", BotDatabase::fail::AddMemberToBotGroup(), new_member->GetCleanName(), botgroup_leader->GetCleanName());
		Bot::RemoveBotFromGroup(new_member, botgroup_leader->GetGroup());
		return;
	}

	std::string botgroup_name;
	if (!botdb.LoadBotGroupNameByLeaderID(botgroup_leader->GetBotID(), botgroup_name))
		c->Message(m_fail, "%s", BotDatabase::fail::LoadBotGroupNameByLeaderID());

	c->Message(m_action, "Successfully added %s to bot-group %s", new_member->GetCleanName(), botgroup_name.c_str());
}

void bot_subcommand_botgroup_create(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_botgroup_create", sep->arg[0], "botgroupcreate"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_leader>) %s [group_name] ([leader_name])", sep->arg[0]);
		return;
	}

	std::string botgroup_name_arg = sep->arg[1];
	if (botgroup_name_arg.empty()) {
		c->Message(m_fail, "You must specify a [name] for this bot-group to use this command");
		return;
	}

	bool extant_flag = false;
	if (!botdb.QueryBotGroupExistence(botgroup_name_arg, extant_flag)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::QueryBotGroupExistence(), botgroup_name_arg.c_str());
		return;
	}
	if (extant_flag) {
		c->Message(m_fail, "The [name] %s already exists for a bot-group. Please choose another", botgroup_name_arg.c_str());
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a group leader as a bot that you own to use this command");
		return;
	}

	auto botgroup_leader = sbl.front();
	if (!botgroup_leader) {
		c->Message(m_unknown, "Error: Group leader bot dereferenced to nullptr");
		return;
	}

	if (botgroup_leader->HasGroup()) {
		c->Message(m_fail, "%s is already a current member of a group", botgroup_leader->GetCleanName());
		return;
	}

	uint32 botgroup_id = 0;
	if (!botdb.LoadBotGroupIDByLeaderID(botgroup_leader->GetBotID(), botgroup_id)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadBotGroupIDByLeaderID(), botgroup_leader->GetCleanName());
		return;
	}
	if (botgroup_id) {
		c->Message(m_fail, "%s is already the current leader of a bot-group", botgroup_leader->GetCleanName());
		return;
	}

	botgroup_id = 0;
	if (!botdb.LoadBotGroupIDByMemberID(botgroup_leader->GetBotID(), botgroup_id)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadBotGroupIDByMemberID(), botgroup_leader->GetCleanName());
		return;
	}
	if (botgroup_id) {
		c->Message(m_fail, "%s is already a current member of a bot-group", botgroup_leader->GetCleanName());
		return;
	}

	Group* group_inst = new Group(botgroup_leader);
	if (!group_inst) {
		c->Message(m_unknown, "Could not create a new group instance");
		return;
	}

	if (!botdb.CreateBotGroup(botgroup_name_arg, botgroup_leader->GetBotID())) {
		c->Message(m_fail, "%s '%s'", BotDatabase::fail::CreateBotGroup(), botgroup_name_arg.c_str());
		safe_delete(group_inst);
		return;
	}

	entity_list.AddGroup(group_inst);
	database.SetGroupID(botgroup_leader->GetCleanName(), group_inst->GetID(), botgroup_leader->GetBotID());
	database.SetGroupLeaderName(group_inst->GetID(), botgroup_leader->GetCleanName());
	botgroup_leader->SetFollowID(c->GetID());

	c->Message(m_action, "Successfully created bot-group '%s' with '%s' as its leader", botgroup_name_arg.c_str(), botgroup_leader->GetCleanName());
}

void bot_subcommand_botgroup_delete(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_botgroup_delete", sep->arg[0], "botgroupdelete"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [botgroup_name]", sep->arg[0]);
		return;
	}

	std::string botgroup_name_arg = sep->arg[1];
	if (botgroup_name_arg.empty()) {
		c->Message(m_fail, "You must specify a [name] for this bot-group to use this command");
		return;
	}

	uint32 botgroup_id = 0;
	if (!botdb.LoadBotGroupIDForLoadBotGroup(c->CharacterID(), botgroup_name_arg, botgroup_id)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadBotGroupIDForLoadBotGroup(), botgroup_name_arg.c_str());
		return;
	}
	if (!botgroup_id) {
		c->Message(m_fail, "Could not locate group id for '%s'", botgroup_name_arg.c_str());
		return;
	}

	uint32 leader_id = 0;
	if (!botdb.LoadLeaderIDByBotGroupID(botgroup_id, leader_id)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadLeaderIDByBotGroupID(), botgroup_name_arg.c_str());
		return;
	}
	if (!leader_id) {
		c->Message(m_fail, "Could not locate leader id for '%s'", botgroup_name_arg.c_str());
		return;
	}

	std::list<Bot*> gbl;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	std::map<uint32, std::list<uint32>> member_list;
	if (!botdb.LoadBotGroup(botgroup_name_arg, member_list)) {
		c->Message(m_fail, "%s '%s'", BotDatabase::fail::LoadBotGroup(), botgroup_name_arg.c_str());
		return;
	}
	if (member_list.find(botgroup_id) == member_list.end() || member_list[botgroup_id].empty()) {
		c->Message(m_fail, "Could not locate member list for bot-group '%s'", botgroup_name_arg.c_str());
		return;
	}

	for (auto bot_iter : sbl) {
		for (auto group_iter : member_list[botgroup_id]) {
			if (bot_iter->GetBotID() == group_iter) {
				gbl.push_back(bot_iter);
				break;
			}
		}
	}
	gbl.unique();

	for (auto group_member : gbl) {
		if (group_member->HasGroup())
			Bot::RemoveBotFromGroup(group_member, group_member->GetGroup());
	}

	if (!botdb.DeleteBotGroup(leader_id)) {
		c->Message(m_fail, "%s '%s'", BotDatabase::fail::DeleteBotGroup(), botgroup_name_arg.c_str());
		return;
	}

	c->Message(m_action, "Successfully deleted bot-group %s", botgroup_name_arg.c_str());
}

void bot_subcommand_botgroup_list(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_botgroup_list", sep->arg[0], "botgrouplist"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s", sep->arg[0]);
		return;
	}

	std::list<std::pair<std::string, std::string>> botgroups_list;
	if (!botdb.LoadBotGroupsListByOwnerID(c->CharacterID(), botgroups_list)) {
		c->Message(m_fail, "%s", BotDatabase::fail::LoadBotGroupsListByOwnerID());
		return;
	}
	if (botgroups_list.empty()) {
		c->Message(m_fail, "You have no saved bot-groups");
		return;
	}

	int botgroup_count = 0;
	for (auto botgroups_iter : botgroups_list)
		c->Message(m_message, "(%i) Bot-group name: %s | Leader: %s", (++botgroup_count), botgroups_iter.first.c_str(), botgroups_iter.second.c_str());

	c->Message(m_action, "%i bot-groups listed", botgroup_count);
}

void bot_subcommand_botgroup_load(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_botgroup_load", sep->arg[0], "botgroupload"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [botgroup_name]", sep->arg[0]);
		return;
	}

	std::string botgroup_name_arg = sep->arg[1];
	if (botgroup_name_arg.empty()) {
		c->Message(m_fail, "You must specify the [name] of a bot-group to load to use this command");
		return;
	}

	bool extant_flag = false;
	if (!botdb.QueryBotGroupExistence(botgroup_name_arg, extant_flag)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::QueryBotGroupExistence(), botgroup_name_arg.c_str());
		return;
	}
	if (!extant_flag) {
		c->Message(m_fail, "Bot-group %s does not exist", botgroup_name_arg.c_str());
		return;
	}

	Group* owner_group = c->GetGroup();
	if (owner_group) {
		std::list<Client*> member_list;
		owner_group->GetClientList(member_list);
		member_list.remove(nullptr);

		for (auto member_iter : member_list) {
			if (member_iter->IsEngaged() || member_iter->GetAggroCount() > 0) {
				c->Message(m_fail, "You can't spawn bots while your group is engaged");
				return;
			}
		}
	}
	else {
		if (c->GetAggroCount() > 0) {
			c->Message(m_fail, "You can't spawn bots while you are engaged");
			return;
		}
	}

	uint32 botgroup_id = 0;
	if (!botdb.LoadBotGroupIDForLoadBotGroup(c->CharacterID(), botgroup_name_arg, botgroup_id) || !botgroup_id) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadBotGroupIDForLoadBotGroup(), botgroup_name_arg.c_str());
		return;
	}

	std::map<uint32, std::list<uint32>> member_list;
	if (!botdb.LoadBotGroup(botgroup_name_arg, member_list)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadBotGroup(), botgroup_name_arg.c_str());
		return;
	}
	if (member_list.find(botgroup_id) == member_list.end() || member_list[botgroup_id].empty()) {
		c->Message(m_fail, "Database returned an empty list for bot-group '%s'", botgroup_name_arg.c_str());
		return;
	}

	int spawned_bot_count = Bot::SpawnedBotCount(c->CharacterID());

	if (RuleB(Bots, QuestableSpawnLimit)) {
		int allowed_bot_count = 0;
		if (!botdb.LoadQuestableSpawnCount(c->CharacterID(), allowed_bot_count)) {
			c->Message(m_fail, "%s", BotDatabase::fail::LoadQuestableSpawnCount());
			return;
		}

		if (!allowed_bot_count) {
			c->Message(m_fail, "You can not spawn any bots");
			return;
		}

		if (spawned_bot_count >= allowed_bot_count || (spawned_bot_count + member_list.begin()->second.size()) > allowed_bot_count) {
			c->Message(m_fail, "You can not spawn more than %i bot%s (quest-limit)", allowed_bot_count, ((allowed_bot_count == 1) ? ("") : ("s")));
			return;
		}
	}

	const int allowed_bot_limit = RuleI(Bots, SpawnLimit);
	if (spawned_bot_count >= allowed_bot_limit || (spawned_bot_count + member_list.begin()->second.size()) > allowed_bot_limit) {
		c->Message(m_fail, "You can not spawn more than %i bot%s (hard-limit)", allowed_bot_limit, ((allowed_bot_limit == 1) ? ("") : ("s")));
		return;
	}

	uint32 leader_id = 0;
	if (!botdb.LoadLeaderIDByBotGroupName(botgroup_name_arg, leader_id)) {
		c->Message(m_fail, "%s for '%s'", BotDatabase::fail::LoadLeaderIDByBotGroupName(), botgroup_name_arg.c_str());
		return;
	}
	if (!leader_id) {
		c->Message(m_fail, "Can not locate bot-group leader id for '%s'", botgroup_name_arg.c_str());
		return;
	}

	auto botgroup_leader = Bot::LoadBot(leader_id);
	if (!botgroup_leader) {
		c->Message(m_fail, "Could not spawn bot-group leader for '%s'", botgroup_name_arg.c_str());
		safe_delete(botgroup_leader);
		return;
	}

	botgroup_leader->Spawn(c);
	
	Group* group_inst = new Group(botgroup_leader);

	entity_list.AddGroup(group_inst);
	database.SetGroupID(botgroup_leader->GetCleanName(), group_inst->GetID(), botgroup_leader->GetBotID());
	database.SetGroupLeaderName(group_inst->GetID(), botgroup_leader->GetCleanName());
	botgroup_leader->SetFollowID(c->GetID());

	member_list[botgroup_id].remove(0);
	member_list[botgroup_id].remove(botgroup_leader->GetBotID());
	for (auto member_iter : member_list[botgroup_id]) {
		auto botgroup_member = Bot::LoadBot(member_iter);
		if (!botgroup_member) {
			c->Message(m_fail, "Could not load bot id %i", member_iter);
			safe_delete(botgroup_member);
			return;
		}

		botgroup_member->Spawn(c);
		Bot::AddBotToGroup(botgroup_member, group_inst);
	}

	c->Message(m_action, "Successfully loaded bot-group %s", botgroup_name_arg.c_str());
}

void bot_subcommand_botgroup_remove_member(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_botgroup_remove_member", sep->arg[0], "botgroupremovemember"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a group member as a bot that you own to use this command");
		return;
	}

	auto group_member = sbl.front();
	if (!group_member) {
		c->Message(m_unknown, "Error: Group member bot dereferenced to nullptr");
		return;
	}

	if (!group_member->HasGroup()) {
		c->Message(m_fail, "%s is not a current member of a group", group_member->GetCleanName());
		return;
	}

	if (!Bot::RemoveBotFromGroup(group_member, group_member->GetGroup())) {
		c->Message(m_fail, "Could not remove %s from their group", group_member->GetCleanName());
		return;
	}

	if (!botdb.RemoveMemberFromBotGroup(group_member->GetBotID())) {
		c->Message(m_fail, "%s - '%s'", BotDatabase::fail::RemoveMemberFromBotGroup(), group_member->GetCleanName());
		return;
	}

	c->Message(m_action, "Successfully removed %s from their bot-group", group_member->GetCleanName());
}

void bot_subcommand_circle(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Depart];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Depart) || helper_command_alias_fail(c, "bot_subcommand_circle", sep->arg[0], "circle"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [list | destination] ([option: single])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Depart, DRUID);
		return;
	}

	bool single = false;
	std::string single_arg = sep->arg[2];
	if (!single_arg.compare("single"))
		single = true;

	std::string destination = sep->arg[1];
	if (!destination.compare("list")) {
		auto my_druid_bot = ActionableBots::AsGroupMember_ByClass(c, c, DRUID);
		helper_command_depart_list(c, my_druid_bot, nullptr, local_list, single);
		return;
	}
	else if (destination.empty()) {
		c->Message(m_fail, "A [destination] or [list] argument is required to use this command");
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToDepart();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->caster_class != DRUID)
			continue;
		if (local_entry->single != single)
			continue;
		if (destination.compare(spells[local_entry->spell_id].teleport_zone))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}

void bot_subcommand_heal_rotation_adaptive_targeting(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_adaptive_targeting", sep->arg[0], "healrotationadaptivetargeting"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name]) ([option: on | off])", sep->arg[0]);
		return;
	}

	std::string adaptive_targeting_arg;

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		adaptive_targeting_arg = sep->arg[2];
	}
	else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		adaptive_targeting_arg = sep->arg[1];
	}

	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	bool hr_adaptive_targeting = false;

	if (!adaptive_targeting_arg.compare("on")) {
		hr_adaptive_targeting = true;
	}
	else if (adaptive_targeting_arg.compare("off")) {
		c->Message(m_action, "Adaptive targeting is currently '%s' for %s's Heal Rotation", (((*current_member->MemberOfHealRotation())->AdaptiveTargeting()) ? ("on") : ("off")), current_member->GetCleanName());
		return;
	}

	(*current_member->MemberOfHealRotation())->SetAdaptiveTargeting(hr_adaptive_targeting);

	c->Message(m_action, "Adaptive targeting is now '%s' for %s's Heal Rotation", (((*current_member->MemberOfHealRotation())->AdaptiveTargeting()) ? ("on") : ("off")), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_add_member(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_add_member", sep->arg[0], "healrotationaddmember"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s [new_member_name] ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty()) {
		c->Message(m_fail, "You must [name] a new member as a bot that you own to use this command");
		return;
	}

	auto new_member = sbl.front();
	if (!new_member) {
		c->Message(m_unknown, "Error: New member bot dereferenced to nullptr");
		return;
	}

	if (new_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is already a current member of a Heal Rotation and can not join another one", new_member->GetCleanName());
		return;
	}

	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!new_member->JoinHealRotationMemberPool(current_member->MemberOfHealRotation())) {
		c->Message(m_fail, "Failed to add %s as a current member of this Heal Rotation", new_member->GetCleanName());
		return;
	}

	c->Message(m_action, "Successfully added %s as a current member of this Heal Rotation", new_member->GetCleanName());
}

void bot_subcommand_heal_rotation_add_target(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_add_target", sep->arg[0], "healrotationaddtarget"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s [heal_target_name] ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	auto heal_target = entity_list.GetMob(sep->arg[1]);
	if (!heal_target) {
		c->Message(m_fail, "No target exists by the name '%s'", sep->arg[1]);
		return;
	}

	if ((!heal_target->IsClient() && !heal_target->IsBot() && !heal_target->IsPet()) ||
		(heal_target->IsPet() && (!heal_target->GetOwner() || (!heal_target->GetOwner()->IsClient() && !heal_target->GetOwner()->IsBot()))))
	{
		c->Message(m_fail, "%s's entity type is not an allowable heal target", heal_target->GetCleanName());
		return;
	}

	if (!heal_target->JoinHealRotationTargetPool(current_member->MemberOfHealRotation())) {
		c->Message(m_fail, "Failed to add heal target with a name of '%s'", heal_target->GetCleanName());
		return;
	}

	c->Message(m_action, "Successfully added heal target %s to %s's Heal Rotation", heal_target->GetCleanName(), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_adjust_critical(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_adjust_critical", sep->arg[0], "healrotationadjustcritical"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s [armor_type] [value: %3.1f-%3.1f | + | -] ([member_name])", sep->arg[0], CRITICAL_HP_RATIO_BASE, SAFE_HP_RATIO_BASE);
		c->Message(m_note, "armor_types: %u(Base), %u(Cloth), %u(Leather), %u(Chain), %u(Plate)",
			ARMOR_TYPE_UNKNOWN, ARMOR_TYPE_CLOTH, ARMOR_TYPE_LEATHER, ARMOR_TYPE_CHAIN, ARMOR_TYPE_PLATE);
		return;
	}

	std::string armor_type_arg = sep->arg[1];
	std::string critical_arg = sep->arg[2];

	uint8 armor_type_value = 255;
	if (sep->IsNumber(1))
		armor_type_value = atoi(armor_type_arg.c_str());

	if (armor_type_value > ARMOR_TYPE_LAST) {
		c->Message(m_fail, "You must specify a valid [armor_type: %u-%u] to use this command", ARMOR_TYPE_FIRST, ARMOR_TYPE_LAST);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[3]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	float critical_ratio = CRITICAL_HP_RATIO_BASE;
	if (sep->IsNumber(2))
		critical_ratio = atof(critical_arg.c_str());
	else if (!critical_arg.compare("+"))
		critical_ratio = (*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(armor_type_value) + HP_RATIO_DELTA;
	else if (!critical_arg.compare("-"))
		critical_ratio = (*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(armor_type_value) - HP_RATIO_DELTA;

	if (critical_ratio > SAFE_HP_RATIO_ABS)
		critical_ratio = SAFE_HP_RATIO_ABS;
	if (critical_ratio < CRITICAL_HP_RATIO_ABS)
		critical_ratio = CRITICAL_HP_RATIO_ABS;

	if (!(*current_member->MemberOfHealRotation())->SetArmorTypeCriticalHPRatio(armor_type_value, critical_ratio)) {
		c->Message(m_fail, "Critical value %3.1f%%(%u) exceeds safe value %3.1f%%(%u) for %s's Heal Rotation",
			critical_ratio, armor_type_value, (*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(armor_type_value), armor_type_value, current_member->GetCleanName());
		return;
	}

	c->Message(m_action, "Class Armor Type %u critical value %3.1f%% set for %s's Heal Rotation",
		armor_type_value, (*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(armor_type_value), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_adjust_safe(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_adjust_safe", sep->arg[0], "healrotationadjustsafe"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s [armor_type] [value: %3.1f-%3.1f | + | -] ([member_name])", sep->arg[0], CRITICAL_HP_RATIO_BASE, SAFE_HP_RATIO_BASE);
		c->Message(m_note, "armor_types: %u(Base), %u(Cloth), %u(Leather), %u(Chain), %u(Plate)",
			ARMOR_TYPE_UNKNOWN, ARMOR_TYPE_CLOTH, ARMOR_TYPE_LEATHER, ARMOR_TYPE_CHAIN, ARMOR_TYPE_PLATE);
		return;
	}

	std::string armor_type_arg = sep->arg[1];
	std::string safe_arg = sep->arg[2];

	uint8 armor_type_value = 255;
	if (sep->IsNumber(1))
		armor_type_value = atoi(armor_type_arg.c_str());

	if (armor_type_value > ARMOR_TYPE_LAST) {
		c->Message(m_fail, "You must specify a valid [armor_type: %u-%u] to use this command", ARMOR_TYPE_FIRST, ARMOR_TYPE_LAST);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[3]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	float safe_ratio = SAFE_HP_RATIO_BASE;
	if (sep->IsNumber(2))
		safe_ratio = atof(safe_arg.c_str());
	else if (!safe_arg.compare("+"))
		safe_ratio = (*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(armor_type_value) + HP_RATIO_DELTA;
	else if (!safe_arg.compare("-"))
		safe_ratio = (*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(armor_type_value) - HP_RATIO_DELTA;

	if (safe_ratio > SAFE_HP_RATIO_ABS)
		safe_ratio = SAFE_HP_RATIO_ABS;
	if (safe_ratio < CRITICAL_HP_RATIO_ABS)
		safe_ratio = CRITICAL_HP_RATIO_ABS;

	if (!(*current_member->MemberOfHealRotation())->SetArmorTypeSafeHPRatio(armor_type_value, safe_ratio)) {
		c->Message(m_fail, "Safe value %3.1f%%(%u) does not exceed critical value %3.1f%%(%u) for %s's Heal Rotation",
			safe_ratio, armor_type_value, (*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(armor_type_value), armor_type_value, current_member->GetCleanName());
		return;
	}

	c->Message(m_action, "Class Armor Type %u safe value %3.1f%% set for %s's Heal Rotation",
		armor_type_value, (*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(armor_type_value), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_casting_override(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_casting_override", sep->arg[0], "healrotationcastingoverride"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name]) ([option: on | off])", sep->arg[0]);
		return;
	}

	std::string casting_override_arg;

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		casting_override_arg = sep->arg[2];
	}
	else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		casting_override_arg = sep->arg[1];
	}

	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	bool hr_casting_override = false;

	if (!casting_override_arg.compare("on")) {
		hr_casting_override = true;
	}
	else if (casting_override_arg.compare("off")) {
		c->Message(m_action, "Casting override is currently '%s' for %s's Heal Rotation", (((*current_member->MemberOfHealRotation())->CastingOverride()) ? ("on") : ("off")), current_member->GetCleanName());
		return;
	}

	(*current_member->MemberOfHealRotation())->SetCastingOverride(hr_casting_override);

	c->Message(m_action, "Casting override is now '%s' for %s's Heal Rotation", (((*current_member->MemberOfHealRotation())->CastingOverride()) ? ("on") : ("off")), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_change_interval(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_change_interval", sep->arg[0], "healrotationchangeinterval"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name]) ([interval=%u: %u-%u(seconds)])",
			sep->arg[0], CASTING_CYCLE_DEFAULT_INTERVAL_S, CASTING_CYCLE_MINIMUM_INTERVAL_S, CASTING_CYCLE_MAXIMUM_INTERVAL_S);
		return;
	}

	std::string change_interval_arg;

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		change_interval_arg = sep->arg[2];
	}
	else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		change_interval_arg = sep->arg[1];
	}

	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	uint32 hr_change_interval_s = CASTING_CYCLE_DEFAULT_INTERVAL_S;

	if (!change_interval_arg.empty()) {
		hr_change_interval_s = atoi(change_interval_arg.c_str());
	}
	else {
		hr_change_interval_s = (*current_member->MemberOfHealRotation())->IntervalS();
		c->Message(m_action, "Casting interval is currently '%i' second%s for %s's Heal Rotation", hr_change_interval_s, ((hr_change_interval_s == 1) ? ("") : ("s")), current_member->GetCleanName());
		return;
	}

	if (hr_change_interval_s < CASTING_CYCLE_MINIMUM_INTERVAL_S || hr_change_interval_s > CASTING_CYCLE_MAXIMUM_INTERVAL_S)
		hr_change_interval_s = CASTING_CYCLE_DEFAULT_INTERVAL_S;

	(*current_member->MemberOfHealRotation())->SetIntervalS(hr_change_interval_s);

	hr_change_interval_s = (*current_member->MemberOfHealRotation())->IntervalS();
	c->Message(m_action, "Casting interval is now '%i' second%s for %s's Heal Rotation", hr_change_interval_s, ((hr_change_interval_s == 1) ? ("") : ("s")), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_clear_hot(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_clear_hot", sep->arg[0], "healrotationclearhot"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->ClearHOTTarget()) {
		c->Message(m_fail, "Failed to clear %s's Heal Rotation HOT", current_member->GetCleanName());
	}

	c->Message(m_action, "Succeeded in clearing %s's Heal Rotation HOT", current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_clear_targets(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_clear_targets", sep->arg[0], "healrotationcleartargets"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->ClearTargetPool()) {
		c->Message(m_fail, "Failed to clear all targets from %s's Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(m_action, "All targets have been cleared from %s's Heal Rotation", current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_create(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_create", sep->arg[0], "healrotationcreate"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_creator>) %s ([creator_name]) ([interval=%u: %u-%u(seconds)] [fastheals=off: on | off] [adaptivetargeting=off: on | off] [castingoverride=off: on | off])",
			sep->arg[0], CASTING_CYCLE_DEFAULT_INTERVAL_S, CASTING_CYCLE_MINIMUM_INTERVAL_S, CASTING_CYCLE_MAXIMUM_INTERVAL_S);
		return;
	}
	
	std::string interval_arg;
	std::string fast_heals_arg;
	std::string adaptive_targeting_arg;
	std::string casting_override_arg;

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		interval_arg = sep->arg[2];
		fast_heals_arg = sep->arg[3];
		adaptive_targeting_arg = sep->arg[4];
		casting_override_arg = sep->arg[5];
	}
	else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		interval_arg = sep->arg[1];
		fast_heals_arg = sep->arg[2];
		adaptive_targeting_arg = sep->arg[3];
		casting_override_arg = sep->arg[4];
	}

	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a creator as a bot that you own to use this command");
		return;
	}

	auto creator_member = sbl.front();
	if (!creator_member) {
		c->Message(m_unknown, "Error: Creator bot dereferenced to nullptr");
		return;
	}

	if (creator_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is already a current member of a Heal Rotation", creator_member->GetCleanName());
		return;
	}

	uint32 hr_interval_s = CASTING_CYCLE_DEFAULT_INTERVAL_S;
	bool hr_fast_heals = false;
	bool hr_adaptive_targeting = false;
	bool hr_casting_override = false;

	if (!casting_override_arg.compare("on")) {
		hr_casting_override = true;
		if (!adaptive_targeting_arg.compare("on"))
			hr_adaptive_targeting = true;
		if (!fast_heals_arg.compare("on"))
			hr_fast_heals = true;
		hr_interval_s = atoi(interval_arg.c_str());
	}
	else if (!casting_override_arg.compare("off")) {
		if (!adaptive_targeting_arg.compare("on"))
			hr_adaptive_targeting = true;
		if (!fast_heals_arg.compare("on"))
			hr_fast_heals = true;
		hr_interval_s = atoi(interval_arg.c_str());
	}
	
	if (hr_interval_s < CASTING_CYCLE_MINIMUM_INTERVAL_S || hr_interval_s > CASTING_CYCLE_MAXIMUM_INTERVAL_S)
		hr_interval_s = CASTING_CYCLE_DEFAULT_INTERVAL_S;
	
	hr_interval_s *= 1000; // convert to milliseconds for Bot/HealRotation constructor

	if (!creator_member->CreateHealRotation(hr_interval_s, hr_fast_heals, hr_adaptive_targeting, hr_casting_override)) {
		c->Message(m_fail, "Failed to add %s as a current member to a new Heal Rotation", creator_member->GetCleanName());
		return;
	}

	std::list<uint32> member_list;
	std::list<std::string> target_list;
	bool load_flag = false;
	bool member_fail = false;
	bool target_fail = false;

	if (!botdb.LoadHealRotation(creator_member, member_list, target_list, load_flag, member_fail, target_fail))
		c->Message(m_fail, "%s", BotDatabase::fail::LoadHealRotation());
	
	if (!load_flag) {
		c->Message(m_action, "Successfully added %s as a current member to a new Heal Rotation", creator_member->GetCleanName());
		return;
	}
	
	if (!member_fail) {
		MyBots::PopulateSBL_BySpawnedBots(c, sbl);
		for (auto member_iter : member_list) {
			if (!member_iter || member_iter == creator_member->GetBotID())
				continue;

			bool member_found = false;
			for (auto bot_iter : sbl) {
				if (bot_iter->GetBotID() != member_iter)
					continue;

				if (!bot_iter->JoinHealRotationMemberPool(creator_member->MemberOfHealRotation()))
					c->Message(m_fail, "Failed to add member '%s'", bot_iter->GetCleanName());
				member_found = true;

				break;
			}

			if (!member_found)
				c->Message(m_fail, "Could not locate member with bot id '%u'", member_iter);
		}
	}
	else {
		c->Message(m_fail, "%s", BotDatabase::fail::LoadHealRotationMembers());
	}

	if (!target_fail) {
		for (auto target_iter : target_list) {
			if (target_iter.empty())
				continue;

			auto target_mob = entity_list.GetMob(target_iter.c_str());
			if (!target_mob) {
				c->Message(m_fail, "Could not locate target '%s'", target_iter.c_str());
				continue;
			}

			if (!target_mob->JoinHealRotationTargetPool(creator_member->MemberOfHealRotation()))
				c->Message(m_fail, "Failed to add target '%s'", target_mob->GetCleanName());
		}
	}
	else {
		c->Message(m_fail, "%s", BotDatabase::fail::LoadHealRotationTargets());
	}
	
	c->Message(m_action, "Successfully loaded %s's Heal Rotation", creator_member->GetCleanName());
}

void bot_subcommand_heal_rotation_delete(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_delete", sep->arg[0], "healrotationdelete"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([option: all]) ([member_name])", sep->arg[0]);
		return;
	}

	bool all_flag = false;
	int name_arg = 1;
	if (!strcasecmp(sep->arg[1], "all")) {
		all_flag = true;
		name_arg = 2;
	}

	if (all_flag) {
		if (botdb.DeleteAllHealRotations(c->CharacterID()))
			c->Message(m_action, "Succeeded in deleting all heal rotations");
		else
			c->Message(m_fail, "%s", BotDatabase::fail::DeleteAllHealRotations());

		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[name_arg]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!botdb.DeleteHealRotation(current_member->GetBotID())) {
		c->Message(m_fail, "%s", BotDatabase::fail::DeleteHealRotation());
		return;
	}

	c->Message(m_action, "Succeeded in deleting %s's heal rotation", current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_fast_heals(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_fast_heals", sep->arg[0], "healrotationfastheals"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name]) ([option: on | off])", sep->arg[0]);
		return;
	}

	std::string fast_heals_arg;

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (!sbl.empty()) {
		fast_heals_arg = sep->arg[2];
	}
	else {
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
		fast_heals_arg = sep->arg[1];
	}

	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	bool hr_fast_heals = false;

	if (!fast_heals_arg.compare("on")) {
		hr_fast_heals = true;
	}
	else if (fast_heals_arg.compare("off")) {
		c->Message(m_action, "Fast heals are currently '%s' for %s's Heal Rotation", (((*current_member->MemberOfHealRotation())->FastHeals()) ? ("on") : ("off")), current_member->GetCleanName());
		return;
	}

	(*current_member->MemberOfHealRotation())->SetFastHeals(hr_fast_heals);

	c->Message(m_action, "Fast heals are now '%s' for %s's Heal Rotation", (((*current_member->MemberOfHealRotation())->FastHeals()) ? ("on") : ("off")), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_list(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_list", sep->arg[0], "healrotationlist"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(m_note, "Heal Rotation Settings:");

	c->Message(m_message, "Current state: %s", (((*current_member->MemberOfHealRotation())->IsActive()) ? ("active") : ("inactive")));
	c->Message(m_message, "Casting interval: %i seconds", (*current_member->MemberOfHealRotation())->IntervalS());
	c->Message(m_message, "Fast heals: '%s'", (((*current_member->MemberOfHealRotation())->FastHeals()) ? ("on") : ("off")));
	c->Message(m_message, "Adaptive targeting: '%s'", (((*current_member->MemberOfHealRotation())->AdaptiveTargeting()) ? ("on") : ("off")));
	c->Message(m_message, "Casting override: '%s'", (((*current_member->MemberOfHealRotation())->CastingOverride()) ? ("on") : ("off")));
	c->Message(m_message, "HOT state: %s", (((*current_member->MemberOfHealRotation())->IsHOTActive()) ? ("active") : ("inactive")));
	c->Message(m_message, "HOT target: %s", (((*current_member->MemberOfHealRotation())->HOTTarget()) ? ((*current_member->MemberOfHealRotation())->HOTTarget()->GetCleanName()) : ("null")));

	c->Message(m_message, "Base hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_UNKNOWN),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_UNKNOWN));
	c->Message(m_message, "Cloth hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_CLOTH),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_CLOTH));
	c->Message(m_message, "Leather hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_LEATHER),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_LEATHER));
	c->Message(m_message, "Chain hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_CHAIN),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_CHAIN));
	c->Message(m_message, "Plate hp limits - critical: %3.1f%%, safe: %3.1f%%",
		(*current_member->MemberOfHealRotation())->ArmorTypeCriticalHPRatio(ARMOR_TYPE_PLATE),
		(*current_member->MemberOfHealRotation())->ArmorTypeSafeHPRatio(ARMOR_TYPE_PLATE));

	c->Message(m_note, "Heal Rotation Members:");

	int member_index = 0;
	auto member_pool = (*current_member->MemberOfHealRotation())->MemberList();
	for (auto member_iter : *member_pool) {
		if (!member_iter)
			continue;

		c->Message(m_message, "(%i) %s", (++member_index), member_iter->GetCleanName());
	}
	if (!member_index)
		c->Message(m_fail, "(0) None");

	c->Message(m_note, "Heal Rotation Targets:");

	int target_index = 0;
	auto target_pool = (*current_member->MemberOfHealRotation())->TargetList();
	for (auto target_iter : *target_pool) {
		if (!target_iter)
			continue;

		c->Message(m_message, "(%i) %s", (++target_index), target_iter->GetCleanName());
	}
	if (!target_index)
		c->Message(m_message, "(0) None");
}

void bot_subcommand_heal_rotation_remove_member(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_remove_member", sep->arg[0], "healrotationremovemember"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!current_member->LeaveHealRotationMemberPool()) {
		c->Message(m_fail, "Failed to remove %s from their Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(m_action, "%s has been removed from their Heal Rotation", current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_remove_target(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_remove_target", sep->arg[0], "healrotationremovetarget"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s [heal_target_name] ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	auto heal_target = entity_list.GetMob(sep->arg[1]);
	if (!heal_target) {
		c->Message(m_fail, "No target exists by the name '%s'", sep->arg[1]);
		return;
	}

	if (!current_member->MemberOfHealRotation()->get()->IsTargetInPool(heal_target) || !heal_target->LeaveHealRotationTargetPool()) {
		c->Message(m_fail, "Failed to remove heal target with a name of '%s'", heal_target->GetCleanName());
		return;
	}

	c->Message(m_action, "Successfully removed heal target %s from %s's Heal Rotation", heal_target->GetCleanName(), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_reset_limits(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_reset_limits", sep->arg[0], "healrotationresetlimits"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	(*current_member->MemberOfHealRotation())->ResetArmorTypeHPLimits();

	c->Message(m_action, "Class Armor Type HP limit criteria has been set to default values for %s's Heal Rotation", current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_save(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_save", sep->arg[0], "healrotationsave"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	bool member_fail = false;
	bool target_fail = false;
	if (!botdb.SaveHealRotation(current_member, member_fail, target_fail)) {
		c->Message(m_fail, "%s", BotDatabase::fail::SaveHealRotation());
		return;
	}
	if (member_fail)
		c->Message(m_fail, "Failed to save heal rotation members");
	if (target_fail)
		c->Message(m_fail, "Failed to save heal rotation targets");

	c->Message(m_action, "Succeeded in saving %s's heal rotation", current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_set_hot(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_set_hot", sep->arg[0], "healrotationsethot"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s [heal_override_target_name] ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[2]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	auto hot_target = entity_list.GetMob(sep->arg[1]);
	if (!hot_target) {
		c->Message(m_fail, "No target exists by the name '%s'", sep->arg[1]);
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->IsTargetInPool(hot_target)) {
		c->Message(m_fail, "%s is not a target in %s's Heal Rotation", hot_target->GetCleanName(), current_member->GetCleanName());
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->SetHOTTarget(hot_target)) {
		c->Message(m_fail, "Failed to set %s as the HOT in %s's Heal Rotation", hot_target->GetCleanName(), current_member->GetCleanName());
		return;
	}

	c->Message(m_action, "Succeeded in setting %s as the HOT in %s's Heal Rotation", hot_target->GetCleanName(), current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_start(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_start", sep->arg[0], "healrotationstart"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if ((*current_member->MemberOfHealRotation())->IsActive()) {
		c->Message(m_fail, "%s's Heal Rotation is already active", current_member->GetCleanName());
		return;
	}

	if (!current_member->MemberOfHealRotation()->get()->Start()) {
		c->Message(m_fail, "Failed to start %s's Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(m_action, "%s's Heal Rotation is now active", current_member->GetCleanName());
}

void bot_subcommand_heal_rotation_stop(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_heal_rotation_stop", sep->arg[0], "healrotationstop"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: (<target_member>) %s ([member_name])", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_ByNamedBot(c, sbl, sep->arg[1]);
	if (sbl.empty())
		MyBots::PopulateSBL_ByTargetedBot(c, sbl);
	if (sbl.empty()) {
		c->Message(m_fail, "You must <target> or [name] a current member as a bot that you own to use this command");
		return;
	}

	auto current_member = sbl.front();
	if (!current_member) {
		c->Message(m_unknown, "Error: Current member bot dereferenced to nullptr");
		return;
	}

	if (!current_member->IsHealRotationMember()) {
		c->Message(m_fail, "%s is not a current member of a Heal Rotation", current_member->GetCleanName());
		return;
	}

	if (!(*current_member->MemberOfHealRotation())->IsActive()) {
		c->Message(m_fail, "%s's Heal Rotation is already inactive", current_member->GetCleanName());
		return;
	}

	if (!current_member->MemberOfHealRotation()->get()->Stop()) {
		c->Message(m_fail, "Failed to stop %s's Heal Rotation", current_member->GetCleanName());
		return;
	}

	c->Message(m_action, "%s's Heal Rotation is now inactive", current_member->GetCleanName());
}

void bot_subcommand_inventory_give(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_inventory_give", sep->arg[0], "inventorygive"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	auto my_bot = sbl.front();
	if (!my_bot) {
		c->Message(m_unknown, "ActionableBots returned 'nullptr'");
		return;
	}

	my_bot->FinishTrade(c, Bot::BotTradeClientNoDropNoTrade);
}

void bot_subcommand_inventory_list(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_inventory_list", sep->arg[0], "inventorylist"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	auto my_bot = sbl.front();
	if (!my_bot) {
		c->Message(m_unknown, "ActionableBots returned 'nullptr'");
		return;
	}

	const ItemInst* inst = nullptr;
	const EQEmu::ItemBase* item = nullptr;
	bool is2Hweapon = false;

	std::string item_link;
	EQEmu::SayLinkEngine linker;
	linker.SetLinkType(EQEmu::saylink::SayLinkItemInst);

	uint32 inventory_count = 0;
	for (int i = EQEmu::legacy::EQUIPMENT_BEGIN; i <= (EQEmu::legacy::EQUIPMENT_END + 1); ++i) {
		if ((i == EQEmu::legacy::SlotSecondary) && is2Hweapon)
			continue;

		inst = my_bot->CastToBot()->GetBotItem(i == 22 ? EQEmu::legacy::SlotPowerSource : i);
		if (!inst || !inst->GetItem()) {
			c->Message(m_message, "I need something for my %s (slot %i)", GetBotEquipSlotName(i), (i == 22 ? EQEmu::legacy::SlotPowerSource : i));
			continue;
		}
		
		item = inst->GetItem();
		if ((i == EQEmu::legacy::SlotPrimary) && item->IsType2HWeapon()) {
			is2Hweapon = true;
		}

		linker.SetItemInst(inst);
		item_link = linker.GenerateLink();
		c->Message(m_message, "Using %s in my %s (slot %i)", item_link.c_str(), GetBotEquipSlotName(i), (i == 22 ? EQEmu::legacy::SlotPowerSource : i));

		++inventory_count;
	}

	uint32 database_count = 0;
	if (!botdb.QueryInventoryCount(my_bot->GetBotID(), database_count))
		c->Message(m_unknown, "%s", BotDatabase::fail::QueryInventoryCount());

	if (inventory_count != database_count)
		c->Message(m_unknown, "Inventory-database item count mismatch: inv = '%u', db = '%u'", inventory_count, database_count);
}

void bot_subcommand_inventory_remove(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_inventory_remove", sep->arg[0], "inventoryremove"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [slotid: 0-22] ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	if (c->GetTradeskillObject() || (c->trade->state == Trading)) {
		c->Message_StringID(MT_Tell, MERCHANT_BUSY);
		return;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]) == ActionableBots::ABT_None)
		return;

	auto my_bot = sbl.front();
	if (!my_bot) {
		c->Message(m_unknown, "ActionableBots returned 'nullptr'");
		return;
	}

	int slotId = atoi(sep->arg[1]);
	if (!sep->IsNumber(1) || ((slotId > EQEmu::legacy::EQUIPMENT_END || slotId < EQEmu::legacy::EQUIPMENT_BEGIN) && slotId != EQEmu::legacy::SlotPowerSource)) {
		c->Message(m_fail, "Valid slots are 0-21 or 9999");
		return;
	}

	const EQEmu::ItemBase* itm = nullptr;
	const ItemInst* itminst = my_bot->GetBotItem(slotId);
	if (itminst)
		itm = itminst->GetItem();

	if (itminst && itm && c->CheckLoreConflict(itm)) {
		c->Message_StringID(0, PICK_LORE);
		return;
	}

	for (int m = AUG_INDEX_BEGIN; m < EQEmu::legacy::ITEM_COMMON_SIZE; ++m) {
		if (!itminst)
			break;

		ItemInst *itma = itminst->GetAugment(m);
		if (!itma)
			continue;
		if (!c->CheckLoreConflict(itma->GetItem()))
			continue;

		c->Message_StringID(0, PICK_LORE);
		return;
	}
	
	std::string error_message;
	if (itm) {
		c->PushItemOnCursor(*itminst, true);
		if ((slotId == EQEmu::legacy::SlotRange) || (slotId == EQEmu::legacy::SlotAmmo) || (slotId == EQEmu::legacy::SlotPrimary) || (slotId == EQEmu::legacy::SlotSecondary))
			my_bot->SetBotArcher(false);

		my_bot->RemoveBotItemBySlot(slotId, &error_message);
		if (!error_message.empty()) {
			c->Message(m_unknown, "Database Error: %s", error_message.c_str());
			return;
		}

		my_bot->BotRemoveEquipItem(slotId);
		my_bot->CalcBotStats();
	}

	switch (slotId) {
	case EQEmu::legacy::SlotCharm:
	case EQEmu::legacy::SlotEar1:
	case EQEmu::legacy::SlotHead:
	case EQEmu::legacy::SlotFace:
	case EQEmu::legacy::SlotEar2:
	case EQEmu::legacy::SlotNeck:
	case EQEmu::legacy::SlotBack:
	case EQEmu::legacy::SlotWrist1:
	case EQEmu::legacy::SlotWrist2:
	case EQEmu::legacy::SlotRange:
	case EQEmu::legacy::SlotPrimary:
	case EQEmu::legacy::SlotSecondary:
	case EQEmu::legacy::SlotFinger1:
	case EQEmu::legacy::SlotFinger2:
	case EQEmu::legacy::SlotChest:
	case EQEmu::legacy::SlotWaist:
	case EQEmu::legacy::SlotPowerSource:
	case EQEmu::legacy::SlotAmmo:
		c->Message(m_message, "My %s is %s unequipped", GetBotEquipSlotName(slotId), ((itm) ? ("now") : ("already")));
		break;
	case EQEmu::legacy::SlotShoulders:
	case EQEmu::legacy::SlotArms:
	case EQEmu::legacy::SlotHands:
	case EQEmu::legacy::SlotLegs:
	case EQEmu::legacy::SlotFeet:
		c->Message(m_message, "My %s are %s unequipped", GetBotEquipSlotName(slotId), ((itm) ? ("now") : ("already")));
		break;
	default:
		c->Message(m_fail, "I'm soo confused...");
		break;
	}
}

void bot_subcommand_inventory_window(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_inventory_window", sep->arg[0], "inventorywindow"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [actionable: target]", sep->arg[0]);
		return;
	}
	int ab_mask = ActionableBots::ABM_Target;

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	auto my_bot = sbl.front();
	if (!my_bot) {
		c->Message(m_unknown, "ActionableBots returned 'nullptr'");
		return;
	}

	std::string window_title = my_bot->GetCleanName();
	window_title.append("`s Inventory");

	std::string window_text;
	//std::string item_link;
	//Client::TextLink linker;
	//linker.SetLinkType(linker.linkItemInst);

	for (int i = EQEmu::legacy::EQUIPMENT_BEGIN; i <= (EQEmu::legacy::EQUIPMENT_END + 1); ++i) {
		const EQEmu::ItemBase* item = nullptr;
		const ItemInst* inst = my_bot->CastToBot()->GetBotItem(i == 22 ? EQEmu::legacy::SlotPowerSource : i);
		if (inst)
			item = inst->GetItem();

		window_text.append("<c \"#FFFFFF\">");
		window_text.append(GetBotEquipSlotName(i == 22 ? EQEmu::legacy::SlotPowerSource : i));
		window_text.append(": ");
		if (item) {
			//window_text.append("</c>");
			//linker.SetItemInst(inst);
			//item_link = linker.GenerateLink();
			//window_text.append(item_link.c_str());

			window_text.append("<c \"#00FF00\">");
			window_text.append(StringFormat("%s", item->Name));
		}
		else {
			window_text.append("<c \"#FFFF00\">");
			window_text.append("[empty]");
		}
		window_text.append("<br>");
	}
	window_text.append("</c>");

	c->SendPopupToClient(window_title.c_str(), window_text.c_str());
}

void bot_subcommand_pet_remove(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_pet_remove", sep->arg[0], "petremove"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	uint16 class_mask = (PLAYER_CLASS_DRUID_BIT | PLAYER_CLASS_NECROMANCER_BIT | PLAYER_CLASS_ENCHANTER_BIT);
	ActionableBots::Filter_ByClasses(c, sbl, class_mask);
	if (sbl.empty()) {
		c->Message(m_fail, "You have no spawned bots capable of charming");
		return;
	}
	sbl.remove(nullptr);

	int charmed_pet = 0;
	int summoned_pet = 0;
	for (auto bot_iter : sbl) { // Probably needs some work to release charmed pets
		if (bot_iter->IsBotCharmer()) {
			bot_iter->SetBotCharmer(false);
			if (sbl.size() == 1)
				Bot::BotGroupSay(bot_iter, "Using a summoned pet");
			++summoned_pet;
			continue;
		}

		if (bot_iter->GetPet()) {
			bot_iter->GetPet()->Say_StringID(PET_GETLOST_STRING);
			bot_iter->GetPet()->Depop(false);
			bot_iter->SetPetID(0);
		}
		bot_iter->SetBotCharmer(true);
		if (sbl.size() == 1)
			Bot::BotGroupSay(bot_iter, "Available for Charming");
		++charmed_pet;
	}

	if (sbl.size() != 1)
		c->Message(m_action, "%i of your bots set for charming, %i of your bots set for summoned pet use", charmed_pet, summoned_pet);
}

void bot_subcommand_pet_set_type(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_subcommand_pet_set_type", sep->arg[0], "petsettype"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [type: water | fire | air | earth | monster] ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		c->Message(m_note, "requires one of the following bot classes:");
		c->Message(m_message, "Magician(1)");
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName); // this can be expanded without code modification

	std::string pet_arg = sep->arg[1];

	uint8 pet_type = 255;
	uint8 level_req = 255;
	if (!pet_arg.compare("water")) {
		pet_type = 0;
		level_req = 1;
	}
	else if (!pet_arg.compare("fire")) {
		pet_type = 1;
		level_req = 3;
	}
	else if (!pet_arg.compare("air")) {
		pet_type = 2;
		level_req = 4;
	}
	else if (!pet_arg.compare("earth")) {
		pet_type = 3;
		level_req = 5;
	}
	else if (!pet_arg.compare("monster")) {
		pet_type = 4;
		level_req = 30;
	}

	if (pet_type == 255) {
		c->Message(m_fail, "You must specify a pet [type: water | fire | air | earth | monster]");
		return;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]) == ActionableBots::ABT_None)
		return;

	uint16 class_mask = PLAYER_CLASS_MAGICIAN_BIT;
	ActionableBots::Filter_ByClasses(c, sbl, class_mask);
	if (sbl.empty()) {
		c->Message(m_fail, "You have no spawned Magician bots");
		return;
	}

	ActionableBots::Filter_ByMinLevel(c, sbl, level_req);
	if (sbl.empty()) {
		c->Message(m_fail, "You have no spawned Magician bots capable of using this pet type: '%s'", pet_arg.c_str());
		return;
	}
	
	uint16 reclaim_energy_id = 331;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		bot_iter->SetPetChooser(true);
		bot_iter->SetPetChooserID(pet_type);
		if (bot_iter->GetPet()) {
			auto pet_id = bot_iter->GetPetID();
			bot_iter->SetPetID(0);
			bot_iter->CastSpell(reclaim_energy_id, pet_id);
		}
	}
}

void bot_subcommand_portal(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Depart];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Depart) || helper_command_alias_fail(c, "bot_subcommand_portal", sep->arg[0], "portal"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(m_usage, "usage: %s [list | destination] ([option: single])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Depart, WIZARD);
		return;
	}

	bool single = false;
	std::string single_arg = sep->arg[2];
	if (!single_arg.compare("single"))
		single = true;

	std::string destination = sep->arg[1];
	if (!destination.compare("list")) {
		auto my_wizard_bot = ActionableBots::AsGroupMember_ByClass(c, c, WIZARD);
		helper_command_depart_list(c, nullptr, my_wizard_bot, local_list, single);
		return;
	}
	else if (destination.empty()) {
		c->Message(m_fail, "A [destination] or [list] argument is required to use this command");
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToDepart();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->caster_class != WIZARD)
			continue;
		if (local_entry->single != single)
			continue;
		if (destination.compare(spells[local_entry->spell_id].teleport_zone))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}
	
	helper_no_available_bots(c, my_bot);
}


/*
 * bot command helpers go below here
 */
bool helper_bot_appearance_fail(Client *bot_owner, Bot *my_bot, BCEnum::AFType fail_type, const char* type_desc)
{
	switch (fail_type) {
	case BCEnum::AFT_Value:
		bot_owner->Message(m_fail, "Failed to change '%s' for %s due to invalid value for this command", type_desc, my_bot->GetCleanName());
		return true;
	case BCEnum::AFT_GenderRace:
		bot_owner->Message(m_fail, "Failed to change '%s' for %s due to invalid bot gender and/or race for this command", type_desc, my_bot->GetCleanName());
		return true;
	case BCEnum::AFT_Race:
		bot_owner->Message(m_fail, "Failed to change '%s' for %s due to invalid bot race for this command", type_desc, my_bot->GetCleanName());
		return true;
	default:
		return false;
	}
}

void helper_bot_appearance_form_final(Client *bot_owner, Bot *my_bot)
{
	if (!MyBots::IsMyBot(bot_owner, my_bot))
		return;
	if (!my_bot->Save()) {
		bot_owner->Message(m_unknown, "Failed to save appearance change for %s due to unknown cause...", my_bot->GetCleanName());
		return;
	}

	helper_bot_appearance_form_update(my_bot);
	bot_owner->Message(m_action, "Successfully changed appearance for %s!", my_bot->GetCleanName());
}

void helper_bot_appearance_form_update(Bot *my_bot)
{
	if (!my_bot)
		return;

	my_bot->SendIllusionPacket(
		my_bot->GetRace(),
		my_bot->GetGender(),
		0xFF,	//my_bot->GetTexture(),		// 0xFF - change back if issues arise
		0xFF,	//my_bot->GetHelmTexture(),	// 0xFF - change back if issues arise
		my_bot->GetHairColor(),
		my_bot->GetBeardColor(),
		my_bot->GetEyeColor1(),
		my_bot->GetEyeColor2(),
		my_bot->GetHairStyle(),
		my_bot->GetLuclinFace(),
		my_bot->GetBeard(),
		0xFF,					// aa_title (0xFF)
		my_bot->GetDrakkinHeritage(),
		my_bot->GetDrakkinTattoo(),
		my_bot->GetDrakkinDetails(),
		my_bot->GetSize()
	);
}

uint32 helper_bot_create(Client *bot_owner, std::string bot_name, uint8 bot_class, uint16 bot_race, uint8 bot_gender)
{
	uint32 bot_id = 0;
	if (!bot_owner)
		return bot_id;
	if (!Bot::IsValidName(bot_name)) {
		bot_owner->Message(m_fail, "'%s' is an invalid name. You may only use characters 'A-Z', 'a-z' and '_'", bot_name.c_str());
		return bot_id;
	}

	bool available_flag = false;
	if (!botdb.QueryNameAvailablity(bot_name, available_flag)) {
		bot_owner->Message(m_fail, "%s for '%s'", BotDatabase::fail::QueryNameAvailablity(), bot_name.c_str());
		return bot_id;
	}
	if (!available_flag) {
		bot_owner->Message(m_fail, "The name %s is already being used. Please choose a different name", bot_name.c_str());
		return bot_id;
	}

	if (!Bot::IsValidRaceClassCombo(bot_race, bot_class)) {
		bot_owner->Message(m_fail, "'%s'(%u):'%s'(%u) is an invalid race-class combination",
			Bot::RaceIdToString(bot_race).c_str(), bot_race, Bot::ClassIdToString(bot_class).c_str(), bot_class);
		return bot_id;
	}

	if (bot_gender > FEMALE) {
		bot_owner->Message(m_fail, "gender: %u(M), %u(F)", MALE, FEMALE);
		return bot_id;
	}

	uint32 max_bot_count = RuleI(Bots, CreationLimit);

	uint32 bot_count = 0;
	if (!botdb.QueryBotCount(bot_owner->CharacterID(), bot_count)) {
		bot_owner->Message(m_fail, "%s", BotDatabase::fail::QueryBotCount());
		return bot_id;
	}
	if (bot_count >= max_bot_count) {
		bot_owner->Message(m_fail, "You have reached the maximum limit of %i bots", max_bot_count);
		return bot_id;
	}

	auto DefaultNPCTypeStruct = Bot::CreateDefaultNPCTypeStructForBot(
		bot_name.c_str(),
		"",
		bot_owner->GetLevel(),
		bot_race,
		bot_class,
		bot_gender
	);

	auto my_bot = new Bot(DefaultNPCTypeStruct, bot_owner);

	if (!my_bot->Save()) {
		bot_owner->Message(m_unknown, "Failed to create '%s' due to unknown cause", my_bot->GetCleanName());
		return bot_id;
	}

	bot_owner->Message(m_action, "Successfully created '%s' (id: %u)", my_bot->GetCleanName(), my_bot->GetBotID());

	bot_id = my_bot->GetBotID();
	safe_delete(my_bot);

	return bot_id;
}

void helper_bot_out_of_combat(Client *bot_owner, Bot *my_bot)
{
	if (!bot_owner || !my_bot)
		return;

	switch (my_bot->GetClass()) {
	case WARRIOR:
	case CLERIC:
	case PALADIN:
	case RANGER:
	case SHADOWKNIGHT:
	case DRUID:
	case MONK:
		bot_owner->Message(m_unknown, "%s has no out-of-combat behavior defined", my_bot->GetCleanName());
		break;
	case BARD:
		bot_owner->Message(m_action, "%s will %s use out-of-combat behavior for bard songs", my_bot->GetCleanName(), ((my_bot->GetAltOutOfCombatBehavior()) ? ("now") : ("no longer")));
		break;
	case ROGUE:
	case SHAMAN:
	case NECROMANCER:
	case WIZARD:
	case MAGICIAN:
	case ENCHANTER:
	case BEASTLORD:
	case BERSERKER:
		bot_owner->Message(m_unknown, "%s has no out-of-combat behavior defined", my_bot->GetCleanName());
		break;
	default:
		break;
		bot_owner->Message(m_fail, "Undefined bot class for %s", my_bot->GetCleanName());
	}
}

bool helper_cast_standard_spell(Bot* casting_bot, Mob* target_mob, int spell_id, bool annouce_cast, uint32* dont_root_before)
{
	if (!casting_bot || !target_mob)
		return false;

	casting_bot->InterruptSpell();
	if (annouce_cast)
		Bot::BotGroupSay(casting_bot, "Attempting to cast '%s' on %s", spells[spell_id].name, target_mob->GetCleanName());

	return casting_bot->CastSpell(spell_id, target_mob->GetID(), 1, -1, -1, dont_root_before);
}

bool helper_command_alias_fail(Client *bot_owner, const char* command_handler, const char *alias, const char *command)
{
	auto alias_iter = bot_command_aliases.find(&alias[1]);
	if (alias_iter == bot_command_aliases.end() || alias_iter->second.compare(command)) {
		bot_owner->Message(m_fail, "Undefined linker usage in %s (%s)", command_handler, &alias[1]);
		return true;
	}

	return false;
}

void helper_command_depart_list(Client* bot_owner, Bot* druid_bot, Bot* wizard_bot, bcst_list* local_list, bool single_flag)
{
	if (!bot_owner)
		return;

	if (!MyBots::IsMyBot(bot_owner, druid_bot))
		druid_bot = nullptr;
	if (!MyBots::IsMyBot(bot_owner, wizard_bot))
		wizard_bot = nullptr;
	if (!druid_bot && !wizard_bot) {
		bot_owner->Message(m_fail, "No bots are capable of performing this action");
		return;
	}

	bot_owner->Message(m_message, "The following destinations are available:");
	if (!local_list) {
		bot_owner->Message(m_fail, "None");
		return;
	}

	std::string msg;
	std::string text_link;

	int destinations = 0;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToDepart();
		if (!local_entry)
			continue;

		if (druid_bot && druid_bot->GetClass() == local_entry->caster_class && druid_bot->GetLevel() >= local_entry->spell_level) {
			if (local_entry->single != single_flag)
				continue;
			msg = StringFormat("%ccircle %s%s", BOT_COMMAND_CHAR, spells[local_entry->spell_id].teleport_zone, ((single_flag) ? (" single") : ("")));
			text_link = druid_bot->CreateSayLink(bot_owner, msg.c_str(), local_entry->long_name.c_str());
			Bot::BotGroupSay(druid_bot, "dest: '%s' click: %s", spells[local_entry->spell_id].teleport_zone, text_link.c_str());
			++destinations;
			continue;
		}
		if (wizard_bot && wizard_bot->GetClass() == local_entry->caster_class && wizard_bot->GetLevel() >= local_entry->spell_level) {
			if (local_entry->single != single_flag)
				continue;
			msg = StringFormat("%cportal %s%s", BOT_COMMAND_CHAR, spells[local_entry->spell_id].teleport_zone, ((single_flag) ? (" single") : ("")));
			text_link = wizard_bot->CreateSayLink(bot_owner, msg.c_str(), local_entry->long_name.c_str());
			Bot::BotGroupSay(wizard_bot, "dest: '%s' click: %s", spells[local_entry->spell_id].teleport_zone, text_link.c_str());
			++destinations;
			continue;
		}
	}
	if (!destinations)
		bot_owner->Message(m_fail, "None");
}

bool helper_is_help_or_usage(const char* arg)
{
	if (!arg)
		return false;
	if (strcasecmp(arg, "help") && strcasecmp(arg, "usage"))
		return false;

	return true;
}

bool helper_no_available_bots(Client *bot_owner, Bot *my_bot)
{
	if (!bot_owner)
		return true;
	if (!my_bot) {
		bot_owner->Message(m_fail, "No bots are capable of performing this action");
		return true;
	}

	return false;
}

void helper_send_available_subcommands(Client *bot_owner, const char* command_simile, const std::list<const char*>& subcommand_list)
{
	bot_owner->Message(m_message, "Available %s management subcommands:", command_simile);

	int bot_subcommands_shown = 0;
	for (const auto subcommand_iter : subcommand_list) {
		auto find_iter = bot_command_list.find(subcommand_iter);
		if (find_iter == bot_command_list.end())
			continue;
		if (bot_owner->Admin() < find_iter->second->access)
			continue;

		bot_owner->Message(m_usage, "%c%s - %s", BOT_COMMAND_CHAR, subcommand_iter, ((find_iter != bot_command_list.end()) ? (find_iter->second->desc) : ("[no description]")));
		++bot_subcommands_shown;
	}

	bot_owner->Message(m_message, "%d bot subcommand%s listed.", bot_subcommands_shown, bot_subcommands_shown != 1 ? "s" : "");
}

void helper_send_usage_required_bots(Client *bot_owner, BCEnum::SpType spell_type, uint8 bot_class)
{
	bot_owner->Message(m_note, "requires one of the following bot classes:");
	if (bot_class)
		bot_owner->Message(m_message, "%s", required_bots_map_by_class[spell_type][bot_class].c_str());
	else
		bot_owner->Message(m_message, "%s", required_bots_map[spell_type].c_str());
}

bool helper_spell_check_fail(STBaseEntry* local_entry)
{
	if (!local_entry)
		return true;
	if (spells[local_entry->spell_id].zonetype && zone->GetZoneType() && !(spells[local_entry->spell_id].zonetype & zone->GetZoneType()))
		return true;

	return false;
}

bool helper_spell_list_fail(Client *bot_owner, bcst_list* spell_list, BCEnum::SpType spell_type)
{
	if (!spell_list || spell_list->empty()) {
		bot_owner->Message(m_fail, "%s", required_bots_map[spell_type].c_str());
		return true;
	}

	return false;
}

#endif // BOTS
