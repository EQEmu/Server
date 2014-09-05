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

#include "../common/debug.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <list>
#include "masterentity.h"
#include "../common/rulesys.h"
#include "../common/misc_functions.h"
#include "hate_list.h"
#include "quest_parser_collection.h"
#include "zone.h"
#include "water_map.h"

extern Zone *zone;

HateList::HateList()
{
	owner = nullptr;
}

HateList::~HateList()
{
}

// neotokyo: added for frenzy support
// checks if target still is in frenzy mode
void HateList::CheckFrenzyHate()
{
	auto iterator = list.begin();
	while(iterator != list.end())
	{
		if ((*iterator)->ent->GetHPRatio() >= 20)
			(*iterator)->bFrenzy = false;
		++iterator;
	}
}

void HateList::Wipe()
{
	auto iterator = list.begin();

	while(iterator != list.end())
	{
		Mob* m = (*iterator)->ent;
		if(m)
		{
			parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), m, "0", 0);

			if(m->IsClient())
				m->CastToClient()->DecrementAggroCount();
		}
		delete (*iterator);
		iterator = list.erase(iterator);

	}
}

bool HateList::IsOnHateList(Mob *mob)
{
	if(Find(mob))
		return true;
	return false;
}

tHateEntry *HateList::Find(Mob *ent)
{
	auto iterator = list.begin();
	while(iterator != list.end())
	{
		if((*iterator)->ent == ent)
			return (*iterator);
		++iterator;
	}
	return nullptr;
}

void HateList::Set(Mob* other, uint32 in_hate, uint32 in_dam)
{
	tHateEntry *p = Find(other);
	if(p)
	{
		if(in_dam > 0)
			p->damage = in_dam;
		if(in_hate > 0)
			p->hate = in_hate;
	}
}

Mob* HateList::GetDamageTop(Mob* hater)
{
	Mob* current = nullptr;
	Group* grp = nullptr;
	Raid* r = nullptr;
	uint32 dmg_amt = 0;

	auto iterator = list.begin();
	while(iterator != list.end())
	{
		grp = nullptr;
		r = nullptr;

		if((*iterator)->ent && (*iterator)->ent->IsClient()){
			r = entity_list.GetRaidByClient((*iterator)->ent->CastToClient());
		}

		grp = entity_list.GetGroupByMob((*iterator)->ent);

		if((*iterator)->ent && r){
			if(r->GetTotalRaidDamage(hater) >= dmg_amt)
			{
				current = (*iterator)->ent;
				dmg_amt = r->GetTotalRaidDamage(hater);
			}
		}
		else if ((*iterator)->ent != nullptr && grp != nullptr)
		{
			if (grp->GetTotalGroupDamage(hater) >= dmg_amt)
			{
				current = (*iterator)->ent;
				dmg_amt = grp->GetTotalGroupDamage(hater);
			}
		}
		else if ((*iterator)->ent != nullptr && (uint32)(*iterator)->damage >= dmg_amt)
		{
			current = (*iterator)->ent;
			dmg_amt = (*iterator)->damage;
		}
		++iterator;
	}
	return current;
}

Mob* HateList::GetClosest(Mob *hater) {
	Mob* close = nullptr;
	float closedist = 99999.9f;
	float thisdist;

	auto iterator = list.begin();
	while(iterator != list.end()) {
		thisdist = (*iterator)->ent->DistNoRootNoZ(*hater);
		if((*iterator)->ent != nullptr && thisdist <= closedist) {
			closedist = thisdist;
			close = (*iterator)->ent;
		}
		++iterator;
	}

	if ((!close && hater->IsNPC()) || (close && close->DivineAura()))
		close = hater->CastToNPC()->GetHateTop();

	return close;
}


// neotokyo: a few comments added, rearranged code for readability
void HateList::Add(Mob *ent, int32 in_hate, int32 in_dam, bool bFrenzy, bool iAddIfNotExist)
{
	if(!ent)
		return;

	if(ent->IsCorpse())
		return;

	if(ent->IsClient() && ent->CastToClient()->IsDead())
		return;

	tHateEntry *p = Find(ent);
	if (p)
	{
		p->damage+=(in_dam>=0)?in_dam:0;
		p->hate+=in_hate;
		p->bFrenzy = bFrenzy;
	}
	else if (iAddIfNotExist) {
		p = new tHateEntry;
		p->ent = ent;
		p->damage = (in_dam>=0)?in_dam:0;
		p->hate = in_hate;
		p->bFrenzy = bFrenzy;
		list.push_back(p);
		parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), ent, "1", 0);

		if (ent->IsClient()) {
			if (owner->CastToNPC()->IsRaidTarget()) 
				ent->CastToClient()->SetEngagedRaidTarget(true);
			ent->CastToClient()->IncrementAggroCount();
		}
	}
}

