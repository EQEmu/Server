#ifdef LUA_EQEMU
#include <sstream>

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "quest_parser_collection.h"
#include "quest_interface.h"

#include "masterentity.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_spell.h"
#include "lua_corpse.h"
#include "lua_door.h"
#include "lua_object.h"
#include "lua_packet.h"
#include "lua_encounter.h"
#include "zone.h"
#include "lua_parser_events.h"

//NPC
void handle_npc_event_say(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	npc->DoQuestPause(init);

	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "message");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "language");
}

void handle_npc_event_trade(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Client           l_client(reinterpret_cast<Client *>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_createtable(L, 0, 0);
	const auto npc_id = npc->GetNPCTypeID();

	if (extra_pointers) {
		size_t sz = extra_pointers->size();
		for (size_t i = 0; i < sz; ++i) {
			auto prefix = fmt::format("item{}", i + 1);
			auto* inst = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(i));

			Lua_ItemInst l_inst = inst;
			luabind::adl::object l_inst_o = luabind::adl::object(L, l_inst);
			l_inst_o.push(L);

			lua_setfield(L, -2, prefix.c_str());
		}
	}

	auto money_string = fmt::format("platinum.{}", npc_id);
	uint32 money_value = !parse->GetVar(money_string).empty() ? Strings::ToUnsignedInt(parse->GetVar(money_string)) : 0;

	lua_pushinteger(L, money_value);
	lua_setfield(L, -2, "platinum");

	money_string = fmt::format("gold.{}", npc_id);
	money_value = !parse->GetVar(money_string).empty() ? Strings::ToUnsignedInt(parse->GetVar(money_string)) : 0;

	lua_pushinteger(L, money_value);
	lua_setfield(L, -2, "gold");

	money_string = fmt::format("silver.{}", npc_id);
	money_value = !parse->GetVar(money_string).empty() ? Strings::ToUnsignedInt(parse->GetVar(money_string)) : 0;

	lua_pushinteger(L, money_value);
	lua_setfield(L, -2, "silver");

	money_string = fmt::format("copper.{}", npc_id);
	money_value = !parse->GetVar(money_string).empty() ? Strings::ToUnsignedInt(parse->GetVar(money_string)) : 0;

	lua_pushinteger(L, money_value);
	lua_setfield(L, -2, "copper");

	// set a reference to the client inside of the trade object as well for plugins to process
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_setfield(L, -2, "trade");
}

void handle_npc_event_hp(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	if(extra_data == 1) {
		lua_pushinteger(L, -1);
		lua_setfield(L, -2, "hp_event");
		lua_pushinteger(L, Strings::ToInt(data));
		lua_setfield(L, -2, "inc_hp_event");
	}
	else
	{
		lua_pushinteger(L, Strings::ToInt(data));
		lua_setfield(L, -2, "hp_event");
		lua_pushinteger(L, -1);
		lua_setfield(L, -2, "inc_hp_event");
	}
}

void handle_npc_single_mob(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_single_client(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_single_npc(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_NPC l_npc(reinterpret_cast<NPC*>(init));
	luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
	l_npc_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_task_accepted(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "task_id");
}

void handle_npc_popup(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "popup_id");
}

void handle_npc_waypoint(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, Strings::ToInt(data, -1));
	lua_setfield(L, -2, "wp");
}

void handle_npc_hate(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushboolean(L, Strings::ToInt(data) == 0 ? false : true);
	lua_setfield(L, -2, "joined");
}


void handle_npc_signal(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "signal");
}

void handle_npc_payload(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "payload_id");

	lua_pushstring(L, sep.argplus[1]);
	lua_setfield(L, -2, "payload_value");
}

