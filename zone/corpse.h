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

#ifndef CORPSE_H
#define CORPSE_H

#include "mob.h"

class Client;
class NPC;

#define MAX_LOOTERS 72

class Corpse : public Mob
{
public:
	static void SendEndLootErrorPacket(Client* client);
	static void SendLootReqErrorPacket(Client* client, uint8 response = 2);
	static Corpse* LoadFromDBData(uint32 in_corpseid, uint32 in_charid, char* in_charname, uchar* in_data, uint32 in_datasize, float in_x, float in_y, float in_z, float in_heading, char* timeofdeath, bool rezzed = false, bool wasAtGraveyard = false);

	Corpse(NPC* in_npc, ItemList* in_itemlist, uint32 in_npctypeid, const NPCType** in_npctypedata, uint32 in_decaytime = 600000);
	Corpse(Client* client, int32 in_rezexp);
	Corpse(uint32 in_corpseid, uint32 in_charid, char* in_charname, ItemList* in_itemlist, uint32 in_copper, uint32 in_silver, uint32 in_gold, uint32 in_plat, float in_x, float in_y, float in_z, float in_heading, float in_size, uint8 in_gender, uint16 in_race, uint8 in_class, uint8 in_deity, uint8 in_level, uint8 in_texture, uint8 in_helmtexture,uint32 in_rezexp, bool wasAtGraveyard = false);
	~Corpse();

	//abstract virtual function implementations requird by base abstract class
	virtual bool Death(Mob* killerMob, int32 damage, uint16 spell_id, SkillUseTypes attack_skill) { return true; }
	virtual void Damage(Mob* from, int32 damage, uint16 spell_id, SkillUseTypes attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false) { return; }
	virtual bool Attack(Mob* other, int Hand = 13, bool FromRiposte = false, bool IsStrikethrough = true, bool IsFromSpell = false,
		ExtraAttackOptions *opts = nullptr) { return false; }
	virtual bool HasRaid() { return false; }
	virtual bool HasGroup() { return false; }
	virtual Raid* GetRaid() { return 0; }
	virtual Group* GetGroup() { return 0; }

	void LoadPlayerCorpseDecayTime(uint32 dbid);

	bool	IsCorpse()			const { return true; }
	bool	IsPlayerCorpse()	const { return mPlayerCorpse; }
	bool	IsNPCCorpse()		const { return !mPlayerCorpse; }
	bool	Process();
	bool	Save();
	
	uint32	GetDecayTime()		{ if (!mDecayTimer.Enabled()) return 0xFFFFFFFF; else return mDecayTimer.GetRemainingTime(); }
	uint32	GetResTime()		{ if (!mResurrectionTimer.Enabled()) return 0; else return mResurrectionTimer.GetRemainingTime(); }
	void	CalcCorpseName();

	inline void		ResetLooter()	{ mBeingLootedBy = 0xFFFFFFFF; }
	inline bool		IsBeingLooted() { return (mBeingLootedBy != 0xFFFFFFFF); }
	inline uint32	GetDBID()		{ return mDBID; }
	inline char*	GetOwnerName()	{ return orgname;}

	void	SetDecayTimer(uint32 decaytime);
	bool	IsEmpty() const;
	void	AddItem(uint32 itemnum, uint16 charges, int16 slot = 0, uint32 aug1=0, uint32 aug2=0, uint32 aug3=0, uint32 aug4=0, uint32 aug5=0);
	uint32	GetWornItem(int16 equipSlot) const;
	ServerLootItem_Struct* GetItem(uint16 lootslot, ServerLootItem_Struct** bag_item_data = 0);
	void	RemoveItem(uint16 lootslot);
	void	RemoveItem(ServerLootItem_Struct* item_data);
	
	
	
	
	void	Delete();
	void	Bury();
	virtual void	Depop();
	virtual void	DepopCorpse();

	void	FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	void	MakeLootRequestPackets(Client* client, const EQApplicationPacket* app);
	void	LootItem(Client* client, const EQApplicationPacket* app);
	void	EndLoot(Client* client, const EQApplicationPacket* app);
	bool	Summon(Client* client, bool spell, bool CheckDistance);