bool HateList::RemoveEnt(Mob *ent)
{
	if (!ent)
		return false;

	bool found = false;
	auto iterator = list.begin();

	while(iterator != list.end())
	{
		if((*iterator)->ent == ent)
		{
			if(ent)
			parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), ent, "0", 0);
			found = true;

			
			if(ent && ent->IsClient())
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
	while(iterator != list.end())
	{
		Client *p;

		if ((*iterator)->ent && (*iterator)->ent->IsClient())
			p = (*iterator)->ent->CastToClient();
		else
			p = nullptr;

		if (p)
			p->SetFactionLevel(p->CharacterID(), nfl_id, p->GetBaseClass(), p->GetBaseRace(), p->GetDeity());
		++iterator;
	}
}

int HateList::SummonedPetCount(Mob *hater) {

	//Function to get number of 'Summoned' pets on a targets hate list to allow calculations for certian spell effects.
	//Unclear from description that pets are required to be 'summoned body type'. Will not require at this time.
	int petcount = 0;
	auto iterator = list.begin();
	while(iterator != list.end()) {

		if((*iterator)->ent != nullptr && (*iterator)->ent->IsNPC() && 	((*iterator)->ent->CastToNPC()->IsPet() || ((*iterator)->ent->CastToNPC()->GetSwarmOwner() > 0))) 
		{
			++petcount;
		}
		
		++iterator;
	}

	return petcount;
}

Mob *HateList::GetTop(Mob *center)
{
	Mob* top = nullptr;
	int32 hate = -1;

	if(center == nullptr)
		return nullptr;

	if (RuleB(Aggro,SmartAggroList)){
		Mob* topClientTypeInRange = nullptr;
		int32 hateClientTypeInRange = -1;
		int skipped_count = 0;

		auto iterator = list.begin();
		while(iterator != list.end())
		{
			tHateEntry *cur = (*iterator);
			int16 aggroMod = 0;

			if(!cur){
				++iterator;
				continue;
			}

			if(!cur->ent){
				++iterator;
				continue;
			}

			if(center->IsNPC() && center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if(!zone->watermap->InLiquid(cur->ent->GetX(), cur->ent->GetY(), cur->ent->GetZ())) {
					skipped_count++;
					++iterator;
					continue;
				}
			}

			if (cur->ent->Sanctuary()) { 
				if(hate == -1)
				{
					top = cur->ent;
					hate = 1;
				}
				++iterator;
				continue;
			}

			if(cur->ent->DivineAura() || cur->ent->IsMezzed() || cur->ent->IsFeared()){
				if(hate == -1)
				{
					top = cur->ent;
					hate = 0;
				}
				++iterator;
				continue;
			}

			int32 currentHate = cur->hate;

			if(cur->ent->IsClient()){

				if(cur->ent->CastToClient()->IsSitting()){
					aggroMod += RuleI(Aggro, SittingAggroMod);
				}

				if(center){
					if(center->GetTarget() == cur->ent)
						aggroMod += RuleI(Aggro, CurrentTargetAggroMod);
					if(RuleI(Aggro, MeleeRangeAggroMod) != 0)
					{
						if(center->CombatRange(cur->ent)){
							aggroMod += RuleI(Aggro, MeleeRangeAggroMod);

							if(currentHate > hateClientTypeInRange || cur->bFrenzy){
								hateClientTypeInRange = currentHate;
								topClientTypeInRange = cur->ent;
							}
						}
					}
				}

			}
			else{
				if(center){
					if(center->GetTarget() == cur->ent)
						aggroMod += RuleI(Aggro, CurrentTargetAggroMod);
					if(RuleI(Aggro, MeleeRangeAggroMod) != 0)
					{
						if(center->CombatRange(cur->ent)){
							aggroMod += RuleI(Aggro, MeleeRangeAggroMod);
						}
					}
				}
			}

			if(cur->ent->GetMaxHP() != 0 && ((cur->ent->GetHP()*100/cur->ent->GetMaxHP()) < 20)){
				aggroMod += RuleI(Aggro, CriticallyWoundedAggroMod);
			}

			if(aggroMod){
				currentHate += (currentHate * aggroMod / 100);
			}

			if(currentHate > hate || cur->bFrenzy){
				hate = currentHate;
				top = cur->ent;
			}

			++iterator;
		}

		if(topClientTypeInRange != nullptr && top != nullptr) {
			bool isTopClientType = top->IsClient();
#ifdef BOTS
			if(!isTopClientType) {
				if(top->IsBot()) {
					isTopClientType = true;
					topClientTypeInRange = top;
				}
			}
#endif //BOTS

			if(!isTopClientType) {
				if(top->IsMerc()) {
					isTopClientType = true;
					topClientTypeInRange = top;
				}
			}

			if(!isTopClientType)
				return topClientTypeInRange ? topClientTypeInRange : nullptr;

			return top ? top : nullptr;
		}
		else {
			if(top == nullptr && skipped_count > 0) {
				return center->GetTarget() ? center->GetTarget() : nullptr;
			}
			return top ? top : nullptr;
		}
	}
	else{
		auto iterator = list.begin();
		int skipped_count = 0;
		while(iterator != list.end())
		{
			tHateEntry *cur = (*iterator);
			if(center->IsNPC() && center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if(!zone->watermap->InLiquid(cur->ent->GetX(), cur->ent->GetY(), cur->ent->GetZ())) {
					skipped_count++;
					++iterator;
					continue;
				}
			}

			if(cur->ent != nullptr && ((cur->hate > hate) || cur->bFrenzy ))
			{
				top = cur->ent;
				hate = cur->hate;
			}
			++iterator;
		}
		if(top == nullptr && skipped_count > 0) {
			return center->GetTarget() ? center->GetTarget() : nullptr;
		}
		return top ? top : nullptr;
	}
	return nullptr;
}