void handle_npc_timer(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_npc_death(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "killer_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "damage");

	int spell_id = Strings::ToInt(sep.arg[2]);
	if(IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "skill_id");

	if (extra_pointers && extra_pointers->size() >= 1)
	{
		Lua_Corpse l_corpse(std::any_cast<Corpse*>(extra_pointers->at(0)));
		luabind::adl::object l_corpse_o = luabind::adl::object(L, l_corpse);
		l_corpse_o.push(L);
		lua_setfield(L, -2, "corpse");
	}

	if (extra_pointers && extra_pointers->size() >= 2)
	{
		Lua_NPC l_npc(std::any_cast<NPC*>(extra_pointers->at(1)));
		luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
		l_npc_o.push(L);
		lua_setfield(L, -2, "killed");
	}
}

void handle_npc_cast(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	int spell_id = Strings::ToInt(data);
	if(IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}
}

void handle_npc_area(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, *std::any_cast<int*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "area_id");

	lua_pushinteger(L, *std::any_cast<int*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "area_type");
}

void handle_npc_null(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
}

void handle_npc_loot_zone(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	Lua_Corpse l_corpse(std::any_cast<Corpse*>(extra_pointers->at(1)));
	luabind::adl::object l_corpse_o = luabind::adl::object(L, l_corpse);
	l_corpse_o.push(L);
	lua_setfield(L, -2, "corpse");
}

void handle_npc_spawn_zone(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_NPC l_npc(std::any_cast<NPC*>(init->CastToNPC()));
	luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
	l_npc_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_despawn_zone(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_damage(
	QuestInterface *parse,
	lua_State* L,
	NPC* npc,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	lua_pushnumber(L, Strings::ToUnsignedInt(sep.arg[0]));
	lua_setfield(L, -2, "entity_id");

	lua_pushnumber(L, Strings::ToBigInt(sep.arg[1]));
	lua_setfield(L, -2, "damage");

	lua_pushnumber(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "spell_id");

	lua_pushnumber(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "skill_id");

	lua_pushboolean(L, Strings::ToInt(sep.arg[4]) == 0 ? false : true);
	lua_setfield(L, -2, "is_damage_shield");

	lua_pushboolean(L, Strings::ToInt(sep.arg[5]) == 0 ? false : true);
	lua_setfield(L, -2, "is_avoidable");

	lua_pushnumber(L, Strings::ToInt(sep.arg[6]));
	lua_setfield(L, -2, "buff_slot");

	lua_pushboolean(L, Strings::ToInt(sep.arg[7]) == 0 ? false : true);
	lua_setfield(L, -2, "is_buff_tic");

	lua_pushnumber(L, Strings::ToInt(sep.arg[8]));
	lua_setfield(L, -2, "special_attack");

	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

// Player
void handle_player_say(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "message");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "language");
}

void handle_player_environmental_damage(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "env_damage");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "env_damage_type");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "env_final_damage");
}

void handle_player_death(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	Mob *o = entity_list.GetMobID(Strings::ToInt(sep.arg[0]));
	Lua_Mob l_mob(o);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "killer_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "damage");

	int spell_id = Strings::ToInt(sep.arg[3]);
	if(IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}

	lua_pushinteger(L, Strings::ToInt(sep.arg[4]));
	lua_setfield(L, -2, "skill");
}

void handle_player_timer(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_player_discover_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	const EQ::ItemData *item = database.GetItem(extra_data);
	if(item) {
		Lua_Item l_item(item);
		luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "item");
	} else {
		Lua_Item l_item(nullptr);
		luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "item");
	}
}

void handle_player_fish_forage_success(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_player_click_object(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Object l_object(std::any_cast<Object*>(extra_pointers->at(0)));
	luabind::adl::object l_object_o = luabind::adl::object(L, l_object);
	l_object_o.push(L);
	lua_setfield(L, -2, "object");
}

void handle_player_click_door(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Door l_door(std::any_cast<Doors*>(extra_pointers->at(0)));
	luabind::adl::object l_door_o = luabind::adl::object(L, l_door);
	l_door_o.push(L);
	lua_setfield(L, -2, "door");
}

void handle_player_signal(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "signal");
}

void handle_player_payload(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "payload_id");

	lua_pushstring(L, sep.argplus[1]);
	lua_setfield(L, -2, "payload_value");
}

void handle_player_popup_response(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "popup_id");
}

void handle_player_pick_up(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_player_cast(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	int spell_id = Strings::ToInt(sep.arg[0]);
	if(IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
	}

	lua_setfield(L, -2, "spell");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "caster_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "caster_level");
}

void handle_player_task_fail(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "task_id");
}

