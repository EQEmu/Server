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

#include <string.h>

#ifdef _WINDOWS
#define strcasecmp _stricmp
#endif

#include "../common/data_verification.h"
#include "../common/global_define.h"
#include "../common/eq_packet.h"
#include "../common/features.h"
#include "../common/ptimer.h"
#include "../common/rulesys.h"
#include "../common/serverinfo.h"
#include "../common/strings.h"
#include "../common/say_link.h"

#include "bot_command.h"
#include "zonedb.h"
#include "qglobals.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "titles.h"
#include "water_map.h"
#include "worldserver.h"
#include "mob.h"

#include <fmt/format.h>

extern QueryServ* QServ;
extern WorldServer worldserver;
extern TaskManager *task_manager;

bcst_map                               bot_command_spells;
bcst_required_bot_classes_map          required_bots_map;
bcst_required_bot_classes_map_by_class required_bots_map_by_class;

class BCSpells
{
public:
	static void Load() {
		bot_command_spells.clear();
		bcst_levels_map bot_levels_map;

		for (int i = BCEnum::SpellTypeFirst; i <= BCEnum::SpellTypeLast; ++i) {
			bot_command_spells[static_cast<BCEnum::SpType>(i)];
			bot_levels_map[static_cast<BCEnum::SpType>(i)];
		}

		for (int spell_id = 2; spell_id < SPDAT_RECORDS; ++spell_id) {
			if (!IsValidSpell(spell_id)) {
				continue;
			}

			if (spells[spell_id].player_1[0] == '\0') {
				continue;
			}

			if (
				spells[spell_id].target_type != ST_Target &&
				spells[spell_id].cast_restriction != 0
				) {
				continue;
			}

			auto target_type = BCEnum::TT_None;
			switch (spells[spell_id].target_type) {
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
					switch (spells[spell_id].cast_restriction) {
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

			uint8 class_levels[16] = {0};
			bool player_spell = false;
			for (int class_type = Class::Warrior; class_type <= Class::Berserker; ++class_type) {
				int class_index = CLASSIDTOINDEX(class_type);
				if (spells[spell_id].classes[class_index] == 0 ||
					spells[spell_id].classes[class_index] > HARD_LEVEL_CAP) {
					continue;
					}

				class_levels[class_index] = spells[spell_id].classes[class_index];
				player_spell = true;
			}
			if (!player_spell)
				continue;

			STBaseEntry* entry_prototype = nullptr;
			while (true) {
				switch (spells[spell_id].effect_id[EFFECTIDTOINDEX(1)]) {
					case SE_BindAffinity:
						entry_prototype = new STBaseEntry(BCEnum::SpT_BindAffinity);
						break;
					case SE_Charm:
						if (spells[spell_id].spell_affect_index != 12)
							break;
						entry_prototype = new STCharmEntry();
						if (spells[spell_id].resist_difficulty <= -1000)
							entry_prototype->SafeCastToCharm()->dire = true;
						break;
					case SE_Teleport:
						entry_prototype = new STDepartEntry;
						entry_prototype->SafeCastToDepart()->single = !BCSpells::IsGroupType(target_type);
						break;
					case SE_Succor:
						if (!strcmp(spells[spell_id].teleport_zone, "same")) {
							entry_prototype = new STEscapeEntry;
						} else {
							entry_prototype = new STDepartEntry;
							entry_prototype->SafeCastToDepart()->single = !BCSpells::IsGroupType(target_type);
						}
						break;
					case SE_Translocate:
						if (spells[spell_id].teleport_zone[0] == '\0') {
							entry_prototype = new STSendHomeEntry();
							entry_prototype->SafeCastToSendHome()->group = BCSpells::IsGroupType(target_type);
						} else {
							entry_prototype = new STDepartEntry;
							entry_prototype->SafeCastToDepart()->single = !BCSpells::IsGroupType(target_type);
						}
						break;
					case SE_ModelSize:
						if (spells[spell_id].base_value[EFFECTIDTOINDEX(1)] > 100) {
							entry_prototype = new STSizeEntry;
							entry_prototype->SafeCastToSize()->size_type = BCEnum::SzT_Enlarge;
						} else if (spells[spell_id].base_value[EFFECTIDTOINDEX(1)] > 0 &&
								   spells[spell_id].base_value[EFFECTIDTOINDEX(1)] < 100) {
							entry_prototype = new STSizeEntry;
							entry_prototype->SafeCastToSize()->size_type = BCEnum::SzT_Reduce;
						}
						break;
					case SE_Identify:
						entry_prototype = new STBaseEntry(BCEnum::SpT_Identify);
						break;
					case SE_Invisibility:
						if (spells[spell_id].spell_affect_index != 9)
							break;
						entry_prototype = new STInvisibilityEntry;
						entry_prototype->SafeCastToInvisibility()->invis_type = BCEnum::IT_Living;
						break;
					case SE_SeeInvis:
						if (spells[spell_id].spell_affect_index != 5)
							break;
						entry_prototype = new STInvisibilityEntry;
						entry_prototype->SafeCastToInvisibility()->invis_type = BCEnum::IT_See;
						break;
					case SE_InvisVsUndead:
						if (spells[spell_id].spell_affect_index != 9)
							break;
						entry_prototype = new STInvisibilityEntry;
						entry_prototype->SafeCastToInvisibility()->invis_type = BCEnum::IT_Undead;
						break;
					case SE_InvisVsAnimals:
						if (spells[spell_id].spell_affect_index != 9)
							break;
						entry_prototype = new STInvisibilityEntry;
						entry_prototype->SafeCastToInvisibility()->invis_type = BCEnum::IT_Animal;
						break;
					case SE_Mez:
						if (spells[spell_id].effect_id[EFFECTIDTOINDEX(1)] != 31)
							break;
						entry_prototype = new STBaseEntry(BCEnum::SpT_Mesmerize);
						break;
					case SE_Revive:
						if (spells[spell_id].spell_affect_index != 1)
							break;
						entry_prototype = new STResurrectEntry();
						entry_prototype->SafeCastToResurrect()->aoe = BCSpells::IsCasterCentered(target_type);
						break;
					case SE_Rune:
						if (spells[spell_id].spell_affect_index != 2)
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

				switch (spells[spell_id].effect_id[EFFECTIDTOINDEX(2)]) {
					case SE_Succor:
						entry_prototype = new STEscapeEntry;
						std::string is_lesser = spells[spell_id].name;
						if (is_lesser.find("Lesser") != std::string::npos)
							entry_prototype->SafeCastToEscape()->lesser = true;
						break;
				}
				if (entry_prototype)
					break;

				switch (spells[spell_id].effect_id[EFFECTIDTOINDEX(3)]) {
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

				while (spells[spell_id].type_description_id == 27) {
					if (!spells[spell_id].good_effect)
						break;
					if (spells[spell_id].skill != EQ::skills::SkillOffense &&
						spells[spell_id].skill != EQ::skills::SkillDefense)
						break;

					entry_prototype = new STStanceEntry();
					if (spells[spell_id].skill == EQ::skills::SkillOffense)
						entry_prototype->SafeCastToStance()->stance_type = BCEnum::StT_Aggressive;
					else
						entry_prototype->SafeCastToStance()->stance_type = BCEnum::StT_Defensive;

					break;
				}
				if (entry_prototype)
					break;

				switch (spells[spell_id].spell_affect_index) {
					case 1: {
						bool valid_spell = false;
						entry_prototype = new STCureEntry;

						for (int i = EffectIDFirst; i <= EffectIDLast; ++i) {
							int effect_index = EFFECTIDTOINDEX(i);
							if (spells[spell_id].effect_id[effect_index] != SE_Blind &&
								spells[spell_id].base_value[effect_index] >= 0)
								continue;
							else if (spells[spell_id].effect_id[effect_index] == SE_Blind &&
									 !spells[spell_id].good_effect)
								continue;

							switch (spells[spell_id].effect_id[effect_index]) {
								case SE_Blind:
									entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(
										BCEnum::AT_Blindness)] += spells[spell_id].base_value[effect_index];
									break;
								case SE_DiseaseCounter:
									entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(
										BCEnum::AT_Disease)] += spells[spell_id].base_value[effect_index];
									break;
								case SE_PoisonCounter:
									entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(
										BCEnum::AT_Poison)] += spells[spell_id].base_value[effect_index];
									break;
								case SE_CurseCounter:
									entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(
										BCEnum::AT_Curse)] += spells[spell_id].base_value[effect_index];
									break;
								case SE_CorruptionCounter:
									entry_prototype->SafeCastToCure()->cure_value[AILMENTIDTOINDEX(
										BCEnum::AT_Corruption)] += spells[spell_id].base_value[effect_index];
									break;
								default:
									continue;
							}
							entry_prototype->SafeCastToCure()->cure_total += spells[spell_id].base_value[effect_index];
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
							if (spells[spell_id].max_value[effect_index] <= 0)
								continue;

							switch (spells[spell_id].effect_id[effect_index]) {
								case SE_ResistFire:
									entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(
										BCEnum::RT_Fire)] += spells[spell_id].max_value[effect_index];
									break;
								case SE_ResistCold:
									entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(
										BCEnum::RT_Cold)] += spells[spell_id].max_value[effect_index];
									break;
								case SE_ResistPoison:
									entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(
										BCEnum::RT_Poison)] += spells[spell_id].max_value[effect_index];
									break;
								case SE_ResistDisease:
									entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(
										BCEnum::RT_Disease)] += spells[spell_id].max_value[effect_index];
									break;
								case SE_ResistMagic:
									entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(
										BCEnum::RT_Magic)] += spells[spell_id].max_value[effect_index];
									break;
								case SE_ResistCorruption:
									entry_prototype->SafeCastToResistance()->resist_value[RESISTANCEIDTOINDEX(
										BCEnum::RT_Corruption)] += spells[spell_id].max_value[effect_index];
									break;
								default:
									continue;
							}
							entry_prototype->SafeCastToResistance()->resist_total += spells[spell_id].max_value[effect_index];
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
						if (spells[spell_id].effect_description_id != 65)
							break;
						if (IsEffectInSpell(spell_id, SE_NegateIfCombat))
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

			if (target_type == BCEnum::TT_Self && (entry_prototype->BCST() != BCEnum::SpT_Stance &&
												   entry_prototype->BCST() != BCEnum::SpT_SummonCorpse)) {
#ifdef BCSTSPELLDUMP
				LogError("DELETING entry_prototype (primary clause) - name: [{}], target_type: [{}], BCST: [{}]",
					spells[spell_id].name, BCEnum::TargetTypeEnumToString(target_type).c_str(), BCEnum::SpellTypeEnumToString(entry_prototype->BCST()).c_str());
#endif
				safe_delete(entry_prototype);
				continue;
			}
			if (entry_prototype->BCST() == BCEnum::SpT_Stance && target_type != BCEnum::TT_Self) {
#ifdef BCSTSPELLDUMP
				LogError("DELETING entry_prototype (secondary clause) - name: [{}], BCST: [{}], target_type: [{}]",
					spells[spell_id].name, BCEnum::SpellTypeEnumToString(entry_prototype->BCST()).c_str(), BCEnum::TargetTypeEnumToString(target_type).c_str());
#endif
				safe_delete(entry_prototype);
				continue;
			}

			assert(entry_prototype->BCST() != BCEnum::SpT_None);

			entry_prototype->spell_id = spell_id;
			entry_prototype->target_type = target_type;

			bcst_levels& bot_levels = bot_levels_map[entry_prototype->BCST()];
			for (int class_type = Class::Warrior; class_type <= Class::Berserker; ++class_type) {
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

				if (bot_levels.find(class_type) == bot_levels.end() ||
					bot_levels[class_type] > class_levels[class_index])
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
			for (auto list_iter: map_iter.second) {
			safe_delete(list_iter);
			}
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

			for (auto del_iter: *removed_spells_list)
			{
				safe_delete(del_iter);
			}
			removed_spells_list->clear();

			if (RuleI(Bots, CommandSpellRank) == 1) {
				spells_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (spells[l->spell_id].spell_group < spells[r->spell_id].spell_group)
						return true;
					if (spells[l->spell_id].spell_group == spells[r->spell_id].spell_group && l->caster_class < r->caster_class)
						return true;
					if (spells[l->spell_id].spell_group == spells[r->spell_id].spell_group && l->caster_class == r->caster_class && spells[l->spell_id].rank < spells[r->spell_id].rank)
						return true;

					return false;
				});
				spells_list->unique([removed_spells_list](STBaseEntry* l, STBaseEntry* r) {
					std::string r_name = spells[r->spell_id].name;
					if (spells[l->spell_id].spell_group == spells[r->spell_id].spell_group && l->caster_class == r->caster_class && spells[l->spell_id].rank < spells[r->spell_id].rank) {
						removed_spells_list->push_back(r);
						return true;
					}

					return false;
				});

				for (auto del_iter: *removed_spells_list) {
					safe_delete(del_iter);
				}
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

				for (auto del_iter: *removed_spells_list) {
					safe_delete(del_iter);
				}
				removed_spells_list->clear();
			}