Mob *HateList::GetMostHate(){
	Mob* top = nullptr;
	int32 hate = -1;

	auto iterator = list.begin();
	while(iterator != list.end())
	{
		tHateEntry *cur = (*iterator);
		if(cur->ent != nullptr && (cur->hate > hate))
		{
			top = cur->ent;
			hate = cur->hate;
		}
		++iterator;
	}
	return top;
}


Mob *HateList::GetRandom()
{
	int count = list.size();
	if(count == 0) //If we don't have any entries it'll crash getting a random 0, -1 position.
		return NULL;

	if(count == 1) //No need to do all that extra work if we only have one hate entry
	{
		if(*list.begin()) // Just in case tHateEntry is invalidated somehow...
			return (*list.begin())->ent;

		return NULL;
	}

	auto iterator = list.begin();
	int random = MakeRandomInt(0, count - 1);
	for (int i = 0; i < random; i++)
		++iterator;
	
	return (*iterator)->ent;
}

int32 HateList::GetEntHate(Mob *ent, bool damage)
{
	tHateEntry *p;

	p = Find(ent);

	if ( p && damage)
		return p->damage;
	else if (p)
		return p->hate;
	else
		return 0;
}

//looking for any mob with hate > -1
bool HateList::IsEmpty() {
	return(list.size() == 0);
}

// Prints hate list to a client
void HateList::PrintToClient(Client *c)
{
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		tHateEntry *e = (*iterator);
		c->Message(0, "- name: %s, damage: %d, hate: %d",
			(e->ent && e->ent->GetName()) ? e->ent->GetName() : "(null)",
			e->damage, e->hate);

		++iterator;
	}
}

int HateList::AreaRampage(Mob *caster, Mob *target, int count, ExtraAttackOptions *opts)
{
	if(!target || !caster)
		return 0;

	int ret = 0;
	std::list<uint32> id_list;
	auto iterator = list.begin();
	while (iterator != list.end())
	{
		tHateEntry *h = (*iterator);
		++iterator;
		if(h && h->ent && h->ent != caster)
		{
			if(caster->CombatRange(h->ent))
			{
				id_list.push_back(h->ent->GetID());
				++ret;
			}
		}
	}

	std::list<uint32>::iterator iter = id_list.begin();
	while(iter != id_list.end())
	{
		Mob *cur = entity_list.GetMobID((*iter));
		if(cur)
		{
			for(int i = 0; i < count; ++i) {
				caster->Attack(cur, MainPrimary, false, false, false, opts);
			}
		}
		iter++;
	}

	return ret;
}

void HateList::SpellCast(Mob *caster, uint32 spell_id, float range)
{
	if(!caster)
	{
		return;
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
	while (iterator != list.end())
	{
		tHateEntry *h = (*iterator);
		if(range > 0)
		{
			dist_targ = caster->DistNoRoot(*h->ent);
			if(dist_targ <= range && dist_targ >= min_range2)
			{
				id_list.push_back(h->ent->GetID());
				h->ent->CalcSpellPowerDistanceMod(spell_id, dist_targ);
			}
		}
		else
		{
			id_list.push_back(h->ent->GetID());
			h->ent->CalcSpellPowerDistanceMod(spell_id, 0, caster);
		}
		++iterator;
	}

	std::list<uint32>::iterator iter = id_list.begin();
	while(iter != id_list.end())
	{
		Mob *cur = entity_list.GetMobID((*iter));
		if(cur)
		{
			caster->SpellOnTarget(spell_id, cur);
		}
		iter++;
	}
}