void handle_player_zone(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "from_zone_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "from_instance_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "from_instance_version");

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "zone_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[4]));
	lua_setfield(L, -2, "instance_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[5]));
	lua_setfield(L, -2, "instance_version");
}

void handle_player_duel_win(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Client l_client(std::any_cast<Client*>(extra_pointers->at(1)));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_duel_loss(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Client l_client(std::any_cast<Client*>(extra_pointers->at(0)));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_loot(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	Lua_Corpse l_corpse(std::any_cast<Corpse*>(extra_pointers->at(1)));
	luabind::adl::object l_corpse_o = luabind::adl::object(L, l_corpse);
	l_corpse_o.push(L);
	lua_setfield(L, -2, "corpse");
}

void handle_player_task_stage_complete(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "task_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "activity_id");
}

void handle_player_task_accepted(
	QuestInterface* parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any>* extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "task_id");
}

void handle_player_task_update(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "count");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "activity_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "task_id");
}

void handle_player_command(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str(), ' ', 10, 100, true);
	std::string command(sep.arg[0] + 1);
	lua_pushstring(L, command.c_str());
	lua_setfield(L, -2, "command");

	luabind::adl::object args = luabind::newtable(L);
	int max_args = sep.GetMaxArgNum();
	for(int i = 1; i < max_args; ++i) {
		if(strlen(sep.arg[i]) > 0) {
			args[i] = std::string(sep.arg[i]);
		}
	}

	args.push(L);
	lua_setfield(L, -2, "args");
}

void handle_player_combine(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "recipe_id");

	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "recipe_name");
}

void handle_player_feign(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_NPC l_npc(std::any_cast<NPC*>(extra_pointers->at(0)));
	luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
	l_npc_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_area(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, *std::any_cast<int*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "area_id");

	lua_pushinteger(L, *std::any_cast<int*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "area_type");
}

void handle_player_respawn(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "option");

	lua_pushboolean(L, extra_data == 1 ? true : false);
	lua_setfield(L, -2, "resurrect");
}

void handle_player_packet(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Packet l_packet(std::any_cast<EQApplicationPacket*>(extra_pointers->at(0)));
	luabind::adl::object l_packet_o = luabind::adl::object(L, l_packet);
	l_packet_o.push(L);
	lua_setfield(L, -2, "packet");

	lua_pushboolean(L, extra_data == 1 ? true : false);
	lua_setfield(L, -2, "connecting");
}

void handle_player_null(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
}

void handle_player_use_skill(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "skill_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "skill_level");
}

void handle_test_buff(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
}

void handle_player_combine_validate(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "recipe_id");

	lua_pushstring(L, sep.arg[0]);
	lua_setfield(L, -2, "validate_type");

	int zone_id = -1;
	int tradeskill_id = -1;
	if (strcmp(sep.arg[0], "check_zone") == 0) {
		zone_id = Strings::ToInt(sep.arg[1]);
	}
	else if (strcmp(sep.arg[0], "check_tradeskill") == 0) {
		tradeskill_id = Strings::ToInt(sep.arg[1]);
	}

	lua_pushinteger(L, zone_id);
	lua_setfield(L, -2, "zone_id");

	lua_pushinteger(L, tradeskill_id);
	lua_setfield(L, -2, "tradeskill_id");
}

void handle_player_bot_command(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str(), ' ', 10, 100, true);
	std::string bot_command(sep.arg[0] + 1);
	lua_pushstring(L, bot_command.c_str());
	lua_setfield(L, -2, "bot_command");

	luabind::adl::object args = luabind::newtable(L);
	int max_args = sep.GetMaxArgNum();
	for (int i = 1; i < max_args; ++i) {
		if (strlen(sep.arg[i]) > 0) {
			args[i] = std::string(sep.arg[i]);
		}
	}

	args.push(L);
	lua_setfield(L, -2, "args");
}

void handle_player_warp(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushnumber(L, Strings::ToFloat(sep.arg[0]));
	lua_setfield(L, -2, "from_x");

	lua_pushnumber(L, Strings::ToFloat(sep.arg[1]));
	lua_setfield(L, -2, "from_y");

	lua_pushnumber(L, Strings::ToFloat(sep.arg[2]));
	lua_setfield(L, -2, "from_z");
}