			// needs rework
			if (RuleI(Bots, CommandSpellRank) == 2 || RuleI(Bots, CommandSpellRank) == 3) {
				spells_list->sort([](STBaseEntry* l, STBaseEntry* r) {
					if (spells[l->spell_id].spell_group < spells[r->spell_id].spell_group)
						return true;
					if (spells[l->spell_id].spell_group == spells[r->spell_id].spell_group && l->caster_class < r->caster_class)
						return true;
					if (spells[l->spell_id].spell_group == spells[r->spell_id].spell_group && l->caster_class == r->caster_class && spells[l->spell_id].rank > spells[r->spell_id].rank)
						return true;

					return false;
				});
				spells_list->unique([removed_spells_list](STBaseEntry* l, STBaseEntry* r) {
					std::string l_name = spells[l->spell_id].name;
					if (spells[l->spell_id].spell_group == spells[r->spell_id].spell_group && l->caster_class == r->caster_class && spells[l->spell_id].rank > spells[r->spell_id].rank) {
						removed_spells_list->push_back(r);
						return true;
					}

					return false;
				});

				for (auto del_iter: *removed_spells_list) {
					safe_delete(del_iter);
				}
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
					if (LT_SPELLS(l, r, resist_difficulty))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, max_value, 1))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max_value, 1) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max_value, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
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
					if (EQ_STBASE(l, r, target_type) && LT_SPELLS(l, r, zone_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS(l, r, zone_type) && GT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS(l, r, zone_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Lull:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_SPELLS(l, r, resist_difficulty))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, max_value, 3))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max_value, 3) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max_value, 3) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Mesmerize:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (GT_SPELLS(l, r, resist_difficulty))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, max_value, 1))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max_value, 1) && GT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS(l, r, resist_difficulty) && EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max_value, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_MovementSpeed:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, base_value, 2))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base_value, 2) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base_value, 2) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
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
					if (GT_SPELLS_EFFECT_ID(l, r, base_value, 1))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && LT_STBASE(l, r, target_type))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && EQ_STBASE(l, r, target_type) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && EQ_STBASE(l, r, target_type) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
						return true;

					return false;
				});
				continue;
			case BCEnum::SpT_Rune:
				spell_list->sort([](const STBaseEntry* l, const STBaseEntry* r) {
					if (LT_STBASE(l, r, target_type))
						return true;
					if (EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, max_value, 1))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max_value, 1) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, max_value, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
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
						if (EQ_STBASE(l, r, target_type) && GT_SPELLS_EFFECT_ID(l, r, base_value, 1))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && GT_STBASE(l, r, spell_level))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
							return true;
					}
					if (l_size_type == BCEnum::SzT_Reduce && r_size_type == BCEnum::SzT_Reduce) {
						if (EQ_STBASE(l, r, target_type) && LT_SPELLS_EFFECT_ID(l, r, base_value, 1))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && GT_STBASE(l, r, spell_level))
							return true;
						if (EQ_STBASE(l, r, target_type) && EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && EQ_STBASE(l, r, spell_level) && LT_STBASE(l, r, caster_class))
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
					if (GT_SPELLS_EFFECT_ID(l, r, base_value, 1))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && LT_STBASE(l, r, spell_level))
						return true;
					if (EQ_SPELLS_EFFECT_ID(l, r, base_value, 1) && EQ_STBASE(l, r, spell_level) && EQ_STBASE(l, r, caster_class))
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
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			LogError("load_teleport_zone_names() - Error in zone names query: [{}]", results.ErrorMessage().c_str());
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
		LogCommands("load_bot_command_spells(): - 'RuleI(Bots, CommandSpellRank)' set to [{}]", RuleI(Bots, CommandSpellRank));
		if (bot_command_spells.empty()) {
			LogError("load_bot_command_spells() - 'bot_command_spells' is empty");
			return;
		}

		for (int i = BCEnum::SpellTypeFirst; i <= BCEnum::SpellTypeLast; ++i)
			LogCommands("load_bot_command_spells(): - [{}] returned [{}] spell entries",
				BCEnum::SpellTypeEnumToString(static_cast<BCEnum::SpType>(i)).c_str(), bot_command_spells[static_cast<BCEnum::SpType>(i)].size());
	}

	static void helper_bots_string(BCEnum::SpType type_index, bcst_levels& bot_levels) {
		for (int i = Class::Warrior; i <= Class::Berserker; ++i)
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
				if (strcasecmp(GetClassIDName(levels_iter->first), GetClassIDName(test_iter->first)) < 0 && levels_iter->second <= test_iter->second)
					test_iter = levels_iter;
			}

			std::string bot_segment;
			if (bot_levels.size() == map_size)
				bot_segment = "%s(%u)";
			else if (bot_levels.size() > 1)
				bot_segment = ", %s(%u)";
			else
				bot_segment = " or %s(%u)";

			required_bots_map[type_index].append(StringFormat(bot_segment.c_str(), GetClassIDName(test_iter->first), test_iter->second));
			required_bots_map_by_class[type_index][test_iter->first] = StringFormat("%s(%u)", GetClassIDName(test_iter->first), test_iter->second);
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
					spells[spell_id].resist_difficulty,
					spells[spell_id].zone_type,
					spells[spell_id].description_id,
					spells[spell_id].type_description_id,
					spells[spell_id].effect_description_id,
					spells[spell_id].spell_affect_index
				);

				for (int i = EffectIDFirst; i <= 3/*EffectIDLast*/; ++i) {
					int effect_index = EFFECTIDTOINDEX(i);
					spell_dump << StringFormat(" /e%02i:%04i/b%02i:%06i/m%02i:%06i",
						i, spells[spell_id].effect_id[effect_index], i, spells[spell_id].base_value[effect_index], i, spells[spell_id].max_value[effect_index]);
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

int bot_command_count;

int (*bot_command_dispatch)(Client *,char const *) = bot_command_not_avail;

std::map<std::string, BotCommandRecord *> bot_command_list;
std::map<std::string, std::string> bot_command_aliases;

LinkedList<BotCommandRecord *> cleanup_bot_command_list;

int bot_command_not_avail(Client *c, const char *message)
{
	c->Message(Chat::White, "Bot commands not available.");
	return -1;
}

int bot_command_init(void)
{
	bot_command_aliases.clear();

	if (
		bot_command_add("actionable", "Lists actionable command arguments and use descriptions", AccountStatus::Player, bot_command_actionable) ||
		bot_command_add("aggressive", "Orders a bot to use a aggressive discipline", AccountStatus::Player, bot_command_aggressive) ||
		bot_command_add("applypoison", "Applies cursor-held poison to a rogue bot's weapon", AccountStatus::Player, bot_command_apply_poison) ||
		bot_command_add("applypotion", "Applies cursor-held potion to a bot's effects", AccountStatus::Player, bot_command_apply_potion) ||
		bot_command_add("attack", "Orders bots to attack a designated target", AccountStatus::Player, bot_command_attack) ||
		bot_command_add("bindaffinity", "Orders a bot to attempt an affinity binding", AccountStatus::Player, bot_command_bind_affinity) ||
		bot_command_add("bot", "Lists the available bot management [subcommands]", AccountStatus::Player, bot_command_bot) ||
		bot_command_add("botappearance", "Lists the available bot appearance [subcommands]", AccountStatus::Player, bot_command_appearance) ||
		bot_command_add("botbeardcolor", "Changes the beard color of a bot", AccountStatus::Player, bot_command_beard_color) ||
		bot_command_add("botbeardstyle", "Changes the beard style of a bot", AccountStatus::Player, bot_command_beard_style) ||
		bot_command_add("botcamp", "Orders a bot(s) to camp", AccountStatus::Player, bot_command_camp) ||
		bot_command_add("botclone", "Creates a copy of a bot", AccountStatus::GMMgmt, bot_command_clone) ||
		bot_command_add("botcreate", "Creates a new bot", AccountStatus::Player, bot_command_create) ||
		bot_command_add("botdelete", "Deletes all record of a bot", AccountStatus::Player, bot_command_delete) ||
		bot_command_add("botdetails", "Changes the Drakkin details of a bot", AccountStatus::Player, bot_command_details) ||
		bot_command_add("botdyearmor", "Changes the color of a bot's (bots') armor", AccountStatus::Player, bot_command_dye_armor) ||
		bot_command_add("boteyes", "Changes the eye colors of a bot", AccountStatus::Player, bot_command_eyes) ||
		bot_command_add("botface", "Changes the facial appearance of your bot", AccountStatus::Player, bot_command_face) ||
		bot_command_add("botfollowdistance", "Changes the follow distance(s) of a bot(s)", AccountStatus::Player, bot_command_follow_distance) ||
		bot_command_add("bothaircolor", "Changes the hair color of a bot", AccountStatus::Player, bot_command_hair_color) ||
		bot_command_add("bothairstyle", "Changes the hairstyle of a bot", AccountStatus::Player, bot_command_hairstyle) ||
		bot_command_add("botheritage", "Changes the Drakkin heritage of a bot", AccountStatus::Player, bot_command_heritage) ||
		bot_command_add("botinspectmessage", "Changes the inspect message of a bot", AccountStatus::Player, bot_command_inspect_message) ||
		bot_command_add("botlist", "Lists the bots that you own", AccountStatus::Player, bot_command_list_bots) ||
		bot_command_add("botoutofcombat", "Toggles your bot between standard and out-of-combat spell/skill use - if any specialized behaviors exist", AccountStatus::Player, bot_command_out_of_combat) ||
		bot_command_add("botreport", "Orders a bot to report its readiness", AccountStatus::Player, bot_command_report) ||
		bot_command_add("botspawn", "Spawns a created bot", AccountStatus::Player, bot_command_spawn) ||
		bot_command_add("botstance", "Changes the stance of a bot", AccountStatus::Player, bot_command_stance) ||
		bot_command_add("botstopmeleelevel", "Sets the level a caster or spell-casting fighter bot will stop melee combat", AccountStatus::Player, bot_command_stop_melee_level) ||
		bot_command_add("botsuffix", "Sets a bots suffix", AccountStatus::Player, bot_command_suffix) ||
		bot_command_add("botsummon", "Summons bot(s) to your location", AccountStatus::Player, bot_command_summon) ||
		bot_command_add("botsurname", "Sets a bots surname (last name)", AccountStatus::Player, bot_command_surname) ||
		bot_command_add("bottattoo", "Changes the Drakkin tattoo of a bot", AccountStatus::Player, bot_command_tattoo) ||
		bot_command_add("bottogglearcher", "Toggles a archer bot between melee and ranged weapon use", AccountStatus::Player, bot_command_toggle_archer) ||
		bot_command_add("bottogglehelm", "Toggles the helm visibility of a bot between shown and hidden", AccountStatus::Player, bot_command_toggle_helm) ||
		bot_command_add("bottitle", "Sets a bots title", AccountStatus::Player, bot_command_title) ||
		bot_command_add("botupdate", "Updates a bot to reflect any level changes that you have experienced", AccountStatus::Player, bot_command_update) ||
		bot_command_add("botwoad", "Changes the Barbarian woad of a bot", AccountStatus::Player, bot_command_woad) ||
		bot_command_add("casterrange", "Controls the range casters will try to stay away from a mob (if too far, they will skip spells that are out-of-range)", AccountStatus::Player, bot_command_caster_range) ||
		bot_command_add("charm", "Attempts to have a bot charm your target", AccountStatus::Player, bot_command_charm) ||
		bot_command_add("circle", "Orders a Druid bot to open a magical doorway to a specified destination", AccountStatus::Player, bot_command_circle) ||
		bot_command_add("clickitem", "Orders your targeted bot to click the item in the provided inventory slot.", AccountStatus::Player, bot_command_click_item) ||
		bot_command_add("cure", "Orders a bot to remove any ailments", AccountStatus::Player, bot_command_cure) ||
		bot_command_add("defensive", "Orders a bot to use a defensive discipline", AccountStatus::Player, bot_command_defensive) ||
		bot_command_add("depart", "Orders a bot to open a magical doorway to a specified destination", AccountStatus::Player, bot_command_depart) ||
		bot_command_add("enforcespellsettings", "Toggles your Bot to cast only spells in their spell settings list.", AccountStatus::Player, bot_command_enforce_spell_list) ||
		bot_command_add("escape", "Orders a bot to send a target group to a safe location within the zone", AccountStatus::Player, bot_command_escape) ||
		bot_command_add("findaliases", "Find available aliases for a bot command", AccountStatus::Player, bot_command_find_aliases) ||
		bot_command_add("follow", "Orders bots to follow a designated target (option 'chain' auto-links eligible spawned bots)", AccountStatus::Player, bot_command_follow) ||
		bot_command_add("guard", "Orders bots to guard their current positions", AccountStatus::Player, bot_command_guard) ||
		bot_command_add("healrotation", "Lists the available bot heal rotation [subcommands]", AccountStatus::Player, bot_command_heal_rotation) ||
		bot_command_add("healrotationadaptivetargeting", "Enables or disables adaptive targeting within the heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_adaptive_targeting) ||
		bot_command_add("healrotationaddmember", "Adds a bot to a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_add_member) ||
		bot_command_add("healrotationaddtarget", "Adds target to a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_add_target) ||
		bot_command_add("healrotationadjustcritical", "Adjusts the critial HP limit of the heal rotation instance's Class Armor Type criteria", AccountStatus::Player, bot_command_heal_rotation_adjust_critical) ||
		bot_command_add("healrotationadjustsafe", "Adjusts the safe HP limit of the heal rotation instance's Class Armor Type criteria", AccountStatus::Player, bot_command_heal_rotation_adjust_safe) ||
		bot_command_add("healrotationcastingoverride", "Enables or disables casting overrides within the heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_casting_override) ||
		bot_command_add("healrotationchangeinterval", "Changes casting interval between members within the heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_change_interval) ||
		bot_command_add("healrotationclearhot", "Clears the HOT of a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_clear_hot) ||
		bot_command_add("healrotationcleartargets", "Removes all targets from a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_clear_targets) ||
		bot_command_add("healrotationcreate", "Creates a bot heal rotation instance and designates a leader", AccountStatus::Player, bot_command_heal_rotation_create) ||
		bot_command_add("healrotationdelete", "Deletes a bot heal rotation entry by leader", AccountStatus::Player, bot_command_heal_rotation_delete) ||
		bot_command_add("healrotationfastheals", "Enables or disables fast heals within the heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_fast_heals) ||
		bot_command_add("healrotationlist", "Reports heal rotation instance(s) information", AccountStatus::Player, bot_command_heal_rotation_list) ||
		bot_command_add("healrotationremovemember", "Removes a bot from a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_remove_member) ||
		bot_command_add("healrotationremovetarget", "Removes target from a heal rotations instance", AccountStatus::Player, bot_command_heal_rotation_remove_target) ||
		bot_command_add("healrotationresetlimits", "Resets all Class Armor Type HP limit criteria in a heal rotation to its default value", AccountStatus::Player, bot_command_heal_rotation_reset_limits) ||
		bot_command_add("healrotationsave", "Saves a bot heal rotation entry by leader", AccountStatus::Player, bot_command_heal_rotation_save) ||
		bot_command_add("healrotationsethot", "Sets the HOT in a heal rotation instance", AccountStatus::Player, bot_command_heal_rotation_set_hot) ||
		bot_command_add("healrotationstart", "Starts a heal rotation", AccountStatus::Player, bot_command_heal_rotation_start) ||
		bot_command_add("healrotationstop", "Stops a heal rotation", AccountStatus::Player, bot_command_heal_rotation_stop) ||
		bot_command_add("help", "List available commands and their description - specify partial command as argument to search", AccountStatus::Player, bot_command_help) ||
		bot_command_add("hold", "Prevents a bot from attacking until released", AccountStatus::Player, bot_command_hold) ||
		bot_command_add("identify", "Orders a bot to cast an item identification spell", AccountStatus::Player, bot_command_identify) ||
		bot_command_add("inventory", "Lists the available bot inventory [subcommands]", AccountStatus::Player, bot_command_inventory) ||
		bot_command_add("inventorygive", "Gives the item on your cursor to a bot", AccountStatus::Player, bot_command_inventory_give) ||
		bot_command_add("inventorylist", "Lists all items in a bot's inventory", AccountStatus::Player, bot_command_inventory_list) ||
		bot_command_add("inventoryremove", "Removes an item from a bot's inventory", AccountStatus::Player, bot_command_inventory_remove) ||
		bot_command_add("inventorywindow", "Displays all items in a bot's inventory in a pop-up window", AccountStatus::Player, bot_command_inventory_window) ||
		bot_command_add("invisibility", "Orders a bot to cast a cloak of invisibility, or allow them to be seen", AccountStatus::Player, bot_command_invisibility) ||
		bot_command_add("itemuse", "Elicits a report from spawned bots that can use the item on your cursor (option 'empty' yields only empty slots)", AccountStatus::Player, bot_command_item_use) ||
		bot_command_add("levitation", "Orders a bot to cast a levitation spell", AccountStatus::Player, bot_command_levitation) ||
		bot_command_add("lull", "Orders a bot to cast a pacification spell", AccountStatus::Player, bot_command_lull) ||
		bot_command_add("mesmerize", "Orders a bot to cast a mesmerization spell", AccountStatus::Player, bot_command_mesmerize) ||
		bot_command_add("movementspeed", "Orders a bot to cast a movement speed enhancement spell", AccountStatus::Player, bot_command_movement_speed) ||
		bot_command_add("owneroption", "Sets options available to bot owners", AccountStatus::Player, bot_command_owner_option) ||
		bot_command_add("pet", "Lists the available bot pet [subcommands]", AccountStatus::Player, bot_command_pet) ||
		bot_command_add("petgetlost", "Orders a bot to remove its summoned pet", AccountStatus::Player, bot_command_pet_get_lost) ||
		bot_command_add("petremove", "Orders a bot to remove its charmed pet", AccountStatus::Player, bot_command_pet_remove) ||
		bot_command_add("petsettype", "Orders a Magician bot to use a specified pet type", AccountStatus::Player, bot_command_pet_set_type) ||
		bot_command_add("picklock", "Orders a capable bot to pick the lock of the closest door", AccountStatus::Player, bot_command_pick_lock) ||
		bot_command_add("pickpocket", "Orders a capable bot to pickpocket a NPC", AccountStatus::Player, bot_command_pickpocket) ||
		bot_command_add("precombat", "Sets flag used to determine pre-combat behavior", AccountStatus::Player, bot_command_precombat) ||
		bot_command_add("portal", "Orders a Wizard bot to open a magical doorway to a specified destination", AccountStatus::Player, bot_command_portal) ||
		bot_command_add("pull", "Orders a designated bot to 'pull' an enemy", AccountStatus::Player, bot_command_pull) ||
		bot_command_add("release", "Releases a suspended bot's AI processing (with hate list wipe)", AccountStatus::Player, bot_command_release) ||
		bot_command_add("resistance", "Orders a bot to cast a specified resistance buff", AccountStatus::Player, bot_command_resistance) ||
		bot_command_add("resurrect", "Orders a bot to resurrect a player's (players') corpse(s)", AccountStatus::Player, bot_command_resurrect) ||
		bot_command_add("rune", "Orders a bot to cast a rune of protection", AccountStatus::Player, bot_command_rune) ||
		bot_command_add("sendhome", "Orders a bot to open a magical doorway home", AccountStatus::Player, bot_command_send_home) ||
		bot_command_add("size", "Orders a bot to change a player's size", AccountStatus::Player, bot_command_size) ||
		bot_command_add("spellinfo", "Opens a dialogue window with spell info", AccountStatus::Player, bot_spell_info_dialogue_window) ||
		bot_command_add("spells", "Lists all Spells learned by the Bot.", AccountStatus::Player, bot_command_spell_list) ||
		bot_command_add("spellsettings", "Lists a bot's spell setting entries", AccountStatus::Player, bot_command_spell_settings_list) ||
		bot_command_add("spellsettingsadd", "Add a bot spell setting entry", AccountStatus::Player, bot_command_spell_settings_add) ||
		bot_command_add("spellsettingsdelete", "Delete a bot spell setting entry", AccountStatus::Player, bot_command_spell_settings_delete) ||
		bot_command_add("spellsettingstoggle", "Toggle a bot spell use", AccountStatus::Player, bot_command_spell_settings_toggle) ||
		bot_command_add("spellsettingsupdate", "Update a bot spell setting entry", AccountStatus::Player, bot_command_spell_settings_update) ||
		bot_command_add("summoncorpse", "Orders a bot to summon a corpse to its feet", AccountStatus::Player, bot_command_summon_corpse) ||
		bot_command_add("suspend", "Suspends a bot's AI processing until released", AccountStatus::Player, bot_command_suspend) ||
		bot_command_add("taunt", "Toggles taunt use by a bot", AccountStatus::Player, bot_command_taunt) ||
		bot_command_add("timer", "Checks or clears timers of the chosen type.", AccountStatus::GMMgmt, bot_command_timer) ||
		bot_command_add("track", "Orders a capable bot to track enemies", AccountStatus::Player, bot_command_track) ||
		bot_command_add("viewcombos", "Views bot race class combinations", AccountStatus::Player, bot_command_view_combos) ||
		bot_command_add("waterbreathing", "Orders a bot to cast a water breathing spell", AccountStatus::Player, bot_command_water_breathing)
	) {
		bot_command_deinit();
		return -1;
	}

	std::map<std::string, std::pair<uint8, std::vector<std::string>>> bot_command_settings;
	database.botdb.LoadBotCommandSettings(bot_command_settings);

	std::vector<std::pair<std::string, uint8>> injected_bot_command_settings;
	std::vector<std::string> orphaned_bot_command_settings;

	for (auto bcs_iter : bot_command_settings) {

		auto bcl_iter = bot_command_list.find(bcs_iter.first);
		if (bcl_iter == bot_command_list.end()) {

			orphaned_bot_command_settings.push_back(bcs_iter.first);
			LogInfo(
				"Bot Command [{}] no longer exists... Deleting orphaned entry from `bot_command_settings` table",
				bcs_iter.first.c_str()
			);
		}
	}

	if (orphaned_bot_command_settings.size()) {
		if (!database.botdb.UpdateOrphanedBotCommandSettings(orphaned_bot_command_settings)) {
			LogInfo("Failed to process 'Orphaned Bot Commands' update operation.");
		}
	}

	auto working_bcl = bot_command_list;
	for (auto working_bcl_iter : working_bcl) {

		auto bcs_iter = bot_command_settings.find(working_bcl_iter.first);
		if (bcs_iter == bot_command_settings.end()) {

			injected_bot_command_settings.emplace_back(std::pair<std::string, uint8>(working_bcl_iter.first, working_bcl_iter.second->access));
			LogInfo(
				"New Bot Command [{}] found... Adding to `bot_command_settings` table with access [{}]",
				working_bcl_iter.first.c_str(),
				working_bcl_iter.second->access
			);

			if (working_bcl_iter.second->access == 0) {
				LogCommands(
					"bot_command_init(): Warning: Bot Command [{}] defaulting to access level 0!",
					working_bcl_iter.first.c_str()
				);
			}

			continue;
		}

		working_bcl_iter.second->access = bcs_iter->second.first;
		LogCommands(
			"bot_command_init(): - Bot Command [{}] set to access level [{}]",
			working_bcl_iter.first.c_str(),
			bcs_iter->second.first
		);

		if (bcs_iter->second.second.empty()) {
			continue;
		}

		for (auto alias_iter : bcs_iter->second.second) {
			if (alias_iter.empty()) {
				continue;
			}

			if (bot_command_list.find(alias_iter) != bot_command_list.end()) {
				LogCommands(
					"bot_command_init(): Warning: Alias [{}] already exists as a bot command - skipping!",
					alias_iter.c_str()
				);

				continue;
			}

			bot_command_list[alias_iter] = working_bcl_iter.second;
			bot_command_aliases[alias_iter] = working_bcl_iter.first;

			LogCommands(
				"bot_command_init(): - Alias [{}] added to bot command [{}]",
				alias_iter.c_str(),
				bot_command_aliases[alias_iter].c_str()
			);
		}
	}

	if (injected_bot_command_settings.size()) {
		if (!database.botdb.UpdateInjectedBotCommandSettings(injected_bot_command_settings)) {
			LogInfo("Failed to process 'Injected Bot Commands' update operation.");
		}
	}

	bot_command_dispatch = bot_command_real_dispatch;

	BCSpells::Load();

	return bot_command_count;
}

void bot_command_deinit(void)
{
	bot_command_list.clear();
	bot_command_aliases.clear();

	bot_command_dispatch = bot_command_not_avail;
	bot_command_count = 0;

	BCSpells::Unload();
}

int bot_command_add(std::string bot_command_name, const char *desc, int access, BotCmdFuncPtr function)
{
	if (bot_command_name.empty()) {
		LogError("bot_command_add() - Bot command added with empty name string - check bot_command.cpp");
		return -1;
	}
	if (function == nullptr) {
		LogError("bot_command_add() - Bot command [{}] added without a valid function pointer - check bot_command.cpp", bot_command_name.c_str());
		return -1;
	}
	if (bot_command_list.count(bot_command_name) != 0) {
		LogError("bot_command_add() - Bot command [{}] is a duplicate bot command name - check bot_command.cpp", bot_command_name.c_str());
		return -1;
	}
	for (auto iter : bot_command_list) {
		if (iter.second->function != function)
			continue;
		LogError("bot_command_add() - Bot command [{}] equates to an alias of [{}] - check bot_command.cpp", bot_command_name.c_str(), iter.first.c_str());
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

int bot_command_real_dispatch(Client *c, const char *message)
{
	Seperator sep(message, ' ', 10, 100, true); // "three word argument" should be considered 1 arg

	std::string cstr(sep.arg[0]+1);

	if(bot_command_list.count(cstr) != 1) {
		return(-2);
	}

	BotCommandRecord *cur = bot_command_list[cstr];
	if(c->Admin() < cur->access){
		c->Message(Chat::White, "Your access level is not high enough to use this bot command.");
		return(-1);
	}

	/* QS: Player_Log_Issued_Commands */
	if (RuleB(QueryServ, PlayerLogIssuedCommandes)){
		std::string event_desc = StringFormat("Issued bot command :: '%s' in zoneid:%i instid:%i",  message, c->GetZoneID(), c->GetInstanceID());
		QServ->PlayerLogEvent(Player_Log_Issued_Commands, c->CharacterID(), event_desc);
	}

	if(cur->access >= COMMANDS_LOGGING_MIN_STATUS) {
		LogCommands("[{}] ([{}]) used bot command: [{}] (target=[{}])",  c->GetName(), c->AccountName(), message, c->GetTarget()?c->GetTarget()->GetName():"NONE");
	}

	if(cur->function == nullptr) {
		LogError("Bot command [{}] has a null function\n", cstr.c_str());
		return(-1);
	} else {
		//dispatch C++ bot command
		cur->function(c, &sep);	// dispatch bot command
	}
	return 0;

}

bool helper_bot_appearance_fail(Client *bot_owner, Bot *my_bot, BCEnum::AFType fail_type, const char* type_desc)
{
	switch (fail_type) {
	case BCEnum::AFT_Value:
		bot_owner->Message(Chat::White, "Failed to change '%s' for %s due to invalid value for this command", type_desc, my_bot->GetCleanName());
		return true;
	case BCEnum::AFT_GenderRace:
		bot_owner->Message(Chat::White, "Failed to change '%s' for %s due to invalid bot gender and/or race for this command", type_desc, my_bot->GetCleanName());
		return true;
	case BCEnum::AFT_Race:
		bot_owner->Message(Chat::White, "Failed to change '%s' for %s due to invalid bot race for this command", type_desc, my_bot->GetCleanName());
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
		bot_owner->Message(Chat::White, "Failed to save appearance change for %s due to unknown cause...", my_bot->GetCleanName());
		return;
	}

	helper_bot_appearance_form_update(my_bot);
	bot_owner->Message(Chat::White, "Successfully changed appearance for %s!", my_bot->GetCleanName());
}

void helper_bot_appearance_form_update(Bot *my_bot)
{
	if (!my_bot) {
		return;
	}

	my_bot->SendIllusionPacket(
		AppearanceStruct{
			.beard = my_bot->GetBeard(),
			.beard_color = my_bot->GetBeardColor(),
			.drakkin_details = my_bot->GetDrakkinDetails(),
			.drakkin_heritage = my_bot->GetDrakkinHeritage(),
			.drakkin_tattoo = my_bot->GetDrakkinTattoo(),
			.eye_color_one = my_bot->GetEyeColor1(),
			.eye_color_two = my_bot->GetEyeColor2(),
			.face = my_bot->GetLuclinFace(),
			.gender_id = my_bot->GetGender(),
			.hair = my_bot->GetHairStyle(),
			.hair_color = my_bot->GetHairColor(),
			.helmet_texture = my_bot->GetHelmTexture(),
			.race_id = my_bot->GetRace(),
			.size = my_bot->GetSize(),
			.texture = my_bot->GetTexture(),
		}
	);
}

uint32 helper_bot_create(Client *bot_owner, std::string bot_name, uint8 bot_class, uint16 bot_race, uint8 bot_gender)
{
	uint32 bot_id = 0;
	if (!bot_owner) {
		return bot_id;
	}

	if (!Bot::IsValidName(bot_name)) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"'{}' is an invalid name. You may only use characters 'A-Z' or 'a-z'. Mixed case {} allowed.",
				bot_name, RuleB(Bots, AllowCamelCaseNames) ? "is" : "is not"
			).c_str()
		);
		return bot_id;
	}

	bool available_flag = false;
	if (!database.botdb.QueryNameAvailablity(bot_name, available_flag)) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Failed to query name availability for '{}'.",
				bot_name
			).c_str()
		);
		return bot_id;
	}

	if (!available_flag) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"The name '{}' is already being used. Please choose a different name",
				bot_name
			).c_str()
		);
		return bot_id;
	}

	if (!Bot::IsValidRaceClassCombo(bot_race, bot_class)) {
		const std::string bot_race_name = GetRaceIDName(bot_race);
		const std::string bot_class_name = GetClassIDName(bot_class);

		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} {} is an invalid race-class combination, would you like to {} proper combinations for {}?",
				bot_race_name,
				bot_class_name,
				Saylink::Silent(
					fmt::format("^viewcombos {}", bot_race),
					"view"
				),
				bot_race_name
			).c_str()
		);

		return bot_id;
	}

	if (!EQ::ValueWithin(bot_gender, Gender::Male, Gender::Female)) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Gender: {} ({}) or {} ({})",
				GetGenderName(Gender::Male),
				Gender::Male,
				GetGenderName(Gender::Female),
				Gender::Female
			).c_str()
		);
		return bot_id;
	}

	auto bot_creation_limit = bot_owner->GetBotCreationLimit();
	auto bot_creation_limit_class = bot_owner->GetBotCreationLimit(bot_class);

	uint32 bot_count = 0;
	uint32 bot_class_count = 0;
	if (!database.botdb.QueryBotCount(bot_owner->CharacterID(), bot_class, bot_count, bot_class_count)) {
		bot_owner->Message(Chat::White, "Failed to query bot count.");
		return bot_id;
	}

	if (bot_creation_limit >= 0 && bot_count >= bot_creation_limit) {
		std::string message;

		if (bot_creation_limit) {
			message = fmt::format(
				"You cannot create anymore than {} bot{}.",
				bot_creation_limit,
				bot_creation_limit != 1 ? "s" : ""
			);
		} else {
			message = "You cannot create any bots.";
		}

		bot_owner->Message(Chat::White, message.c_str());
		return bot_id;
	}

	if (bot_creation_limit_class >= 0 && bot_class_count >= bot_creation_limit_class) {
		std::string message;

		if (bot_creation_limit_class) {
			message = fmt::format(
				"You cannot create anymore than {} {} bot{}.",
				bot_creation_limit_class,
				GetClassIDName(bot_class),
				bot_creation_limit_class != 1 ? "s" : ""
			);
		} else {
			message = fmt::format(
				"You cannot create any {} bots.",
				GetClassIDName(bot_class)
			);
		}

		bot_owner->Message(Chat::White, message.c_str());
		return bot_id;
	}

	auto bot_character_level = bot_owner->GetBotRequiredLevel();

	if (
		bot_character_level >= 0 &&
		bot_owner->GetLevel() < bot_character_level
	) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"You must be level {} to use bots.",
				bot_character_level
			).c_str()
		);
		return bot_id;
	}

	auto bot_character_level_class = bot_owner->GetBotRequiredLevel(bot_class);

	if (
		bot_character_level_class >= 0 &&
		bot_owner->GetLevel() < bot_character_level_class
	) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"You must be level {} to use {} bots.",
				bot_character_level_class,
				GetClassIDName(bot_class)
			).c_str()
		);
		return bot_id;
	}


	auto my_bot = new Bot(Bot::CreateDefaultNPCTypeStructForBot(bot_name, "", bot_owner->GetLevel(), bot_race, bot_class, bot_gender), bot_owner);

	if (!my_bot->Save()) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Failed to create '{}' due to unknown cause.",
				my_bot->GetCleanName()
			).c_str()
		);
		safe_delete(my_bot);
		return bot_id;
	}

	bot_owner->Message(
		Chat::White,
		fmt::format(
			"Bot Created | Name: {} ID: {} Race: {} Class: {}",
			my_bot->GetCleanName(),
			my_bot->GetBotID(),
			GetRaceIDName(my_bot->GetRace()),
			GetClassIDName(my_bot->GetClass())
		).c_str()
	);

	bot_id = my_bot->GetBotID();
	if (parse->PlayerHasQuestSub(EVENT_BOT_CREATE)) {
		const auto& export_string = fmt::format(
			"{} {} {} {} {}",
			bot_name,
			bot_id,
			bot_race,
			bot_class,
			bot_gender
		);

		parse->EventPlayer(EVENT_BOT_CREATE, bot_owner, export_string, 0);
	}

	my_bot->AddBotStartingItems(bot_race, bot_class);

	safe_delete(my_bot);

	return bot_id;
}

