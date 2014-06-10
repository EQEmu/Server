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

#ifndef HATELIST_H
#define HATELIST_H

class Mob;
// TODO: Later fix pre-declarations. There is some crazy shit going on with include order in other files obviously.

struct HateEntry
{
	HateEntry(Mob* pMOB, int32 pDamage, int32 pHate, bool pFrenzy) : mMOB(pMOB), mDamage(pDamage), mHate(pHate), mFrenzy(pFrenzy) {}
	Mob *mMOB;
	int32 mDamage;
	int32 mHate;
	bool mFrenzy;
};

class HateList
{
public:
	HateList();
	~HateList();

	// Set the owner of the HateList.
	void setOwner(Mob* pOwner) { mOwner = pOwner; }

	// Returns whether the HateList is empty.
	bool isEmpty();

	// Returns the number of pets on the HateList.
	int getSummonedPetCount();

	// Removes a specific MOB from the HateList.
	bool clear(Mob* pMOB);

	// Removes all MOBs from the HateList.
	void clear();

	// Adds a MOB to the HateList.
	// TODO: Look more into parameter 'iAddIfNotExist' .. I can't think of circumstances where this would be needed.
	void add(Mob* pMOB, int32 pHate = 0, int32 pDamage = 0, bool pFrenzy = false, bool iAddIfNotExist = true);

	// Sets a MOBs hate.
	void set(Mob* pMOB, uint32 pHate, uint32 pDamage);

	// Returns the hate value of a specific MOB. (Or Damage for fun under some circumstances...)
	// TODO: Remove the 'damage' parameter and use getDamage where appropriate.
	int32 getHate(Mob * pMOB, bool damage = false);

	// Returns the damage value of a specific MOB.
	int32 getDamage(Mob* pMOB);

	// Returns the MOB with the highest hate value or null if none.
	Mob *getHighestHate(Mob* pCenter);

	// Returns the MOB with the highest damage value or null if none.
	Mob *getHighestDamage(Mob* pHater);

	// Returns a random MOB on the HateList.
	Mob* getRandom();

	// Returns the closest MOB.
	Mob* getClosest(Mob* pHater);

	// Returns whether the specific MOB is on the HateList.
	bool isHated(Mob * pMOB);

	//Gets the target with the most hate regardless of things like frenzy etc.
	Mob* getMostHate();

	//For accessing the hate list via perl; don't use for anything else
	std::list<HateEntry*>& GetHateList() { return mEntries; }

	// TODO: Remove, this functionality does not belong here.
	void DoFactionHits(int32 nfl_id);
	// TODO: Remove, this functionality does not belong here.
	int AreaRampage(Mob *caster, Mob *target, int count, ExtraAttackOptions *opts);
	// TODO: Remove, this functionality does not belong here.
	void SpellCast(Mob *caster, uint32 spell_id, float range);
	// TODO: Remove, this functionality does not belong here.
	void PrintToClient(Client *c);
private:
	HateEntry* find(Mob* pMOB);
	std::list<HateEntry*> mEntries;
	Mob* mOwner;
};

#endif