void handle_player_quest_combine(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "container_slot");
 }

void handle_player_consider(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "entity_id");

	if (extra_pointers && extra_pointers->size() == 1) {
		Lua_NPC l_npc(std::any_cast<NPC*>(extra_pointers->at(0)));
		luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
		l_npc_o.push(L);
		lua_setfield(L, -2, "other");
	}
}

void handle_player_consider_corpse(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "corpse_entity_id");

	if (extra_pointers && extra_pointers->size() == 1) {
		Lua_Corpse           l_corpse(std::any_cast<Corpse *>(extra_pointers->at(0)));
		luabind::adl::object l_corpse_o = luabind::adl::object(L, l_corpse);
		l_corpse_o.push(L);
		lua_setfield(L, -2, "corpse");
	}
}

void handle_player_inspect(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(std::any_cast<Mob*>(extra_pointers->at(0)));
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_aa_buy(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "aa_cost");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "aa_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "aa_previous_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "aa_next_id");
}

void handle_player_aa_gain(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "aa_gained");
}

void handle_player_aa_exp_gain(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToUnsignedBigInt(data));
	lua_setfield(L, -2, "aa_exp_gained");
}

void handle_player_exp_gain(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToUnsignedBigInt(data));
	lua_setfield(L, -2, "exp_gained");
}

void handle_player_level_up(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToUnsignedInt(data));
	lua_setfield(L, -2, "levels_gained");
}

void handle_player_level_down(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToUnsignedInt(data));
	lua_setfield(L, -2, "levels_lost");
}

void handle_player_gm_command(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "message");
}

void handle_player_bot_create(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushstring(L, sep.arg[0]);
	lua_setfield(L, -2, "bot_name");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "bot_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "bot_race");

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "bot_class");

	lua_pushinteger(L, Strings::ToInt(sep.arg[4]));
	lua_setfield(L, -2, "bot_gender");
}

void handle_player_damage(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	lua_pushnumber(L, Strings::ToUnsignedInt(sep.arg[0]));
	lua_setfield(L, -2, "entity_id");

	lua_pushnumber(L, Strings::ToBigInt(sep.arg[1]));
	lua_setfield(L, -2, "damage");

	lua_pushnumber(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "spell_id");

	lua_pushnumber(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "skill_id");

	lua_pushboolean(L, Strings::ToInt(sep.arg[4]) == 0 ? false : true);
	lua_setfield(L, -2, "is_damage_shield");

	lua_pushboolean(L, Strings::ToInt(sep.arg[5]) == 0 ? false : true);
	lua_setfield(L, -2, "is_avoidable");

	lua_pushnumber(L, Strings::ToInt(sep.arg[6]));
	lua_setfield(L, -2, "buff_slot");

	lua_pushboolean(L, Strings::ToInt(sep.arg[7]) == 0 ? false : true);
	lua_setfield(L, -2, "is_buff_tic");

	lua_pushnumber(L, Strings::ToInt(sep.arg[8]));
	lua_setfield(L, -2, "special_attack");

	if (extra_pointers && extra_pointers->size() >= 1) {
		Lua_Mob l_mob(std::any_cast<Mob*>(extra_pointers->at(0)));
		luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
		l_mob_o.push(L);
		lua_setfield(L, -2, "other");
	}
}

void handle_player_item_click(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushnumber(L, Strings::ToInt(data));
	lua_setfield(L, -2, "slot_id");

	if (extra_pointers && extra_pointers->size() >= 1) {
		lua_pushnumber(L, std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))->GetID());
		lua_setfield(L, -2, "item_id");

		lua_pushstring(L, std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))->GetItem()->Name);
		lua_setfield(L, -2, "item_name");

		lua_pushnumber(L, std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))->GetItem()->Click.Effect);
		lua_setfield(L, -2, "spell_id");

		Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
		luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "item");
	}
}

