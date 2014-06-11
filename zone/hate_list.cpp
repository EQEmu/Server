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
#include "../common/MiscFunctions.h"
#include "hate_list.h"
#include "QuestParserCollection.h"
#include "zone.h"
#include "water_map.h"

extern Zone *zone;

HateList::HateList() : mOwner(nullptr) { }
HateList::~HateList() { }


void HateList::clear()
{
	auto i = mEntries.begin();
	while(i != mEntries.end())
	{
		Mob* m = (*i)->mMOB;
		if(m)
		{
			parse->EventNPC(EVENT_HATE_LIST, mOwner->CastToNPC(), m, "0", 0);

			if(m->IsClient())
				m->CastToClient()->DecrementAggroCount();
		}
		delete (*i);
		i = mEntries.erase(i);
	}
}

bool HateList::isHated(Mob* pMOB)
{
	return find(pMOB) != nullptr;
}

HateEntry* HateList::find(Mob* pMOB)
{
	for (auto i = mEntries.begin(); i != mEntries.end(); i++)
		if ((*i)->mMOB == pMOB) return *i;
	return nullptr;
}

void HateList::set(Mob* pMob, uint32 pHate, uint32 pDamage)
{
	HateEntry *entry = find(pMob);
	if (entry)
	{
		if(pDamage > 0)
			entry->mDamage = pDamage;
		if(pHate > 0)
			entry->mHate = pHate;
	}
}

Mob* HateList::getHighestDamage(Mob* hater)
{
	/*
	This is called in NPC::Death
	It calls this on it's own HateList, passing itself as the hater parameter.
	-- Under the circumstances where there were 40 people in a raid it would calculate the total raid damage 80 times. 
	*/
	Mob* current = nullptr;
	Group* grp = nullptr;
	Raid* r = nullptr;
	uint32 dmg_amt = 0;

	auto iterator = mEntries.begin();
	while(iterator != mEntries.end())
	{
		grp = nullptr;
		r = nullptr;

		if((*iterator)->mMOB && (*iterator)->mMOB->IsClient()){
			r = entity_list.GetRaidByClient((*iterator)->mMOB->CastToClient());
		}

		grp = entity_list.GetGroupByMob((*iterator)->mMOB);

		if((*iterator)->mMOB && r){
			if(r->GetTotalRaidDamage(hater) >= dmg_amt)
			{
				current = (*iterator)->mMOB;
				dmg_amt = r->GetTotalRaidDamage(hater);
			}
		}
		else if ((*iterator)->mMOB != nullptr && grp != nullptr)
		{
			if (grp->GetTotalGroupDamage(hater) >= dmg_amt)
			{
				current = (*iterator)->mMOB;
				dmg_amt = grp->GetTotalGroupDamage(hater);
			}
		}
		else if ((*iterator)->mMOB != nullptr && (uint32)(*iterator)->mDamage >= dmg_amt)
		{
			current = (*iterator)->mMOB;
			dmg_amt = (*iterator)->mDamage;
		}
		++iterator;
	}
	return current;
}

Mob* HateList::getClosest(Mob *hater) {
	Mob* close = nullptr;
	float closedist = 99999.9f;
	float thisdist;

	auto iterator = mEntries.begin();
	while(iterator != mEntries.end()) {
		thisdist = (*iterator)->mMOB->DistNoRootNoZ(*hater);
		if((*iterator)->mMOB != nullptr && thisdist <= closedist) {
			closedist = thisdist;
			close = (*iterator)->mMOB;
		}
		++iterator;
	}

	if (close == 0 && hater->IsNPC() || close->DivineAura())
		close = hater->CastToNPC()->GetHateTop();

	return close;
}

void HateList::add(Mob* pMOB, int32 pHate, int32 pDamage, bool pFrenzy, bool iAddIfNotExist)
{
	if (!pMOB) return;

	// Do not add corpses to HateList.
	if (pMOB->IsCorpse()) return;

	// Do not add dead dead players to HateList.
	if (pMOB->IsClient() && pMOB->CastToClient()->IsDead()) return;

	// Where pMOB is already on the HateList, increase Hate/Damage values.
	HateEntry *entry = find(pMOB);
	if (entry)
	{
		entry->mDamage += (pDamage >= 0) ? pDamage : 0;
		entry->mHate += pHate;
		entry->mFrenzy = pFrenzy;
	}
	// Where pMOB is not on the HateList, add it.
	else if (iAddIfNotExist) {
		entry = new HateEntry(pMOB, (pDamage >= 0) ? pDamage : 0, pHate, pFrenzy);
		mEntries.push_back(entry);
		parse->EventNPC(EVENT_HATE_LIST, mOwner->CastToNPC(), pMOB, "1", 0);

		// Where pMOB is client, update 'Aggro Count'.
		if (pMOB->IsClient())
			pMOB->CastToClient()->IncrementAggroCount();
	}
}

