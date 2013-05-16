#include <sstream>

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <boost/any.hpp>

#include "QuestParserCollection.h"
#include "QuestInterface.h"

#include "masterentity.h"
#include "../common/seperator.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_item.h"
#include "zone.h"
#include "lua_parser_events.h"

//NPC
void handle_npc_event_say(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "message");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "language");
}

void handle_npc_event_trade(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
	
	lua_createtable(L, 0, 0);
	std::stringstream ident;
	ident << npc->GetNPCTypeID();
	
	lua_pushinteger(L, std::stoul(parse->GetVar("item1." + ident.str())));
	lua_setfield(L, -2, "item1");

	lua_pushinteger(L, std::stoul(parse->GetVar("item2." + ident.str())));
	lua_setfield(L, -2, "item2");

	lua_pushinteger(L, std::stoul(parse->GetVar("item3." + ident.str())));
	lua_setfield(L, -2, "item3");

	lua_pushinteger(L, std::stoul(parse->GetVar("item4." + ident.str())));
	lua_setfield(L, -2, "item4");

	lua_pushinteger(L, std::stoul(parse->GetVar("item1.charges." + ident.str())));
	lua_setfield(L, -2, "item1_charges");

	lua_pushinteger(L, std::stoul(parse->GetVar("item2.charges." + ident.str())));
	lua_setfield(L, -2, "item2_charges");

	lua_pushinteger(L, std::stoul(parse->GetVar("item3.charges." + ident.str())));
	lua_setfield(L, -2, "item3_charges");

	lua_pushinteger(L, std::stoul(parse->GetVar("item4.charges." + ident.str())));
	lua_setfield(L, -2, "item4_charges");

	lua_pushboolean(L, std::stoul(parse->GetVar("item1.attuned." + ident.str())) != 0 ? true : false);
	lua_setfield(L, -2, "item1_attuned");

	lua_pushboolean(L, std::stoul(parse->GetVar("item2.attuned." + ident.str())) != 0 ? true : false);
	lua_setfield(L, -2, "item1_attuned");

	lua_pushboolean(L, std::stoul(parse->GetVar("item3.attuned." + ident.str())) != 0 ? true : false);
	lua_setfield(L, -2, "item3_attuned");

	lua_pushboolean(L, std::stoul(parse->GetVar("item4.attuned." + ident.str())) != 0 ? true : false);
	lua_setfield(L, -2, "item4_attuned");

	lua_pushinteger(L, std::stoul(parse->GetVar("platinum." + ident.str())));
	lua_setfield(L, -2, "platinum");

	lua_pushinteger(L, std::stoul(parse->GetVar("gold." + ident.str())));
	lua_setfield(L, -2, "gold");

	lua_pushinteger(L, std::stoul(parse->GetVar("silver." + ident.str())));
	lua_setfield(L, -2, "silver");

	lua_pushinteger(L, std::stoul(parse->GetVar("copper." + ident.str())));
	lua_setfield(L, -2, "copper");
	lua_setfield(L, -2, "trade");
}

void handle_npc_event_hp(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	if(extra_data == 1) {
		lua_pushinteger(L, -1);
		lua_setfield(L, -2, "hp_event");
		lua_pushinteger(L, std::stoi(data));
		lua_setfield(L, -2, "inc_hp_event");
	}
	else
	{
		lua_pushinteger(L, std::stoi(data));
		lua_setfield(L, -2, "hp_event");
		lua_pushinteger(L, -1);
		lua_setfield(L, -2, "inc_hp_event");
	}
}

void handle_npc_single_mob(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_single_client(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_single_npc(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_NPC l_npc(reinterpret_cast<NPC*>(init));
	luabind::object l_npc_o = luabind::object(L, l_npc);
	l_npc_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_popup(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "popup_id");
}

void handle_npc_waypoint(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "wp");
}

void handle_npc_hate(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushboolean(L, std::stoi(data) == 0 ? false : true);
	lua_setfield(L, -2, "joined");
}


void handle_npc_signal(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "signal_id");
}

void handle_npc_timer(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_npc_death(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "damage");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "spell_id");

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "skill_id");
}

void handle_npc_null(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
}

//Player
void handle_player_null(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data) {
}

//Item
void handle_item_null(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, uint32 objid, uint32 extra_data) {
}

//Spell
void handle_spell_null(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, uint32 extra_data) {
}