void handle_player_destroy_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	if (extra_pointers && extra_pointers->size() >= 1) {
		lua_pushnumber(L, std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))->GetID());
		lua_setfield(L, -2, "item_id");

		lua_pushstring(L, std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))->GetItem()->Name);
		lua_setfield(L, -2, "item_name");

		lua_pushnumber(L, std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))->IsStackable() ? std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0))->GetCharges() : 1);
		lua_setfield(L, -2, "quantity");

		Lua_ItemInst         l_item(std::any_cast<EQ::ItemInstance *>(extra_pointers->at(0)));

		luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "item");
	}
}

void handle_player_drop_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushnumber(L, extra_data);
	lua_setfield(L, -2, "slot_id");

	if (extra_pointers && extra_pointers->size() == 1) {
		EQ::ItemInstance* item_inst = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));

		lua_pushnumber(L, item_inst->IsStackable() ? item_inst->GetCharges() : 1);
		lua_setfield(L, -2, "quantity");

		lua_pushnumber(L, item_inst->GetItem()->ID);
		lua_setfield(L, -2, "item_id");

		lua_pushstring(L, item_inst->GetItem()->Name);
		lua_setfield(L, -2, "item_name");

		lua_pushnumber(L, item_inst->GetItem()->Click.Effect);
		lua_setfield(L, -2, "spell_id");

		Lua_Item             l_item(item_inst->GetItem());
		luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "item");
	}
}

void handle_player_target_change(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	if (extra_pointers && extra_pointers->size() == 1) {
		Lua_Mob              l_mob(std::any_cast<Mob*>(extra_pointers->at(0)));
		luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
		l_mob_o.push(L);
		lua_setfield(L, -2, "other");
	}
}

// Item
void handle_item_click(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_timer(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_item_proc(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(mob);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "target");

	if(IsValidSpell(extra_data)) {
		Lua_Spell l_spell(&spells[extra_data]);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}
}

void handle_item_loot(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	if(mob && mob->IsCorpse()) {
		Lua_Corpse l_corpse(mob->CastToCorpse());
		luabind::adl::object l_corpse_o = luabind::adl::object(L, l_corpse);
		l_corpse_o.push(L);
		lua_setfield(L, -2, "corpse");
	} else {
		Lua_Corpse l_corpse(nullptr);
		luabind::adl::object l_corpse_o = luabind::adl::object(L, l_corpse);
		l_corpse_o.push(L);
		lua_setfield(L, -2, "corpse");
	}
}

void handle_item_equip(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "aug");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment_insert(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment_remove(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");

	lua_pushboolean(L, *std::any_cast<bool*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "destroyed");
}

void handle_item_null(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	EQ::ItemInstance* item,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
}

// Spell
void handle_spell_event(
	QuestInterface *parse,
	lua_State* L,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	if (mob) {
		Lua_Mob l_mob(mob);
		luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
		l_mob_o.push(L);
	} else if(client) {
		Lua_Mob l_client(client);
		luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
		l_client_o.push(L);
	} else {
		Lua_Mob l_mob(nullptr);
		luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
		l_mob_o.push(L);
	}

	lua_setfield(L, -2, "target");

	lua_pushinteger(L, spell_id);
	lua_setfield(L, -2, "spell_id");

	Seperator sep(data.c_str());

	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "caster_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "tics_remaining");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "caster_level");

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "buff_slot");

	Lua_Spell l_spell(spell_id);
	luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
	l_spell_o.push(L);
	lua_setfield(L, -2, "spell");
}

void handle_translocate_finish(
	QuestInterface *parse,
	lua_State* L,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	if (mob) {
		Lua_Mob l_mob(mob);
		luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
		l_mob_o.push(L);
	} else if(client) {
		Lua_Mob l_client(client);
		luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
		l_client_o.push(L);
	} else {
		Lua_Mob l_mob(nullptr);
		luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
		l_mob_o.push(L);
	}

	lua_setfield(L, -2, "target");
}

void handle_player_equip_item(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushnumber(L, extra_data);
	lua_setfield(L, -2, "item_id");

	Seperator sep(data.c_str());

	lua_pushnumber(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "item_quantity");

	lua_pushnumber(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "slot_id");

	Lua_ItemInst l_item(extra_data);
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_spell_null(
	QuestInterface *parse,
	lua_State* L,
	Mob* mob,
	Client* client,
	uint32 spell_id,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
}

void handle_encounter_timer(
	QuestInterface *parse,
	lua_State* L,
	Encounter* encounter,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_encounter_load(
	QuestInterface *parse,
	lua_State* L,
	Encounter* encounter,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	if (encounter) {
		Lua_Encounter l_enc(encounter);
		luabind::adl::object l_enc_o = luabind::adl::object(L, l_enc);
		l_enc_o.push(L);
		lua_setfield(L, -2, "encounter");
	}
	if (extra_pointers) {
		std::string *str = std::any_cast<std::string*>(extra_pointers->at(0));
		lua_pushstring(L, str->c_str());
		lua_setfield(L, -2, "data");
	}
}

void handle_encounter_unload(
	QuestInterface *parse,
	lua_State* L,
	Encounter* encounter,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	if (extra_pointers) {
		std::string *str = std::any_cast<std::string*>(extra_pointers->at(0));
		lua_pushstring(L, str->c_str());
		lua_setfield(L, -2, "data");
	}
}

void handle_encounter_null(
	QuestInterface *parse,
	lua_State* L,
	Encounter* encounter,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
}

void handle_player_skill_up(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "skill_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "skill_value");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "skill_max");

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "is_tradeskill");
}

void handle_player_language_skill_up(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "skill_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "skill_value");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "skill_max");
}

void handle_player_alt_currency_merchant(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "currency_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "npc_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "merchant_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "item_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[4]));
	lua_setfield(L, -2, "item_cost");
}