void helper_bot_out_of_combat(Client *bot_owner, Bot *my_bot)
{
	if (!bot_owner || !my_bot)
		return;

	switch (my_bot->GetClass()) {
	case Class::Warrior:
	case Class::Cleric:
	case Class::Paladin:
	case Class::Ranger:
	case Class::ShadowKnight:
	case Class::Druid:
	case Class::Monk:
		bot_owner->Message(Chat::White, "%s has no out-of-combat behavior defined", my_bot->GetCleanName());
		break;
	case Class::Bard:
		bot_owner->Message(Chat::White, "%s will %s use out-of-combat behavior for bard songs", my_bot->GetCleanName(), ((my_bot->GetAltOutOfCombatBehavior()) ? ("now") : ("no longer")));
		break;
	case Class::Rogue:
	case Class::Shaman:
	case Class::Necromancer:
	case Class::Wizard:
	case Class::Magician:
	case Class::Enchanter:
	case Class::Beastlord:
	case Class::Berserker:
		bot_owner->Message(Chat::White, "%s has no out-of-combat behavior defined", my_bot->GetCleanName());
		break;
	default:
		break;
		bot_owner->Message(Chat::White, "Undefined bot class for %s", my_bot->GetCleanName());
	}
}

int helper_bot_follow_option_chain(Client* bot_owner)
{
	if (!bot_owner) {
		return 0;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(bot_owner, sbl);
	if (sbl.empty()) {
		return 0;
	}

	int chain_follow_count = 0;
	Mob* followee = bot_owner;

	// only add groups that do not belong to bot_owner
	std::map<uint32, Group*> bot_group_map;
	for (auto bot_iter : sbl) {

		if (!bot_iter || bot_iter->GetManualFollow() || bot_iter->GetGroup() == bot_owner->GetGroup()) {
			continue;
		}

		Group* bot_group = bot_iter->GetGroup();
		if (!bot_iter->GetGroup()) {
			continue;
		}

		bot_group_map[bot_group->GetID()] = bot_group;
	}

	std::list<Bot*> bot_member_list;
	if (bot_owner->GetGroup()) {

		bot_owner->GetGroup()->GetBotList(bot_member_list);
		for (auto bot_member_iter : bot_member_list) {

			if (!bot_member_iter || bot_member_iter->GetBotOwnerCharacterID() != bot_owner->CharacterID() || bot_member_iter == followee || bot_member_iter->GetManualFollow()) {
				continue;
			}

			bot_member_iter->SetFollowID(followee->GetID());
			followee = bot_member_iter;
			++chain_follow_count;
		}
	}

	for (auto bot_group_iter : bot_group_map) {

		if (!bot_group_iter.second) {
			continue;
		}

		bot_group_iter.second->GetBotList(bot_member_list);
		for (auto bot_member_iter : bot_member_list) {

			if (!bot_member_iter || bot_member_iter->GetBotOwnerCharacterID() != bot_owner->CharacterID() || bot_member_iter == followee || bot_member_iter->GetManualFollow()) {
				continue;
			}

			bot_member_iter->SetFollowID(followee->GetID());
			followee = bot_member_iter;
			++chain_follow_count;
		}
	}

	return chain_follow_count;
}

bool helper_cast_standard_spell(Bot* casting_bot, Mob* target_mob, int spell_id, bool annouce_cast, uint32* dont_root_before)
{
	if (!casting_bot || !target_mob)
		return false;

	casting_bot->InterruptSpell();
	if (annouce_cast) {
		Bot::BotGroupSay(
			casting_bot,
			fmt::format(
				"Attempting to cast {} on {}.",
				spells[spell_id].name,
				target_mob->GetCleanName()
			).c_str()
		);
	}

	return casting_bot->CastSpell(spell_id, target_mob->GetID(), EQ::spells::CastingSlot::Gem2, -1, -1, dont_root_before);
}

bool helper_command_disabled(Client* bot_owner, bool rule_value, const char* command)
{
	if (!rule_value) {
		bot_owner->Message(Chat::White, "Bot command %s is not enabled on this server.", command);
		return true;
	}

	return false;
}

bool helper_command_alias_fail(Client *bot_owner, const char* command_handler, const char *alias, const char *command)
{
	auto alias_iter = bot_command_aliases.find(&alias[1]);
	if (alias_iter == bot_command_aliases.end() || alias_iter->second.compare(command)) {
		bot_owner->Message(Chat::White, "Undefined linker usage in %s (%s)", command_handler, &alias[1]);
		return true;
	}

	return false;
}

void helper_command_depart_list(Client* bot_owner, Bot* druid_bot, Bot* wizard_bot, bcst_list* local_list, bool single_flag)
{
	if (!bot_owner) {
		return;
	}

	if (!MyBots::IsMyBot(bot_owner, druid_bot)) {
		druid_bot = nullptr;
	}

	if (!MyBots::IsMyBot(bot_owner, wizard_bot)) {
		wizard_bot = nullptr;
	}

	if (!druid_bot && !wizard_bot) {
		bot_owner->Message(Chat::White, "No bots are capable of performing this action");
		return;
	}

	if (!local_list) {
		bot_owner->Message(Chat::White, "There are no destinations you can be taken to.");
		return;
	}

	auto destination_count = 0;
	auto destination_number = 1;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToDepart();
		if (!local_entry) {
			continue;
		}

		if (
			druid_bot &&
			druid_bot->GetClass() == local_entry->caster_class &&
			druid_bot->GetLevel() >= local_entry->spell_level
		) {
			if (local_entry->single != single_flag) {
				continue;
			}

			druid_bot->OwnerMessage(
				fmt::format(
					"Destination {} | {} | {}",
					destination_number,
					local_entry->long_name,
					Saylink::Silent(
						fmt::format(
							"^circle {}{}",
							spells[local_entry->spell_id].teleport_zone,
							single_flag ? " single" : ""
						),
						"Goto"
					)
				)
			);

			destination_count++;
			destination_number++;
			continue;
		}

		if (
			wizard_bot &&
			wizard_bot->GetClass() == local_entry->caster_class &&
			wizard_bot->GetLevel() >= local_entry->spell_level
		) {
			if (local_entry->single != single_flag) {
				continue;
			}

			wizard_bot->OwnerMessage(
				fmt::format(
					"Destination {} | {} | {}",
					destination_number,
					local_entry->long_name,
					Saylink::Silent(
						fmt::format(
							"^portal {}{}",
							spells[local_entry->spell_id].teleport_zone,
							single_flag ? " single" : ""
						),
						"Goto"
					)
				)
			);

			destination_count++;
			destination_number++;
			continue;
		}
	}

	if (!destination_count) {
		bot_owner->Message(Chat::White, "There are no destinations you can be taken to.");
	}
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
		bot_owner->Message(Chat::White, "No bots are capable of performing this action");
		return true;
	}

	return false;
}