bool HateList::clear(Mob* pMOB)
{
	if (!pMOB) return false;

	for (auto i = mEntries.begin(); i != mEntries.end(); i++) {
		if ((*i)->mMOB == pMOB) {
			parse->EventNPC(EVENT_HATE_LIST, mOwner->CastToNPC(), pMOB, "0", 0);
			pMOB->CastToClient()->DecrementAggroCount();

			// Clean up.
			delete *i;
			mEntries.erase(i);
			return true;
		}
	}

	return false;
}

void HateList::DoFactionHits(int32 nfl_id) {
	if (nfl_id <= 0)
		return;
	auto iterator = mEntries.begin();
	while(iterator != mEntries.end())
	{
		Client *p;

		if ((*iterator)->mMOB && (*iterator)->mMOB->IsClient())
			p = (*iterator)->mMOB->CastToClient();
		else
			p = nullptr;

		if (p)
			p->SetFactionLevel(p->CharacterID(), nfl_id, p->GetBaseClass(), p->GetBaseRace(), p->GetDeity());
		++iterator;
	}
}

int HateList::getSummonedPetCount() {
	int count = 0;
	for (auto i = mEntries.begin(); i != mEntries.end(); i++) {
		Mob* entryMOB = (*i)->mMOB;
		if (entryMOB && entryMOB->IsNPC() && (entryMOB->CastToNPC()->IsPet() || (entryMOB->CastToNPC()->GetSwarmOwner() > 0)))
			count++;
	}

	return count;
}

