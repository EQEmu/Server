#ifndef EQEMU_LUA_CORPSE_H
#define EQEMU_LUA_CORPSE_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Corpse;
class Lua_Client;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_corpse();

class Lua_Corpse : public Lua_Mob
{
	typedef Corpse NativeType;
public:
	Lua_Corpse() { SetLuaPtrData(nullptr); }
	Lua_Corpse(Corpse *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_Corpse() { }

	operator Corpse*() {
		return reinterpret_cast<Corpse*>(GetLuaPtrData());
	}

	uint32 GetCharID();
	uint32 GetDecayTime();
	void Lock();
	void UnLock();
	bool IsLocked();
	void ResetLooter();
	uint32 GetDBID();
	bool IsRezzed();
	const char *GetOwnerName();
	bool Save();
	void Delete();
	void Bury();
	void Depop();
	uint32 CountItems();
	void AddItem(uint32 itemnum, uint16 charges, int16 slot, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5);
	uint32 GetWornItem(int16 equipSlot);
	void RemoveItem(uint16 lootslot);
	void SetCash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum);
	void RemoveCash();
	bool IsEmpty();
	void SetDecayTimer(uint32 decaytime);
	bool CanMobLoot(int charid);
	void AllowMobLoot(Lua_Mob them, uint8 slot);
	bool Summon(Lua_Client client, bool spell, bool checkdistance);
	uint32 GetCopper();
	uint32 GetSilver();
	uint32 GetGold();
	uint32 GetPlatinum();
	void AddLooter(Lua_Mob who);
	bool HasItem(uint32 item_id);
	uint16 CountItem(uint32 item_id);
	uint32 GetItemIDBySlot(uint16 loot_slot);
	uint16 GetFirstSlotByItemID(uint32 item_id);
};

#endif
#endif