void helper_send_available_subcommands(Client *bot_owner, const char* command_simile, const std::list<const char*>& subcommand_list)
{
	bot_owner->Message(Chat::White, "Available %s management subcommands:", command_simile);

	int bot_subcommands_shown = 0;
	for (const auto subcommand_iter : subcommand_list) {
		auto find_iter = bot_command_list.find(subcommand_iter);
		if (find_iter == bot_command_list.end())
			continue;
		if (bot_owner->Admin() < find_iter->second->access)
			continue;

		bot_owner->Message(
			Chat::White,
			fmt::format(
				"^{} - {}",
				subcommand_iter,
				find_iter != bot_command_list.end() ? find_iter->second->desc : "No Description"
			).c_str()
		);

		++bot_subcommands_shown;
	}

	bot_owner->Message(Chat::White, "%d bot subcommand%s listed.", bot_subcommands_shown, bot_subcommands_shown != 1 ? "s" : "");
}

void helper_send_usage_required_bots(Client *bot_owner, BCEnum::SpType spell_type, uint8 bot_class)
{
	bot_owner->Message(Chat::White, "requires one of the following bot classes:");
	if (bot_class)
		bot_owner->Message(Chat::White, "%s", required_bots_map_by_class[spell_type][bot_class].c_str());
	else
		bot_owner->Message(Chat::White, "%s", required_bots_map[spell_type].c_str());
}

