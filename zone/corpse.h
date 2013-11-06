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
	bool	IsPlayerCorpse()	const { return p_PlayerCorpse; }
	bool	IsNPCCorpse()		const { return !p_PlayerCorpse; }
	bool	IsBecomeNPCCorpse() const { return become_npc; }
	bool	Process();
	bool	Save();
	uint32	GetCharID()			{ return charid; }
	uint32	SetCharID(uint32 iCharID) { if (IsPlayerCorpse()) { return (charid=iCharID); } return 0xFFFFFFFF; };
	uint32	GetDecayTime()		{ if (!corpse_decay_timer.Enabled()) return 0xFFFFFFFF; else return corpse_decay_timer.GetRemainingTime(); }
	uint32	GetResTime()		{ if (!corpse_res_timer.Enabled()) return 0; else return corpse_res_timer.GetRemainingTime(); }
	void	CalcCorpseName();
	inline void		Lock()			{ pLocked = true; }
	inline void		UnLock()		{ pLocked = false; }
	inline bool		IsLocked()		{ return pLocked; }
	inline void		ResetLooter()	{ BeingLootedBy = 0xFFFFFFFF; }
	inline bool		IsBeingLooted() { return (BeingLootedBy != 0xFFFFFFFF); }
	inline uint32	GetDBID()		{ return dbid; }
	inline char*	GetOwnerName()	{ return orgname;}

	void	SetDecayTimer(uint32 decaytime);
	bool	IsEmpty() const;
	void	AddItem(uint32 itemnum, uint16 charges, int16 slot = 0, uint32 aug1=0, uint32 aug2=0, uint32 aug3=0, uint32 aug4=0, uint32 aug5=0);
	uint32	GetWornItem(int16 equipSlot) const;
	ServerLootItem_Struct* GetItem(uint16 lootslot, ServerLootItem_Struct** bag_item_data = 0);
	void	RemoveItem(uint16 lootslot);
	void	RemoveItem(ServerLootItem_Struct* item_data);
	void	SetCash(uint32 in_copper, uint32 in_silver, uint32 in_gold, uint32 in_platinum);
	void	RemoveCash();
	void	QueryLoot(Client* to);
	uint32	CountItems();
	void	Delete();
	void	Bury();
	virtual void	Depop();
	virtual void	DepopCorpse();

	uint32	GetCopper()		{ return copper; }
	uint32	GetSilver()		{ return silver; }
	uint32	GetGold()		{ return gold; }
	uint32	GetPlatinum()	{ return platinum; }

	void	FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	void	MakeLootRequestPackets(Client* client, const EQApplicationPacket* app);
	void	LootItem(Client* client, const EQApplicationPacket* app);
	void	EndLoot(Client* client, const EQApplicationPacket* app);
	bool	Summon(Client* client, bool spell, bool CheckDistance);
	void	CastRezz(uint16 spellid, Mob* Caster);
	void	CompleteRezz();
	void	SetPKItem(int32 id) { pkitem = id; }
	int32	GetPKItem() { return pkitem; }
	bool CanMobLoot(int charid);
	void AllowMobLoot(Mob *them, uint8 slot);
	void AddLooter(Mob *who);
	bool Rezzed() { return rez; }
	void Rezzed(bool in_rez) { rez = in_rez; }
	void	Spawn();

	char		orgname[64];
	uint32 GetEquipment(uint8 material_slot) const;	// returns item id
	uint32 GetEquipmentColor(uint8 material_slot) const;
	inline int GetRezzExp() { return rezzexp; }

protected:
	std::list<uint32> MoveItemToCorpse(Client *client, ItemInst *item, int16 equipslot);

private:
	bool		p_PlayerCorpse;	bool		pIsChanged;
	bool		pLocked;
	int32		pkitem;
	uint32		dbid;
	uint32		charid;
	ItemList	itemlist;
	uint32		copper;
	uint32		silver;
	uint32		gold;
	uint32		platinum;
	bool		p_depop;
	uint32		BeingLootedBy;
	uint32		rezzexp;
	bool		rez;
	bool		can_rez;
	bool		become_npc;
	int			looters[MAX_LOOTERS]; // People allowed to loot the corpse, character id
	Timer		corpse_decay_timer;
	Timer		corpse_res_timer;
	Timer		corpse_delay_timer;
	Timer		corpse_graveyard_timer;
	Timer		loot_cooldown_timer;
	Color_Struct item_tint[9];
};

#endif
