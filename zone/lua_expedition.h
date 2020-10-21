/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EQEMU_LUA_EXPEDITION_H
#define EQEMU_LUA_EXPEDITION_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"
#include "../common/types.h"
#include <string>

class Expedition;
class Lua_Client;
struct lua_State;

namespace luabind {
	struct scope;
	namespace adl {
		class object;
	}
	using adl::object;
}

luabind::scope lua_register_expedition();
luabind::scope lua_register_expedition_lock_messages();

class Lua_Expedition : public Lua_Ptr<Expedition>
{
	typedef Expedition NativeType;
public:
	Lua_Expedition() : Lua_Ptr(nullptr) { }
	Lua_Expedition(Expedition *d) : Lua_Ptr(d) { }
	virtual ~Lua_Expedition() { }

	operator Expedition*() {
		return reinterpret_cast<Expedition*>(GetLuaPtrData());
	}

	void            AddLockout(std::string event_name, uint32_t seconds);
	void            AddLockoutDuration(std::string event_name, int seconds);
	void            AddLockoutDuration(std::string event_name, int seconds, bool members_only);
	void            AddReplayLockout(uint32_t seconds);
	void            AddReplayLockoutDuration(int seconds);
	void            AddReplayLockoutDuration(int seconds, bool members_only);
	uint32_t        GetDynamicZoneID();
	uint32_t        GetID();
	int             GetInstanceID();
	std::string     GetLeaderName();
	luabind::object GetLockouts(lua_State* L);
	std::string     GetLootEventByNPCTypeID(uint32_t npc_type_id);
	std::string     GetLootEventBySpawnID(uint32_t spawn_id);
	uint32_t        GetMemberCount();
	luabind::object GetMembers(lua_State* L);
	std::string     GetName();
	int             GetSecondsRemaining();
	std::string     GetUUID();
	int             GetZoneID();
	std::string     GetZoneName();
	int             GetZoneVersion();
	bool            HasLockout(std::string event_name);
	bool            HasReplayLockout();
	void            RemoveCompass();
	void            RemoveLockout(std::string event_name);
	void            SetCompass(uint32_t zone_id, float x, float y, float z);
	void            SetCompass(std::string zone_name, float x, float y, float z);
	void            SetLocked(bool lock_expedition);
	void            SetLocked(bool lock_expedition, int lock_msg);
	void            SetLocked(bool lock_expedition, int lock_msg, uint32_t color);
	void            SetLootEventByNPCTypeID(uint32_t npc_type_id, std::string event_name);
	void            SetLootEventBySpawnID(uint32_t spawn_id, std::string event_name);
	void            SetReplayLockoutOnMemberJoin(bool enable);
	void            SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading);
	void            SetSafeReturn(std::string zone_name, float x, float y, float z, float heading);
	void            SetSecondsRemaining(uint32_t seconds_remaining);
	void            SetZoneInLocation(float x, float y, float z, float heading);
	void            UpdateLockoutDuration(std::string event_name, uint32_t duration);
	void            UpdateLockoutDuration(std::string event_name, uint32_t duration, bool members_only);
};

#endif // LUA_EQEMU
#endif // EQEMU_LUA_EXPEDITION_H
