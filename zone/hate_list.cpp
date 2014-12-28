/*	 EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include "client.h"
#include "entity.h"
#include "groups.h"
#include "mob.h"
#include "raids.h"

#include "../common/rulesys.h"

#include "hate_list.h"
#include "quest_parser_collection.h"
#include "zone.h"
#include "water_map.h"

#include <stdlib.h>
#include <list>

extern Zone *zone;

HateList::HateList()
{
	owner = nullptr;
}

HateList::~HateList()
{
}

// added for frenzy support
// checks if target still is in frenzy mode
void HateList::IsEntityInFrenzyMode()
{
	auto iterator = list.begin();
	while (iterator != list.end()) {
		if ((*iterator)->entity_on_hatelist->GetHPRatio() >= 20) {
			(*iterator)->is_entity_frenzy = false;
		}
		++iterator;
	}
}

void HateList::WipeHateList()
{
	auto iterator = list.begin();
	while (iterator != list.end()) {
		Mob* m = (*iterator)->entity_on_hatelist;
		if (m) {
			parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), m, "0", 0);
			if (m->IsClient()) {
				m->CastToClient()->DecrementAggroCount();
			}
		}
		delete (*iterator);
		iterator = list.erase(iterator);
	}
}

bool HateList::IsEntOnHateList(Mob *mob)
{
	if (Find(mob)) {
		return true;
	}
	return false;
}

struct_HateList *HateList::Find(Mob *ent)
{
	auto iterator = list.begin();
	while (iterator != list.end()) {
		if ((*iterator)->entity_on_hatelist == ent) {
			return (*iterator);
		}
		++iterator;
	}
	return nullptr;
}

void HateList::SetHateAmountOnEnt(Mob* other, uint32 in_hate, uint32 in_damage)
{
	struct_HateList *hate_list = Find(other);
	if (hate_list) {
		if (in_damage > 0) {
			hate_list->hatelist_damage = in_damage;
		}
		if (in_hate > 0) {
			hate_list->stored_hate_amount = in_hate;
		}
	}
}

Mob* HateList::GetDamageTopOnHateList(Mob* hater)
{
	Mob* current = nullptr;
	Group* grp = nullptr;
	Raid* r = nullptr;
	uint32 dmg_amt = 0;
	auto iterator = list.begin();
	while (iterator != list.end()) {
		grp = nullptr;
		r = nullptr;
		if ((*iterator)->entity_on_hatelist && (*iterator)->entity_on_hatelist->IsClient()) {
			r = entity_list.GetRaidByClient((*iterator)->entity_on_hatelist->CastToClient());
		}
		grp = entity_list.GetGroupByMob((*iterator)->entity_on_hatelist);
		if ((*iterator)->entity_on_hatelist && r) {
			if (r->GetTotalRaidDamage(hater) >= dmg_amt) {
				current = (*iterator)->entity_on_hatelist;
				dmg_amt = r->GetTotalRaidDamage(hater);
			}
		} else if ((*iterator)->entity_on_hatelist != nullptr && grp != nullptr) {
			if (grp->GetTotalGroupDamage(hater) >= dmg_amt) {
				current = (*iterator)->entity_on_hatelist;
				dmg_amt = grp->GetTotalGroupDamage(hater);
			}
		} else if ((*iterator)->entity_on_hatelist != nullptr && (uint32)(*iterator)->hatelist_damage >= dmg_amt) {
			current = (*iterator)->entity_on_hatelist;
			dmg_amt = (*iterator)->hatelist_damage;
		}
		++iterator;
	}
	return current;
}

Mob* HateList::GetClosestEntOnHateList(Mob *hater)
{
	Mob* close_entity = nullptr;
	float close_distance = 99999.9f;
	float this_distance;
	auto iterator = list.begin();
	while (iterator != list.end()) {
		this_distance = (*iterator)->entity_on_hatelist->DistNoRootNoZ(*hater);
		if ((*iterator)->entity_on_hatelist != nullptr && this_distance <= close_distance) {
			close_distance = this_distance;
			close_entity = (*iterator)->entity_on_hatelist;
		}
		++iterator;
	}
	if ((!close_entity && hater->IsNPC()) || (close_entity && close_entity->DivineAura())) {
		close_entity = hater->CastToNPC()->GetHateTop();
	}
	return close_entity;
}

void HateList::AddEntToHateList(Mob *in_entity, int32 in_hate, int32 in_damage, bool in_is_entity_frenzy, bool iAddIfNotExist)
{
	if (!in_entity) {
		return;
	}
	if (in_entity->IsCorpse()) {
		return;
	}
	if (in_entity->IsClient() && in_entity->CastToClient()->IsDead()) {
		return;
	}
	struct_HateList *hate_list = Find(in_entity);
	if (hate_list) {
		hate_list->hatelist_damage += (in_damage >= 0) ? in_damage : 0;
		hate_list->stored_hate_amount += in_hate;
		hate_list->is_entity_frenzy = in_is_entity_frenzy;
	} else if (iAddIfNotExist) {
		hate_list = new struct_HateList;
		hate_list->entity_on_hatelist = in_entity;
		hate_list->hatelist_damage = (in_damage >= 0) ? in_damage : 0;
		hate_list->stored_hate_amount = in_hate;
		hate_list->is_entity_frenzy = in_is_entity_frenzy;
		list.push_back(hate_list);
		parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), in_entity, "1", 0);
		if (in_entity->IsClient()) {
			if (owner->CastToNPC()->IsRaidTarget()) {
				in_entity->CastToClient()->SetEngagedRaidTarget(true);
			}
			in_entity->CastToClient()->IncrementAggroCount();
		}
	}
}

bool HateList::RemoveEntFromHateList(Mob *in_entity)
{
	if (!in_entity) {
		return false;
	}
	bool found = false;
	auto iterator = list.begin();
	while (iterator != list.end()) {
		if ((*iterator)->entity_on_hatelist == in_entity) {
			if (in_entity) {
				parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), in_entity, "0", 0);
			}
			found = true;
			if (in_entity && in_entity->IsClient()) {
				in_entity->CastToClient()->DecrementAggroCount();
			}
			delete (*iterator);
			iterator = list.erase(iterator);
		} else {
			++iterator;
		}
	}
	return found;
}

void HateList::DoFactionHits(int32 npc_faction_level_id)
{
	if (npc_faction_level_id <= 0) {
		return;
	}
	auto iterator = list.begin();
	while (iterator != list.end()) {
		Client *client;
		if ((*iterator)->entity_on_hatelist && (*iterator)->entity_on_hatelist->IsClient()) {
			client = (*iterator)->entity_on_hatelist->CastToClient();
		} else {
			client = nullptr;
		}
		if (client) {
			client->SetFactionLevel(client->CharacterID(), npc_faction_level_id, client->GetBaseClass(), client->GetBaseRace(), client->GetDeity());
		}
		++iterator;
	}
}

int HateList::GetSummonedPetCountOnHateList(Mob *hater)
{
	int petcount = 0;
	auto iterator = list.begin();
	while (iterator != list.end()) {
		if ((*iterator)->entity_on_hatelist != nullptr && (*iterator)->entity_on_hatelist->IsNPC() && 	((*iterator)->entity_on_hatelist->CastToNPC()->IsPet() || ((*iterator)->entity_on_hatelist->CastToNPC()->GetSwarmOwner() > 0))) {
			++petcount;
		}
		++iterator;
	}
	return petcount;
}

Mob *HateList::GetEntWithMostHateInRange(Mob *entity_as_center)
{
	/* Hack fix for zone shutdown crashes on some servers */
	if (!zone->IsLoaded()) {
		return nullptr;
	}

	Mob* entity_with_most_hate = nullptr;
	int32 hate_status = -1;
	uint32 temp_hate_tracker = 0;

	if (entity_as_center == nullptr) {
		return nullptr;
	}
	if (RuleB(Aggro, SmartAggroList)) {
		Mob* top_client_type_in_range = nullptr;
		int32 hate_client_type_in_range = -1;
		int skipped_count = 0;
		auto iterator = list.begin();
		while (iterator != list.end()) {
			struct_HateList *hate_list = (*iterator);
			int16 aggro_mod = 0;
			if (!hate_list) {
				++iterator;
				continue;
			}
			if (!hate_list->entity_on_hatelist) {
				++iterator;
				continue;
			}
			if (entity_as_center->IsNPC() && entity_as_center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if (!zone->watermap->InLiquid(hate_list->entity_on_hatelist->GetX(), hate_list->entity_on_hatelist->GetY(), hate_list->entity_on_hatelist->GetZ())) {
					skipped_count++;
					++iterator;
					continue;
				}
			}
			if (hate_list->entity_on_hatelist->Sanctuary()) {
				if (hate_status == -1) {
					entity_with_most_hate = hate_list->entity_on_hatelist;
					hate_status = 1;
				}
				++iterator;
				continue;
			}
			if (hate_list->entity_on_hatelist->DivineAura() || hate_list->entity_on_hatelist->IsMezzed() || hate_list->entity_on_hatelist->IsFeared()) {
				if (hate_status == -1) {
					entity_with_most_hate = hate_list->entity_on_hatelist;
					hate_status = 0;
				}
				++iterator;
				continue;
			}
			uint32 current_stored_hate = hate_list->stored_hate_amount;
			if (hate_list->entity_on_hatelist->IsClient()) {
				if (hate_list->entity_on_hatelist->CastToClient()->IsSitting()) {
					aggro_mod += RuleI(Aggro, SittingAggroMod);
				}
				if (entity_as_center) {
					if (entity_as_center->GetTarget() == hate_list->entity_on_hatelist) {
						aggro_mod += RuleI(Aggro, CurrentTargetAggroMod);
					}
					if (RuleI(Aggro, MeleeRangeAggroMod) != 0) {
						if (entity_as_center->CombatRange(hate_list->entity_on_hatelist)) {
							aggro_mod += RuleI(Aggro, MeleeRangeAggroMod);
							if (current_stored_hate > hate_client_type_in_range || hate_list->is_entity_frenzy) {
								hate_client_type_in_range = current_stored_hate;
								top_client_type_in_range = hate_list->entity_on_hatelist;
							}
						}
					}
				}
			}
			else {
				if (entity_as_center) {
					if (entity_as_center->GetTarget() == hate_list->entity_on_hatelist) {
						aggro_mod += RuleI(Aggro, CurrentTargetAggroMod);
					}
					if (RuleI(Aggro, MeleeRangeAggroMod) != 0) {
						if (entity_as_center->CombatRange(hate_list->entity_on_hatelist)) {
							aggro_mod += RuleI(Aggro, MeleeRangeAggroMod);
						}
					}
				}
			}
			if (hate_list->entity_on_hatelist->GetMaxHP() != 0 && ((hate_list->entity_on_hatelist->GetHP() * 100 / hate_list->entity_on_hatelist->GetMaxHP()) < 20)) {
				aggro_mod += RuleI(Aggro, CriticallyWoundedAggroMod);
			}
			if (aggro_mod) {
				current_stored_hate += (current_stored_hate * aggro_mod / 100);
			}
			if (current_stored_hate > temp_hate_tracker || hate_list->is_entity_frenzy) {
				temp_hate_tracker = current_stored_hate;
				entity_with_most_hate = hate_list->entity_on_hatelist;
			}
			++iterator;
		}
		if (top_client_type_in_range != nullptr && entity_with_most_hate != nullptr) {
			bool is_top_client_type = entity_with_most_hate->IsClient();
			#ifdef BOTS
			if (!is_top_client_type) {
				if (entity_with_most_hate->IsBot()) {
					is_top_client_type = true;
					top_client_type_in_range = entity_with_most_hate;
				}
			}
			#endif //BOTS
			if (!is_top_client_type) {
				if (entity_with_most_hate->IsMerc()) {
					is_top_client_type = true;
					top_client_type_in_range = entity_with_most_hate;
				}
			}
			if (!is_top_client_type) {
				if (entity_with_most_hate->GetSpecialAbility(ALLOW_TO_TANK)) {
					is_top_client_type = true;
					top_client_type_in_range = entity_with_most_hate;
				}
			}
			if (!is_top_client_type) {
				return top_client_type_in_range ? top_client_type_in_range : nullptr;
			}
			return entity_with_most_hate ? entity_with_most_hate : nullptr;
		} else {
			if (entity_with_most_hate == nullptr && skipped_count > 0) {
				return entity_as_center->GetTarget() ? entity_as_center->GetTarget() : nullptr;
			}
			return entity_with_most_hate ? entity_with_most_hate : nullptr;
		}
	} 
	/* Process not so smart aggro list */
	else {
		auto iterator = list.begin();
		int skipped_count = 0;
		while (iterator != list.end()) {
			struct_HateList *hate_list = (*iterator);
			if (entity_as_center->IsNPC() && entity_as_center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if (!zone->watermap->InLiquid(hate_list->entity_on_hatelist->GetX(), hate_list->entity_on_hatelist->GetY(), hate_list->entity_on_hatelist->GetZ())) {
					skipped_count++;
					++iterator;
					continue;
				}
			}
			if (hate_list->entity_on_hatelist != nullptr && ((hate_list->stored_hate_amount > temp_hate_tracker) || hate_list->is_entity_frenzy )) {
				entity_with_most_hate = hate_list->entity_on_hatelist;
				temp_hate_tracker = hate_list->stored_hate_amount;
			}
			++iterator;
		}
		if (entity_with_most_hate == nullptr && skipped_count > 0) {
			return entity_as_center->GetTarget() ? entity_as_center->GetTarget() : nullptr;
		}
		return entity_with_most_hate ? entity_with_most_hate : nullptr;
	}
	return nullptr;
}

