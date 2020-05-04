#ifndef EQEMU_LUA_RAID_H
#define EQEMU_LUA_RAID_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class Raid;
class Lua_Client;
class Lua_Mob;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_raid();

class Lua_Raid : public Lua_Ptr<Raid>
{
	typedef Raid NativeType;
public:
	Lua_Raid() : Lua_Ptr(nullptr) { }
	Lua_Raid(Raid *d) : Lua_Ptr(d) { }
	virtual ~Lua_Raid() { }

	operator Raid*() {
		return reinterpret_cast<Raid*>(GetLuaPtrData());
	}

	bool IsRaidMember(const char *name);
	void CastGroupSpell(Lua_Mob caster, int spell_id, uint32 group_id);
	int GroupCount(uint32 group_id);
	int RaidCount();
	int GetGroup(const char *c);
	int GetGroup(Lua_Client c);
	void SplitExp(uint32 exp, Lua_Mob other);
	uint32 GetTotalRaidDamage(Lua_Mob other);
	void SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum);
	void SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Lua_Client splitter);
	void BalanceHP(int penalty, uint32 group_id);
	bool IsLeader(const char *c);
	bool IsLeader(Lua_Client c);
	bool IsGroupLeader(const char *name);
	int GetHighestLevel();
	int GetLowestLevel();
	Lua_Client GetClientByIndex(int index);
	void TeleportGroup(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h, uint32 group_id);
	void TeleportRaid(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h);
	int GetID();
	Lua_Client GetMember(int index);
	int GetGroupNumber(int index);
};

#endif
#endif
