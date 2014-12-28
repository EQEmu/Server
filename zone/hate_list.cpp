/*	EQEMu: Everquest Server Emulator
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
	while (iterator != list.end())
	{
		if ((*iterator)->entity_on_hatelist->GetHPRatio() >= 20)
			(*iterator)->is_entity_frenzy = false;
		++iterator;
	}
}

void HateList::WipeHateList()
{
	auto iterator = list.begin();

	while (iterator != list.end())
	{
		Mob* m = (*iterator)->entity_on_hatelist;
		if (m)
		{
			parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), m, "0", 0);

			if (m->IsClient())
				m->CastToClient()->DecrementAggroCount();
		}
		delete (*iterator);
		iterator = list.erase(iterator);

	}
}

bool HateList::IsEntOnHateList(Mob *mob)
{
	if (Find(mob))
		return true;
	return false;
}

struct_HateList *HateList::Find(Mob *ent)
{
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		if ((*iterator)->entity_on_hatelist == ent)
			return (*iterator);
		++iterator;
	}
	return nullptr;
}

void HateList::SetHateAmountOnEnt(Mob* other, uint32 in_hate, uint32 in_dam)
{
	struct_HateList *p = Find(other);
	if (p)
	{
		if (in_dam > 0)
			p->hatelist_damage = in_dam;
		if (in_hate > 0)
			p->stored_hate_amount = in_hate;
	}
}

Mob* HateList::GetDamageTopOnHateList(Mob* hater)
{
	Mob* current = nullptr;
	Group* grp = nullptr;
	Raid* r = nullptr;
	uint32 dmg_amt = 0;

	auto iterator = list.begin();
	while (iterator != list.end())
	{
		grp = nullptr;
		r = nullptr;

		if ((*iterator)->entity_on_hatelist && (*iterator)->entity_on_hatelist->IsClient()){
			r = entity_list.GetRaidByClient((*iterator)->entity_on_hatelist->CastToClient());
		}

		grp = entity_list.GetGroupByMob((*iterator)->entity_on_hatelist);

		if ((*iterator)->entity_on_hatelist && r){
			if (r->GetTotalRaidDamage(hater) >= dmg_amt)
			{
				current = (*iterator)->entity_on_hatelist;
				dmg_amt = r->GetTotalRaidDamage(hater);
			}
		}
		else if ((*iterator)->entity_on_hatelist != nullptr && grp != nullptr)
		{
			if (grp->GetTotalGroupDamage(hater) >= dmg_amt)
			{
				current = (*iterator)->entity_on_hatelist;
				dmg_amt = grp->GetTotalGroupDamage(hater);
			}
		}
		else if ((*iterator)->entity_on_hatelist != nullptr && (uint32)(*iterator)->hatelist_damage >= dmg_amt)
		{
			current = (*iterator)->entity_on_hatelist;
			dmg_amt = (*iterator)->hatelist_damage;
		}
		++iterator;
	}
	return current;
}

Mob* HateList::GetClosestEntOnHateList(Mob *hater) {
	Mob* close = nullptr;
	float closedist = 99999.9f;
	float thisdist;

	auto iterator = list.begin();
	while (iterator != list.end()) {
		thisdist = (*iterator)->entity_on_hatelist->DistNoRootNoZ(*hater);
		if ((*iterator)->entity_on_hatelist != nullptr && thisdist <= closedist) {
			closedist = thisdist;
			close = (*iterator)->entity_on_hatelist;
		}
		++iterator;
	}

	if ((!close && hater->IsNPC()) || (close && close->DivineAura()))
		close = hater->CastToNPC()->GetHateTop();

	return close;
}


// a few comments added, rearranged code for readability
void HateList::AddEntToHateList(Mob *ent, int32 in_hate, int32 in_dam, bool bFrenzy, bool iAddIfNotExist)
{
	if (!ent)
		return;

	if (ent->IsCorpse())
		return;

	if (ent->IsClient() && ent->CastToClient()->IsDead())
		return;

	struct_HateList *p = Find(ent);
	if (p)
	{
		p->hatelist_damage += (in_dam >= 0) ? in_dam : 0;
		p->stored_hate_amount += in_hate;
		p->is_entity_frenzy = bFrenzy;
	}
	else if (iAddIfNotExist) {
		p = new struct_HateList;
		p->entity_on_hatelist = ent;
		p->hatelist_damage = (in_dam >= 0) ? in_dam : 0;
		p->stored_hate_amount = in_hate;
		p->is_entity_frenzy = bFrenzy;
		list.push_back(p);
		parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), ent, "1", 0);

		if (ent->IsClient()) {
			if (owner->CastToNPC()->IsRaidTarget())
				ent->CastToClient()->SetEngagedRaidTarget(true);
			ent->CastToClient()->IncrementAggroCount();
		}
	}
}

bool HateList::RemoveEntFromHateList(Mob *ent)
{
	if (!ent)
		return false;

	bool found = false;
	auto iterator = list.begin();

	while (iterator != list.end())
	{
		if ((*iterator)->entity_on_hatelist == ent)
		{
			if (ent)
				parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), ent, "0", 0);
			found = true;


			if (ent && ent->IsClient())
				ent->CastToClient()->DecrementAggroCount();

			delete (*iterator);
			iterator = list.erase(iterator);

		}
		else
			++iterator;
	}
	return found;
}

void HateList::DoFactionHits(int32 nfl_id) {
	if (nfl_id <= 0)
		return;
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		Client *p;

		if ((*iterator)->entity_on_hatelist && (*iterator)->entity_on_hatelist->IsClient())
			p = (*iterator)->entity_on_hatelist->CastToClient();
		else
			p = nullptr;

		if (p)
			p->SetFactionLevel(p->CharacterID(), nfl_id, p->GetBaseClass(), p->GetBaseRace(), p->GetDeity());
		++iterator;
	}
}

int HateList::GetSummonedPetCountOnHateList(Mob *hater) {

	//Function to get number of 'Summoned' pets on a targets hate list to allow calculations for certian spell effects.
	//Unclear from description that pets are required to be 'summoned body type'. Will not require at this time.
	int petcount = 0;
	auto iterator = list.begin();
	while (iterator != list.end()) {

		if ((*iterator)->entity_on_hatelist != nullptr && (*iterator)->entity_on_hatelist->IsNPC() && ((*iterator)->entity_on_hatelist->CastToNPC()->IsPet() || ((*iterator)->entity_on_hatelist->CastToNPC()->GetSwarmOwner() > 0)))
		{
			++petcount;
		}

		++iterator;
	}

	return petcount;
}

Mob *HateList::GetEntWithMostHateInRange(Mob *center)
{
	// hack fix for zone shutdown crashes on some servers
	if (!zone->IsLoaded())
		return nullptr;

	Mob* top = nullptr;
	int32 hate = -1;

	if (center == nullptr)
		return nullptr;

	if (RuleB(Aggro, SmartAggroList)){
		Mob* topClientTypeInRange = nullptr;
		int32 hateClientTypeInRange = -1;
		int skipped_count = 0;

		auto iterator = list.begin();
		while (iterator != list.end())
		{
			struct_HateList *cur = (*iterator);
			int16 aggroMod = 0;

			if (!cur){
				++iterator;
				continue;
			}

			if (!cur->entity_on_hatelist){
				++iterator;
				continue;
			}

			if (center->IsNPC() && center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if (!zone->watermap->InLiquid(cur->entity_on_hatelist->GetX(), cur->entity_on_hatelist->GetY(), cur->entity_on_hatelist->GetZ())) {
					skipped_count++;
					++iterator;
					continue;
				}
			}

			if (cur->entity_on_hatelist->Sanctuary()) {
				if (hate == -1)
				{
					top = cur->entity_on_hatelist;
					hate = 1;
				}
				++iterator;
				continue;
			}

			if (cur->entity_on_hatelist->DivineAura() || cur->entity_on_hatelist->IsMezzed() || cur->entity_on_hatelist->IsFeared()){
				if (hate == -1)
				{
					top = cur->entity_on_hatelist;
					hate = 0;
				}
				++iterator;
				continue;
			}

			int32 currentHate = cur->stored_hate_amount;

			if (cur->entity_on_hatelist->IsClient()){

				if (cur->entity_on_hatelist->CastToClient()->IsSitting()){
					aggroMod += RuleI(Aggro, SittingAggroMod);
				}

				if (center){
					if (center->GetTarget() == cur->entity_on_hatelist)
						aggroMod += RuleI(Aggro, CurrentTargetAggroMod);
					if (RuleI(Aggro, MeleeRangeAggroMod) != 0)
					{
						if (center->CombatRange(cur->entity_on_hatelist)){
							aggroMod += RuleI(Aggro, MeleeRangeAggroMod);

							if (currentHate > hateClientTypeInRange || cur->is_entity_frenzy){
								hateClientTypeInRange = currentHate;
								topClientTypeInRange = cur->entity_on_hatelist;
							}
						}
					}
				}

			}
			else{
				if (center){
					if (center->GetTarget() == cur->entity_on_hatelist)
						aggroMod += RuleI(Aggro, CurrentTargetAggroMod);
					if (RuleI(Aggro, MeleeRangeAggroMod) != 0)
					{
						if (center->CombatRange(cur->entity_on_hatelist)){
							aggroMod += RuleI(Aggro, MeleeRangeAggroMod);
						}
					}
				}
			}

			if (cur->entity_on_hatelist->GetMaxHP() != 0 && ((cur->entity_on_hatelist->GetHP() * 100 / cur->entity_on_hatelist->GetMaxHP()) < 20)){
				aggroMod += RuleI(Aggro, CriticallyWoundedAggroMod);
			}

			if (aggroMod){
				currentHate += (currentHate * aggroMod / 100);
			}

			if (currentHate > hate || cur->is_entity_frenzy){
				hate = currentHate;
				top = cur->entity_on_hatelist;
			}

			++iterator;
		}

		if (topClientTypeInRange != nullptr && top != nullptr) {
			bool isTopClientType = top->IsClient();
#ifdef BOTS
			if (!isTopClientType) {
				if (top->IsBot()) {
					isTopClientType = true;
					topClientTypeInRange = top;
				}
			}
#endif //BOTS

			if (!isTopClientType) {
				if (top->IsMerc()) {
					isTopClientType = true;
					topClientTypeInRange = top;
				}
			}

			if (!isTopClientType) {
				if (top->GetSpecialAbility(ALLOW_TO_TANK)){
					isTopClientType = true;
					topClientTypeInRange = top;
				}
			}

			if (!isTopClientType)
				return topClientTypeInRange ? topClientTypeInRange : nullptr;

			return top ? top : nullptr;
		}
		else {
			if (top == nullptr && skipped_count > 0) {
				return center->GetTarget() ? center->GetTarget() : nullptr;
			}
			return top ? top : nullptr;
		}
	}
	else{
		auto iterator = list.begin();
		int skipped_count = 0;
		while (iterator != list.end())
		{
			struct_HateList *cur = (*iterator);
			if (center->IsNPC() && center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if (!zone->watermap->InLiquid(cur->entity_on_hatelist->GetX(), cur->entity_on_hatelist->GetY(), cur->entity_on_hatelist->GetZ())) {
					skipped_count++;
					++iterator;
					continue;
				}
			}

			if (cur->entity_on_hatelist != nullptr && ((cur->stored_hate_amount > hate) || cur->is_entity_frenzy))
			{
				top = cur->entity_on_hatelist;
				hate = cur->stored_hate_amount;
			}
			++iterator;
		}
		if (top == nullptr && skipped_count > 0) {
			return center->GetTarget() ? center->GetTarget() : nullptr;
		}
		return top ? top : nullptr;
	}
	return nullptr;
}

Mob *HateList::GetEntWithMostHateInRange(){
	Mob* top = nullptr;
	int32 hate = -1;

	auto iterator = list.begin();
	while (iterator != list.end())
	{
		struct_HateList *cur = (*iterator);
		if (cur->entity_on_hatelist != nullptr && (cur->stored_hate_amount > hate))
		{
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
	if (count == 0) //If we don't have any entries it'll crash getting a random 0, -1 position.
		return NULL;

	if (count == 1) //No need to do all that extra work if we only have one hate entry
	{
		if (*list.begin()) // Just in case tHateEntry is invalidated somehow...
			return (*list.begin())->entity_on_hatelist;

		return NULL;
	}

	auto iterator = list.begin();
	int random = zone->random.Int(0, count - 1);
	for (int i = 0; i < random; i++)
		++iterator;

	return (*iterator)->entity_on_hatelist;
}

int32 HateList::GetEntHateAmount(Mob *ent, bool damage)
{
	struct_HateList *p;

	p = Find(ent);

	if (p && damage)
		return p->hatelist_damage;
	else if (p)
		return p->stored_hate_amount;
	else
		return 0;
}

//looking for any mob with hate > -1
bool HateList::IsHateListEmpty() {
	return(list.size() == 0);
}

// Prints hate list to a client
void HateList::PrintHateListToClient(Client *c)
{
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		struct_HateList *e = (*iterator);
		c->Message(0, "- name: %s, damage: %d, hate: %d",
			(e->entity_on_hatelist && e->entity_on_hatelist->GetName()) ? e->entity_on_hatelist->GetName() : "(null)",
			e->hatelist_damage, e->stored_hate_amount);

		++iterator;
	}
}

int HateList::AreaRampage(Mob *caster, Mob *target, int count, ExtraAttackOptions *opts)
{
	if (!target || !caster)
		return 0;

	int ret = 0;
	std::list<uint32> id_list;
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		struct_HateList *h = (*iterator);
		++iterator;
		if (h && h->entity_on_hatelist && h->entity_on_hatelist != caster)
		{
			if (caster->CombatRange(h->entity_on_hatelist))
			{
				id_list.push_back(h->entity_on_hatelist->GetID());
				++ret;
			}
		}
	}

	std::list<uint32>::iterator iter = id_list.begin();
	while (iter != id_list.end())
	{
		Mob *cur = entity_list.GetMobID((*iter));
		if (cur)
		{
			for (int i = 0; i < count; ++i) {
				caster->Attack(cur, MainPrimary, false, false, false, opts);
			}
		}
		iter++;
	}

	return ret;
}

void HateList::SpellCast(Mob *caster, uint32 spell_id, float range, Mob* ae_center)
{
	if (!caster)
		return;

	Mob* center = caster;

	if (ae_center)
		center = ae_center;

	//this is slower than just iterating through the list but avoids
	//crashes when people kick the bucket in the middle of this call
	//that invalidates our iterator but there's no way to know sadly
	//So keep a list of entity ids and look up after
	std::list<uint32> id_list;
	range = range * range;
	float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;
	float dist_targ = 0;
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		struct_HateList *h = (*iterator);
		if (range > 0)
		{
			dist_targ = center->DistNoRoot(*h->entity_on_hatelist);
			if (dist_targ <= range && dist_targ >= min_range2)
			{
				id_list.push_back(h->entity_on_hatelist->GetID());
				h->entity_on_hatelist->CalcSpellPowerDistanceMod(spell_id, dist_targ);
			}
		}
		else
		{
			id_list.push_back(h->entity_on_hatelist->GetID());
			h->entity_on_hatelist->CalcSpellPowerDistanceMod(spell_id, 0, caster);
		}
		++iterator;
	}

	std::list<uint32>::iterator iter = id_list.begin();
	while (iter != id_list.end())
	{
		Mob *cur = entity_list.GetMobID((*iter));
		if (cur)
		{
			caster->SpellOnTarget(spell_id, cur);
		}
		iter++;
	}
}