Mob *HateList::GetEntWithMostHateOnList()
{
	Mob* top = nullptr;
	uint32 hate = 0;
	auto iterator = list.begin();
	while (iterator != list.end()) {
		struct_HateList *cur = (*iterator);
		if (cur->entity_on_hatelist != nullptr && (cur->stored_hate_amount > hate)) {
			top = cur->entity_on_hatelist;
			hate = cur->stored_hate_amount;
		}
		++iterator;
	}
	return top;
}


Mob *HateList::GetRandomEntOnHateList()
{
	int count = list.size();
	if (count == 0) { //If we don't have any entries it'll crash getting a random 0, -1 position.
		return NULL;
	}
	if (count == 1) { //No need to do all that extra work if we only have one hate entry
		if (*list.begin()) { // Just in case tHateEntry is invalidated somehow...
			return (*list.begin())->entity_on_hatelist;
		}
		return NULL;
	}
	auto iterator = list.begin();
	int random = zone->random.Int(0, count - 1);
	for (int i = 0; i < random; i++) {
		++iterator;
	}
	return (*iterator)->entity_on_hatelist;
}

uint32 HateList::GetEntHateAmount(Mob *ent, bool damage /*= false*/)
{
	struct_HateList *hate_list;
	hate_list = Find(ent);
	if ( hate_list && damage) {
		return hate_list->hatelist_damage;
	} else if (hate_list) {
		return hate_list->stored_hate_amount;
	} else {
		return 0;
	}
}

