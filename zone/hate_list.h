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

class tHateEntry
{
public:
	Mob *ent;
	int32 damage, hate;
	bool bFrenzy;
};

class HateList
{
public:
	HateList();
	~HateList();

	// adds a mob to the hatelist
	void Add(Mob *ent, int32 in_hate=0, int32 in_dam=0, bool bFrenzy = false, bool iAddIfNotExist = true);
	// sets existing hate
	void Set(Mob *other, uint32 in_hate, uint32 in_dam);
	// removes mobs from hatelist
	bool RemoveEnt(Mob *ent);
	// Remove all
	void Wipe();
	// ???
	void DoFactionHits(int32 nfl_id);
	// Gets Hate amount for mob
	int32 GetEntHate(Mob *ent, bool damage = false);
	// gets top hated mob
	Mob *GetTop(Mob *center);
	// gets any on the list
	Mob *GetRandom();
	// get closest mob or nullptr if list empty
	Mob *GetClosest(Mob *hater);
	// gets top mob or nullptr if hate list empty
	Mob *GetDamageTop(Mob *hater);
	// used to check if mob is on hatelist
	bool IsOnHateList(Mob *);
	// used to remove or add frenzy hate
	void CheckFrenzyHate();
	//Gets the target with the most hate regardless of things like frenzy etc.
	Mob* GetMostHate();

	int AreaRampage(Mob *caster, Mob *target);

	void SpellCast(Mob *caster, uint32 spell_id, float range);

	bool IsEmpty();
	void PrintToClient(Client *c);

	//For accessing the hate list via perl; don't use for anything else
	void GetHateList(std::list<tHateEntry*> &h_list);

	//setting owner
	void SetOwner(Mob *newOwner) { owner = newOwner; }

protected:
	tHateEntry *Find(Mob *ent);
private:
	LinkedList<tHateEntry*> list;
	Mob *owner;
};

#endif