Mob *HateList::getHighestHate(Mob *center)
{
	Mob* top = nullptr;
	int32 hate = -1;

	if(center == nullptr)
		return nullptr;

	if (RuleB(Aggro,SmartAggroList)){
		Mob* topClientTypeInRange = nullptr;
		int32 hateClientTypeInRange = -1;
		int skipped_count = 0;

		auto iterator = mEntries.begin();
		while(iterator != mEntries.end())
		{
			HateEntry *cur = (*iterator);
			int16 aggroMod = 0;

			if(!cur){
				++iterator;
				continue;
			}

			if(!cur->mMOB){
				++iterator;
				continue;
			}

			if(center->IsNPC() && center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if(!zone->watermap->InLiquid(cur->mMOB->GetX(), cur->mMOB->GetY(), cur->mMOB->GetZ())) {
					skipped_count++;
					++iterator;
					continue;
				}
			}

			if(cur->mMOB->DivineAura() || cur->mMOB->IsMezzed() || cur->mMOB->IsFeared()){
				if(hate == -1)
				{
					top = cur->mMOB;
					hate = 0;
				}
				++iterator;
				continue;
			}

			int32 currentHate = cur->mHate;

			if(cur->mMOB->IsClient()){

				if(cur->mMOB->CastToClient()->IsSitting()){
					aggroMod += RuleI(Aggro, SittingAggroMod);
				}

				if(center){
					if(center->GetTarget() == cur->mMOB)
						aggroMod += RuleI(Aggro, CurrentTargetAggroMod);
					if(RuleI(Aggro, MeleeRangeAggroMod) != 0)
					{
						if(center->CombatRange(cur->mMOB)){
							aggroMod += RuleI(Aggro, MeleeRangeAggroMod);

							if(currentHate > hateClientTypeInRange || cur->mFrenzy){
								hateClientTypeInRange = currentHate;
								topClientTypeInRange = cur->mMOB;
							}
						}
					}
				}

			}
			else{
				if(center){
					if(center->GetTarget() == cur->mMOB)
						aggroMod += RuleI(Aggro, CurrentTargetAggroMod);
					if(RuleI(Aggro, MeleeRangeAggroMod) != 0)
					{
						if(center->CombatRange(cur->mMOB)){
							aggroMod += RuleI(Aggro, MeleeRangeAggroMod);
						}
					}
				}
			}

			if(cur->mMOB->GetMaxHP() != 0 && ((cur->mMOB->GetHP()*100/cur->mMOB->GetMaxHP()) < 20)){
				aggroMod += RuleI(Aggro, CriticallyWoundedAggroMod);
			}

			if(aggroMod){
				currentHate += (currentHate * aggroMod / 100);
			}

			if(currentHate > hate || cur->mFrenzy){
				hate = currentHate;
				top = cur->mMOB;
			}

			++iterator;
		}

		if(topClientTypeInRange != nullptr && top != nullptr) {
			bool isTopClientType = top->IsClient();

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
		auto iterator = mEntries.begin();
		int skipped_count = 0;
		while(iterator != mEntries.end())
		{
			HateEntry *cur = (*iterator);
			if(center->IsNPC() && center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if(!zone->watermap->InLiquid(cur->mMOB->GetX(), cur->mMOB->GetY(), cur->mMOB->GetZ())) {
					skipped_count++;
					++iterator;
					continue;
				}
			}

			if(cur->mMOB != nullptr && ((cur->mHate > hate) || cur->mFrenzy ))
			{
				top = cur->mMOB;
				hate = cur->mHate;
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

Mob *HateList::getMostHate(){
	Mob* top = nullptr;
	int32 hate = -1;

	auto iterator = mEntries.begin();
	while(iterator != mEntries.end())
	{
		HateEntry *cur = (*iterator);
		if(cur->mMOB != nullptr && (cur->mHate > hate))
		{
			top = cur->mMOB;
			hate = cur->mHate;
		}
		++iterator;
	}
	return top;
}


Mob *HateList::getRandom()
{
	int count = mEntries.size();
	if(count == 0) //If we don't have any entries it'll crash getting a random 0, -1 position.
		return NULL;

	if(count == 1) //No need to do all that extra work if we only have one hate entry
	{
		if(*mEntries.begin()) // Just in case tHateEntry is invalidated somehow...
			return (*mEntries.begin())->mMOB;

		return NULL;
	}

	auto iterator = mEntries.begin();
	int random = MakeRandomInt(0, count - 1);
	for (int i = 0; i < random; i++)
		++iterator;
	
	return (*iterator)->mMOB;
}

int32 HateList::getHate(Mob* pMOB, bool pDamage)
{
	HateEntry* entry = find(pMOB);
	if (entry) {
		if (pDamage) return entry->mDamage;
		return entry->mHate;
	}

	return 0;
}


int32 HateList::getDamage(Mob* pMOB)
{
	HateEntry* entry = find(pMOB);
	if (entry) return entry->mDamage;

	return 0;
}

bool HateList::isEmpty() {
	return mEntries.size() == 0;
}

// Prints hate list to a client
void HateList::PrintToClient(Client *c)
{
	auto iterator = mEntries.begin();
	while (iterator != mEntries.end())
	{
		HateEntry *e = (*iterator);
		c->Message(0, "- name: %s, damage: %d, hate: %d",
			(e->mMOB && e->mMOB->GetName()) ? e->mMOB->GetName() : "(null)",
			e->mDamage, e->mHate);

		++iterator;
	}
}

int HateList::AreaRampage(Mob *caster, Mob *target, int count, ExtraAttackOptions *opts)
{
	if(!target || !caster)
		return 0;

	int ret = 0;
	std::list<uint32> id_list;
	auto iterator = mEntries.begin();
	while (iterator != mEntries.end())
	{
		HateEntry *h = (*iterator);
		++iterator;
		if(h && h->mMOB && h->mMOB != caster)
		{
			if(caster->CombatRange(h->mMOB))
			{
				id_list.push_back(h->mMOB->GetID());
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
				caster->Attack(cur, 13, false, false, false, opts);
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
	auto iterator = mEntries.begin();
	while (iterator != mEntries.end())
	{
		HateEntry *h = (*iterator);
		if(range > 0)
		{
			if(caster->DistNoRoot(*h->mMOB) <= range)
			{
				id_list.push_back(h->mMOB->GetID());
			}
		}
		else
		{
			id_list.push_back(h->mMOB->GetID());
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