bool HateList::IsHateListEmpty()
{
	return (list.size() == 0);
}

void HateList::PrintHateListToClient(Client *c)
{
	auto iterator = list.begin();
	while (iterator != list.end()) {
		struct_HateList *hate_list = (*iterator);

		c->Message(0, "- name: %s, damage: %d, hate: %d",
		           (hate_list->entity_on_hatelist && hate_list->entity_on_hatelist->GetName()) ? hate_list->entity_on_hatelist->GetName() : "(null)",
		           hate_list->hatelist_damage, hate_list->stored_hate_amount);
		++iterator;
	}
}

int HateList::AreaRampage(Mob *in_caster, Mob *in_target, int in_count, ExtraAttackOptions *options)
{
	if (!in_target || !in_caster) {
		return 0;
	}
	int ret = 0;
	std::list<uint32> id_list;
	auto iterator = list.begin();
	while (iterator != list.end()) {
		struct_HateList *h = (*iterator);
		++iterator;
		if (h && h->entity_on_hatelist && h->entity_on_hatelist != in_caster) {
			if (in_caster->CombatRange(h->entity_on_hatelist)) {
				id_list.push_back(h->entity_on_hatelist->GetID());
				++ret;
			}
		}
	}
	std::list<uint32>::iterator iter = id_list.begin();
	while (iter != id_list.end()) {
		Mob *cur = entity_list.GetMobID((*iter));
		if (cur) {
			for (int i = 0; i < in_count; ++i) {
				in_caster->Attack(cur, MainPrimary, false, false, false, options);
			}
		}
		iter++;
	}
	return ret;
}