bool helper_spell_check_fail(STBaseEntry* local_entry)
{
	if (!local_entry)
		return true;
	if (spells[local_entry->spell_id].zone_type && zone->GetZoneType() && !(spells[local_entry->spell_id].zone_type & zone->GetZoneType()))
		return true;

	return false;
}

bool helper_spell_list_fail(Client *bot_owner, bcst_list* spell_list, BCEnum::SpType spell_type)
{
	if (!spell_list || spell_list->empty()) {
		bot_owner->Message(Chat::White, "%s", required_bots_map[spell_type].c_str());
		return true;
	}

	return false;
}

#include "bot_commands/actionable.cpp"
#include "bot_commands/aggressive.cpp"
#include "bot_commands/appearance.cpp"
#include "bot_commands/apply_poison.cpp"
#include "bot_commands/apply_potion.cpp"
#include "bot_commands/attack.cpp"
#include "bot_commands/bind_affinity.cpp"
#include "bot_commands/bot.cpp"
#include "bot_commands/caster_range.cpp"
#include "bot_commands/charm.cpp"
#include "bot_commands/click_item.cpp"
#include "bot_commands/cure.cpp"
#include "bot_commands/defensive.cpp"
#include "bot_commands/depart.cpp"
#include "bot_commands/escape.cpp"
#include "bot_commands/find_aliases.cpp"
#include "bot_commands/follow.cpp"
#include "bot_commands/guard.cpp"
#include "bot_commands/heal_rotation.cpp"
#include "bot_commands/help.cpp"
#include "bot_commands/hold.cpp"
#include "bot_commands/identify.cpp"
#include "bot_commands/inventory.cpp"
#include "bot_commands/invisibility.cpp"
#include "bot_commands/item_use.cpp"
#include "bot_commands/levitation.cpp"
#include "bot_commands/lull.cpp"
#include "bot_commands/mesmerize.cpp"
#include "bot_commands/movement_speed.cpp"
#include "bot_commands/name.cpp"
#include "bot_commands/owner_option.cpp"
#include "bot_commands/pet.cpp"
#include "bot_commands/pick_lock.cpp"
#include "bot_commands/pickpocket.cpp"
#include "bot_commands/precombat.cpp"
#include "bot_commands/pull.cpp"
#include "bot_commands/release.cpp"
#include "bot_commands/resistance.cpp"
#include "bot_commands/resurrect.cpp"
#include "bot_commands/rune.cpp"
#include "bot_commands/send_home.cpp"
#include "bot_commands/size.cpp"
#include "bot_commands/spell.cpp"
#include "bot_commands/summon.cpp"
#include "bot_commands/summon_corpse.cpp"
#include "bot_commands/suspend.cpp"
#include "bot_commands/taunt.cpp"
#include "bot_commands/teleport.cpp"
#include "bot_commands/timer.cpp"
#include "bot_commands/track.cpp"
#include "bot_commands/view_combos.cpp"
#include "bot_commands/water_breathing.cpp"