	bool canMobLoot(int charid);

	void AllowMobLoot(Mob* pCharacer, uint8 pSlot);
	void addLooter(Mob* pCharacer);

	void	Spawn();

	char		orgname[64];
	uint32 GetEquipment(uint8 material_slot) const;	// returns item id
	uint32 GetEquipmentColor(uint8 material_slot) const;

	// Sets the amount of coin on the corpse.
	// (SCRIPTING)
	void setCash(uint32 pCopper, uint32 pSilver, uint32 pGold, uint32 pPlatinum);

	// Removes all cash from the corpse.
	void removeCash();

	uint32 getCopper() { return mCopper; }
	uint32 getSilver() { return mSilver; }
	uint32 getGold() { return mGold; }
	uint32 getPlatinum() { return mPlatinum; }
	
	// Returns the amount of EXP stored on this corpse.
	inline int getResurrectionExp() { return mResurrectionExp; }

	void CastResurrection(uint16 spellid, Mob* Caster);
	void CompleteResurrection();

	// Returns whether this corpse has been resurrected.
	bool isResurrected() { return mResurrected; }

	// Sets whether this corpse has been resurrected.
	void setResurrected(bool pResurrected) { mResurrected = pResurrected; }

	void setPKItem(int32 id) { mPKItem = id; }
	int32 getPKItem() { return mPKItem; }
	
	// Returns the Character ID this corpse belongs to.
	uint32 getCharacterID() { return mCharacterID; }
	// Sets the Character ID this corpse belongs to.
	// (SCRIPTING)
	uint32 setCharacterID(uint32 pCharacterID) { if (IsPlayerCorpse()) { return (mCharacterID = pCharacterID); } return 0xFFFFFFFF; };

	// locks the corpse to prevent looting by non-GM characters.
	// (SCRIPTING)
	inline void	lock() { mLocked = true; }

	// Unlocks the corpse to allow looting by non-GM characters.
	// (SCRIPTING)
	inline void unlock() { mLocked = false; }

	// Returns whether the corpse is locked.
	// (SCRIPTING)
	inline bool isLocked() { return mLocked; }

	// Returns the number of items on the corpse.
	// (SCRIPTING)
	uint32 getNumItems();

	// Prints details of items/coin on corpse to client.
	// (SCRIPTING)
	void queryLoot(Client* pClient);

protected:
	std::list<uint32> MoveItemToCorpse(Client *client, ItemInst *item, int16 equipslot);

private:

	bool mDespawnRequested; // Flag indicating that this corpse should despawn.
	bool mChanged; // Flag indicating whether a player corpse has been changed since last save.
	bool mResurrected; // Flag indicating whether the corpse has been resurrected.
	bool mLocked; // Flag indicating whether the corpse has been locked.
	bool mPlayerCorpse; // Flag indicating whether the corpse belongs to a player.

	// Coin
	uint32 mCopper;
	uint32 mSilver;
	uint32 mGold;
	uint32 mPlatinum;

	int32 mPKItem; // Not completely sure what this does yet.
	uint32 mDBID; // ID field in 'player_corpses' table.
	uint32 mCharacterID; // Character ID who owns this corpse
	uint32 mBeingLootedBy; // Client ID of whoever is looting this corpse.
	uint32 mResurrectionExp; // Amount of EXP the player lost creating this corpse.

	Timer mDecayTimer; // Timer controlling how long before this corpse decays.
	Timer mResurrectionTimer; // Timer controlling how long this corpse can be resurrected.
	Timer mCoolDownTimer; // Timer prevents anyone from looting a corpse within 10ms (No idea why).
	Timer mGraveyardTimer; // Timer controlling when this corpse will be moved to the graveyard.
	Timer mLockoutTimer; // Timer controlling how long before the corpse becomes open and allow anyone to loot.
	
	ItemList mItems; // List of items on the corpse.
	int mLooters[MAX_LOOTERS]; // Character IDs of those allowed to loot this corpse.
	Color_Struct mItemTints[9];
};

#endif
