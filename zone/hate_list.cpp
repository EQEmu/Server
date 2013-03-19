/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

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

#include "../common/debug.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "masterentity.h"
#include "../common/linked_list.h"
#include "../common/rulesys.h"
#include "../common/MiscFunctions.h"
#include "hate_list.h"
#include "QuestParserCollection.h"
#include "zone.h"
#include "watermap.h"

extern Zone *zone;

HateList::HateList()
{
	owner = NULL;
}

HateList::~HateList()
{
}

// neotokyo: added for frenzy support
// checks if target still is in frenzy mode
void HateList::CheckFrenzyHate()
{
    LinkedListIterator<tHateEntry*> iterator(list);
    iterator.Reset();
	while(iterator.MoreElements())
    {
        if (iterator.GetData()->ent->GetHPRatio() >= 20)
            iterator.GetData()->bFrenzy = false;
        iterator.Advance();
    }
}

void HateList::Wipe()
{
	LinkedListIterator<tHateEntry*> iterator(list);
	iterator.Reset();

	while(iterator.MoreElements()) 
	{
		Mob* m = iterator.GetData()->ent;
        parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), m, "0", 0); 
       	iterator.RemoveCurrent();

		if(m->IsClient())
			m->CastToClient()->DecrementAggroCount();
	}
}

bool HateList::IsOnHateList(Mob *mob)    
{    
	if (Find(mob))    
		return true;    
	return false;    
} 

tHateEntry *HateList::Find(Mob *ent)
{
	_ZP(HateList_Find);
    LinkedListIterator<tHateEntry*> iterator(list);
    iterator.Reset();
	while(iterator.MoreElements())
    {
        if(iterator.GetData()->ent == ent)
            return iterator.GetData();
        iterator.Advance();
    }
	return NULL;
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
	_ZP(HateList_GetDamageTop);
	Mob* current = NULL;
	Group* grp = NULL;
	Raid* r = NULL;
	uint32 dmg_amt = 0;

    LinkedListIterator<tHateEntry*> iterator(list);
    iterator.Reset();
	while(iterator.MoreElements())
    {
		grp = NULL;
		r = NULL;

		if(iterator.GetData()->ent && iterator.GetData()->ent->IsClient()){
			r = entity_list.GetRaidByClient(iterator.GetData()->ent->CastToClient());
		}

        grp = entity_list.GetGroupByMob(iterator.GetData()->ent);

		if(iterator.GetData()->ent && r){
			if(r->GetTotalRaidDamage(hater) >= dmg_amt)
			{
				current = iterator.GetData()->ent;
				dmg_amt = r->GetTotalRaidDamage(hater);
			}
		}
		else if (iterator.GetData()->ent != NULL && grp != NULL)
        {
			if (grp->GetTotalGroupDamage(hater) >= dmg_amt)
            {
				current = iterator.GetData()->ent;
				dmg_amt = grp->GetTotalGroupDamage(hater);
            }
        }
        else if (iterator.GetData()->ent != NULL && (uint32)iterator.GetData()->damage >= dmg_amt)
        {
			current = iterator.GetData()->ent;
			dmg_amt = iterator.GetData()->damage;
        }
        iterator.Advance();
    }
	return current;
}

Mob* HateList::GetClosest(Mob *hater) {
	_ZP(HateList_GetClosest);
	Mob* close = NULL;
	float closedist = 99999.9f;
	float thisdist;
	
    LinkedListIterator<tHateEntry*> iterator(list);
    iterator.Reset();
	while(iterator.MoreElements()) {
		thisdist = iterator.GetData()->ent->DistNoRootNoZ(*hater);
		if(iterator.GetData()->ent != NULL && thisdist <= closedist) {
			closedist = thisdist;
			close = iterator.GetData()->ent;
		}
        iterator.Advance();
	}
	
	if (close == 0 && hater->IsNPC())
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
        list.Append(p);
		parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), ent, "1", 0); 

		if(ent->IsClient())
			ent->CastToClient()->IncrementAggroCount();
    }
}

bool HateList::RemoveEnt(Mob *ent)
{
	bool found = false;
	LinkedListIterator<tHateEntry*> iterator(list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		if(iterator.GetData()->ent == ent)
		{
            parse->EventNPC(EVENT_HATE_LIST, owner->CastToNPC(), ent, "0", 0); 
			iterator.RemoveCurrent();
			found = true;

			if(ent->IsClient())
				ent->CastToClient()->DecrementAggroCount();

        	}
		else
			iterator.Advance();
	}
	return found;
}

void HateList::DoFactionHits(int32 nfl_id) {
	_ZP(HateList_DoFactionHits);
	if (nfl_id <= 0)
		return;
    LinkedListIterator<tHateEntry*> iterator(list);
    iterator.Reset();
	while(iterator.MoreElements())
    {
        Client *p;

        if (iterator.GetData()->ent && iterator.GetData()->ent->IsClient())
            p = iterator.GetData()->ent->CastToClient();
        else
            p = NULL;

        if (p)
			p->SetFactionLevel(p->CharacterID(), nfl_id, p->GetBaseClass(), p->GetBaseRace(), p->GetDeity());
        iterator.Advance();
    }
}

