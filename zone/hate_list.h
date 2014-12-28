/*	 EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2015 EQEMu Development Team (http://eqemu.org)

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

class Client;
class Group;
class Mob;
class Raid;
struct ExtraAttackOptions;

struct struct_HateList {
	Mob *entity_on_hatelist;
	uint32 hatelist_damage;
	uint32 stored_hate_amount;
	bool is_entity_frenzy;
};

class HateList
{
	public:
		HateList();
		~HateList();

		Mob *GetClosestEntOnHateList(Mob *hater);
		Mob *GetDamageTopOnHateList(Mob *hater);
		Mob *GetRandomEntOnHateList();
		Mob *GetEntWithMostHateInRange(Mob *center);
		Mob* GetEntWithMostHateOnList();

		bool IsEntOnHateList(Mob *);
		bool IsHateListEmpty();
		bool RemoveEntFromHateList(Mob *ent);

		int AreaRampage(Mob *caster, Mob *target, int count, ExtraAttackOptions *opts);
		int GetSummonedPetCountOnHateList(Mob *hater);

		uint32 GetEntHateAmount(Mob *in_entity, bool damage = false);

		void AddEntToHateList(Mob *in_entity, int32 in_hate = 0, int32 in_damage = 0, bool in_is_frenzied = false, bool add_to_hate_list_if_not_exist = true);
		void DoFactionHits(int32 npc_faction_level_id);
		void IsEntityInFrenzyMode();
		void PrintHateListToClient(Client *c);
		void SetHateAmountOnEnt(Mob *other, uint32 in_hate, uint32 in_dam);
		void SpellCast(Mob *caster, uint32 spell_id, float range, Mob *ae_center  = nullptr);
		void WipeHateList(); 


		/* For accessing the hate list via perl; don't use for anything else */
		std::list<struct_HateList*>& GetHateList()
		{
			return list;
		}

		/* Setting owner*/
		void SetOwner(Mob *new_owner)
		{
			owner = new_owner;
		}

	protected:
		struct_HateList* Find(Mob *ent);
	private:
		std::list<struct_HateList*> list;
		Mob *owner;
};

#endif