void handle_player_merchant(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "npc_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "merchant_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "item_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "item_quantity");

	lua_pushinteger(L, Strings::ToInt(sep.arg[4]));
	lua_setfield(L, -2, "item_cost");
}

void handle_player_augment_insert(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	Lua_ItemInst l_augment(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(1)));
	luabind::adl::object l_augment_o = luabind::adl::object(L, l_augment);
	l_augment_o.push(L);
	lua_setfield(L, -2, "augment");

	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToUnsignedInt(sep.arg[0]));
	lua_setfield(L, -2, "item_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "item_slot");

	lua_pushinteger(L, Strings::ToUnsignedInt(sep.arg[2]));
	lua_setfield(L, -2, "augment_id");

	lua_pushinteger(L, Strings::ToUnsignedInt(sep.arg[3]));
	lua_setfield(L, -2, "augment_slot");
}

void handle_player_augment_remove(
	QuestInterface *parse,
	lua_State* L,
	Client* client,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_ItemInst l_item(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	Lua_ItemInst l_augment(std::any_cast<EQ::ItemInstance*>(extra_pointers->at(2)));
	luabind::adl::object l_augment_o = luabind::adl::object(L, l_augment);
	l_augment_o.push(L);
	lua_setfield(L, -2, "augment");

	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToUnsignedInt(sep.arg[0]));
	lua_setfield(L, -2, "item_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "item_slot");

	lua_pushinteger(L, Strings::ToUnsignedInt(sep.arg[2]));
	lua_setfield(L, -2, "augment_id");

	lua_pushinteger(L, Strings::ToUnsignedInt(sep.arg[3]));
	lua_setfield(L, -2, "augment_slot");

	lua_pushboolean(L, Strings::ToBool(sep.arg[4]));
	lua_setfield(L, -2, "destroyed");
}

// Bot

void handle_bot_null(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
}

void handle_bot_cast(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	int spell_id = Strings::ToInt(sep.arg[0]);
	if (IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
	}

	lua_setfield(L, -2, "spell");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "caster_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "caster_level");
}

void handle_bot_combat(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushboolean(L, Strings::ToInt(data) == 0 ? false : true);
	lua_setfield(L, -2, "joined");
}

void handle_bot_death(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	Mob *o = entity_list.GetMobID(Strings::ToInt(sep.arg[0]));
	Lua_Mob l_mob(o);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "damage");

	int spell_id = Strings::ToInt(sep.arg[2]);
	if (IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}

	lua_pushinteger(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "skill");
}

void handle_bot_popup_response(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "popup_id");
}

void handle_bot_say(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "message");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "language");
}