void HateList::SpellCast(Mob *caster, uint32 spell_id, float range, Mob* ae_center)
{
	if (!caster) {
		return;
	}
	Mob* center = caster;
	if (ae_center) {
		center = ae_center;
	}
	//this is slower than just iterating through the list but avoids
	//crashes when people kick the bucket in the middle of this call
	//that invalidates our iterator but there's no way to know sadly
	//So keep a list of entity ids and look up after
	std::list<uint32> id_list;
	range = range * range;
	float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;
	float dist_targ = 0;
	auto iterator = list.begin();
	while (iterator != list.end()) {
		struct_HateList *h = (*iterator);
		if (range > 0) {
			dist_targ = center->DistNoRoot(*h->entity_on_hatelist);
			if (dist_targ <= range && dist_targ >= min_range2) {
				id_list.push_back(h->entity_on_hatelist->GetID());
				h->entity_on_hatelist->CalcSpellPowerDistanceMod(spell_id, dist_targ);
			}
		} else {
			id_list.push_back(h->entity_on_hatelist->GetID());
			h->entity_on_hatelist->CalcSpellPowerDistanceMod(spell_id, 0, caster);
		}
		++iterator;
	}
	std::list<uint32>::iterator iter = id_list.begin();
	while (iter != id_list.end()) {
		Mob *cur = entity_list.GetMobID((*iter));
		if (cur) {
			caster->SpellOnTarget(spell_id, cur);
		}
		iter++;
	}
}