Mob *HateList::GetTop(Mob *center)
{
	_ZP(HateList_GetTop);
	Mob* top = NULL;
	int32 hate = -1;
	
	if (RuleB(Aggro,SmartAggroList)){
		Mob* topClientTypeInRange = NULL;
		int32 hateClientTypeInRange = -1;
        int skipped_count = 0;

		LinkedListIterator<tHateEntry*> iterator(list);
		iterator.Reset();
		while(iterator.MoreElements())
		{
    		tHateEntry *cur = iterator.GetData();
			int16 aggroMod = 0;

			if(!cur){
				iterator.Advance();
				continue;
			}			

			if(!cur->ent){
				iterator.Advance();
				continue;
			}

            if(center->IsNPC() && center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
                if(!zone->watermap->InLiquid(cur->ent->GetX(), cur->ent->GetY(), cur->ent->GetZ())) {
                    skipped_count++;
                    iterator.Advance();
                    continue;
                }
            }

			if(cur->ent->DivineAura() || cur->ent->IsMezzed() || cur->ent->IsFeared()){
				if(hate == -1)
				{
					top = cur->ent;
					hate = 0;
				}
				iterator.Advance();
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

			iterator.Advance();
		}

		if(topClientTypeInRange != NULL && top != NULL) {
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
				return topClientTypeInRange;

			return top;
        }
		else {
			if(top == NULL && skipped_count > 0) {
                return center->GetTarget();
            }
			return top;
        }
	}
	else{
		LinkedListIterator<tHateEntry*> iterator(list);
		iterator.Reset();
        int skipped_count = 0;
		while(iterator.MoreElements())
		{
    		tHateEntry *cur = iterator.GetData();
            if(center->IsNPC() && center->CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
                if(!zone->watermap->InLiquid(cur->ent->GetX(), cur->ent->GetY(), cur->ent->GetZ())) {
                    skipped_count++;
                    iterator.Advance();
                    continue;
                }
            }

			if(cur->ent != NULL && ((cur->hate > hate) || cur->bFrenzy ))
			{
				top = cur->ent;
				hate = cur->hate;
			}
			iterator.Advance();
		}
		if(top == NULL && skipped_count > 0) {
            return center->GetTarget();
        }
		return top;
	}
}

Mob *HateList::GetMostHate(){
	_ZP(HateList_GetMostHate);

	Mob* top = NULL;
	int32 hate = -1;

	LinkedListIterator<tHateEntry*> iterator(list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		tHateEntry *cur = iterator.GetData();
		if(cur->ent != NULL && (cur->hate > hate))
		{
			top = cur->ent;
			hate = cur->hate;
		}
		iterator.Advance();
	}
	return top;
}


Mob *HateList::GetRandom()
{
    int count = 0;
    LinkedListIterator<tHateEntry*> iterator(list);
    iterator.Reset();
	while(iterator.MoreElements())
    {
        iterator.Advance();
        count++;
    }
	if(!count)
		return NULL;

    int random = MakeRandomInt(0, count-1);
    iterator.Reset();
    for (int i = 0; i < random; i++)
        iterator.Advance();
    return iterator.GetData()->ent;
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
	_ZP(HateList_IsEmpty);
	
	return(list.Count() == 0);
}

// Prints hate list to a client
void HateList::PrintToClient(Client *c)
{
	LinkedListIterator<tHateEntry*> iterator(list);
	iterator.Reset();
	while (iterator.MoreElements())
	{
		tHateEntry *e = iterator.GetData();
		c->Message(0, "- name: %s, damage: %d, hate: %d",
			(e->ent && e->ent->GetName()) ? e->ent->GetName() : "(null)",
			e->damage, e->hate);

		iterator.Advance();
	}
}

int HateList::AreaRampage(Mob *caster, Mob *target)
{
	if(!target || !caster)
		return 0;

	int ret = 0;
	std::list<uint32> id_list;
	LinkedListIterator<tHateEntry*> iterator(list);
	iterator.Reset();
	while (iterator.MoreElements())
	{
		tHateEntry *h = iterator.GetData();
		iterator.Advance();		
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
			caster->Attack(cur);
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
	LinkedListIterator<tHateEntry*> iterator(list);
	iterator.Reset();
	while (iterator.MoreElements())
	{
		tHateEntry *h = iterator.GetData();
		if(range > 0)
		{
			if(caster->DistNoRoot(*h->ent) <= range)
			{
				id_list.push_back(h->ent->GetID());
			}
		}
		else
		{
			id_list.push_back(h->ent->GetID());
		}
		iterator.Advance();		
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


void HateList::GetHateList(std::list<tHateEntry*> &h_list)
{
	h_list.clear();
	LinkedListIterator<tHateEntry*> iterator(list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		tHateEntry *ent = iterator.GetData();
		h_list.push_back(ent);
		iterator.Advance();
	}
}