void handle_bot_signal(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushinteger(L, Strings::ToInt(data));
	lua_setfield(L, -2, "signal");
}

void handle_bot_payload(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "payload_id");

	lua_pushstring(L, sep.argplus[1]);
	lua_setfield(L, -2, "payload_value");
}

void handle_bot_slay(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_bot_target_change(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_bot_timer(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_bot_trade(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_createtable(L, 0, 0);
	const auto bot_id = bot->GetBotID();

	if (extra_pointers) {
		size_t sz = extra_pointers->size();
		for (size_t i = 0; i < sz; ++i) {
			auto prefix = fmt::format("item{}", i + 1);
			auto* inst = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(i));

			Lua_ItemInst l_inst = inst;
			luabind::adl::object l_inst_o = luabind::adl::object(L, l_inst);
			l_inst_o.push(L);

			lua_setfield(L, -2, prefix.c_str());
		}
	}

	auto money_string = fmt::format("platinum.{}", bot_id);
	uint32 money_value = !parse->GetVar(money_string).empty() ? Strings::ToUnsignedInt(parse->GetVar(money_string)) : 0;

	lua_pushinteger(L, money_value);
	lua_setfield(L, -2, "platinum");

	money_string = fmt::format("gold.{}", bot_id);
	money_value = !parse->GetVar(money_string).empty() ? Strings::ToUnsignedInt(parse->GetVar(money_string)) : 0;

	lua_pushinteger(L, money_value);
	lua_setfield(L, -2, "gold");

	money_string = fmt::format("silver.{}", bot_id);
	money_value = !parse->GetVar(money_string).empty() ? Strings::ToUnsignedInt(parse->GetVar(money_string)) : 0;

	lua_pushinteger(L, money_value);
	lua_setfield(L, -2, "silver");

	money_string = fmt::format("copper.{}", bot_id);
	money_value = !parse->GetVar(money_string).empty() ? Strings::ToUnsignedInt(parse->GetVar(money_string)) : 0;

	lua_pushinteger(L, money_value);
	lua_setfield(L, -2, "copper");

	lua_setfield(L, -2, "trade");
}

void handle_bot_use_skill(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob *init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "skill_id");

	lua_pushinteger(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "skill_level");
}

void handle_bot_equip_item(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	lua_pushnumber(L, extra_data);
	lua_setfield(L, -2, "item_id");

	Seperator sep(data.c_str());

	lua_pushnumber(L, Strings::ToInt(sep.arg[0]));
	lua_setfield(L, -2, "item_quantity");

	lua_pushnumber(L, Strings::ToInt(sep.arg[1]));
	lua_setfield(L, -2, "slot_id");

	Lua_ItemInst l_item(extra_data);
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_bot_damage(
	QuestInterface *parse,
	lua_State* L,
	Bot* bot,
	Mob* init,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	Seperator sep(data.c_str());

	lua_pushnumber(L, Strings::ToUnsignedInt(sep.arg[0]));
	lua_setfield(L, -2, "entity_id");

	lua_pushnumber(L, Strings::ToBigInt(sep.arg[1]));
	lua_setfield(L, -2, "damage");

	lua_pushnumber(L, Strings::ToInt(sep.arg[2]));
	lua_setfield(L, -2, "spell_id");

	lua_pushnumber(L, Strings::ToInt(sep.arg[3]));
	lua_setfield(L, -2, "skill_id");

	lua_pushboolean(L, Strings::ToInt(sep.arg[4]) == 0 ? false : true);
	lua_setfield(L, -2, "is_damage_shield");

	lua_pushboolean(L, Strings::ToInt(sep.arg[5]) == 0 ? false : true);
	lua_setfield(L, -2, "is_avoidable");

	lua_pushnumber(L, Strings::ToInt(sep.arg[6]));
	lua_setfield(L, -2, "buff_slot");

	lua_pushboolean(L, Strings::ToInt(sep.arg[7]) == 0 ? false : true);
	lua_setfield(L, -2, "is_buff_tic");

	lua_pushnumber(L, Strings::ToInt(sep.arg[8]));
	lua_setfield(L, -2, "special_attack");

	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

#endif
