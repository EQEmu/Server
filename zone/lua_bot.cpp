#include "lua_entity.h"
#ifdef BOTS
#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "bot.h"
#include "lua_bot.h"
#include "lua_iteminst.h"
#include "lua_mob.h"

void lua_register_bot(sol::state_view &sv)
{
	auto bot = sv.new_usertype<Lua_Bot>("Bot", sol::constructors<Lua_Bot()>(), sol::base_classes,
					    sol::bases<Lua_Mob, Lua_Entity>());
	bot["AddBotItem"] = sol::overload(
	    (void(Lua_Bot::*)(uint16, uint32)) & Lua_Bot::AddBotItem,
	    (void(Lua_Bot::*)(uint16, uint32, int16)) & Lua_Bot::AddBotItem,
	    (void(Lua_Bot::*)(uint16, uint32, int16, bool)) & Lua_Bot::AddBotItem,
	    (void(Lua_Bot::*)(uint16, uint32, int16, bool, uint32)) & Lua_Bot::AddBotItem,
	    (void(Lua_Bot::*)(uint16, uint32, int16, bool, uint32, uint32)) & Lua_Bot::AddBotItem,
	    (void(Lua_Bot::*)(uint16, uint32, int16, bool, uint32, uint32, uint32)) & Lua_Bot::AddBotItem,
	    (void(Lua_Bot::*)(uint16, uint32, int16, bool, uint32, uint32, uint32, uint32)) & Lua_Bot::AddBotItem,
	    (void(Lua_Bot::*)(uint16, uint32, int16, bool, uint32, uint32, uint32, uint32, uint32)) &
		Lua_Bot::AddBotItem,
	    (void(Lua_Bot::*)(uint16, uint32, int16, bool, uint32, uint32, uint32, uint32, uint32, uint32)) &
		Lua_Bot::AddBotItem);
	bot["CountBotItem"] = (uint32(Lua_Bot::*)(uint32))&Lua_Bot::CountBotItem;
	bot["GetBotItem"] = (Lua_ItemInst(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItem;
	bot["GetBotItemIDBySlot"] = (uint32(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItemIDBySlot;
	bot["GetOwner"] = (Lua_Mob(Lua_Bot::*)(void))&Lua_Bot::GetOwner;
	bot["HasBotItem"] = (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasBotItem;
	bot["RemoveBotItem"] = (void(Lua_Bot::*)(uint32))&Lua_Bot::RemoveBotItem;
}

#endif
#endif
