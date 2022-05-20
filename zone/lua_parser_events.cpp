#ifdef LUA_EQEMU
#include <sstream>

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "quest_parser_collection.h"
#include "quest_interface.h"

#include "masterentity.h"
#include "../common/seperator.h"
#include "../common/misc_functions.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_entity.h"
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
void handle_npc_event_say(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
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

void handle_npc_event_trade(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
	
	lua_createtable(L, 0, 0);
	std::stringstream ident;
	ident << npc->GetNPCTypeID();
	
	if(extra_pointers) {
		size_t sz = extra_pointers->size();
		for(size_t i = 0; i < sz; ++i) {
			std::string prefix = "item" + std::to_string(i + 1);
			EQ::ItemInstance *inst = EQ::any_cast<EQ::ItemInstance*>(extra_pointers->at(i));

			Lua_ItemInst l_inst = inst;
			luabind::adl::object l_inst_o = luabind::adl::object(L, l_inst);
			l_inst_o.push(L);

			lua_setfield(L, -2, prefix.c_str());
		}
	}

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

void handle_npc_event_hp(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
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

void handle_npc_single_mob(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_single_client(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_single_npc(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_NPC l_npc(reinterpret_cast<NPC*>(init));
	luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
	l_npc_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_task_accepted(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "task_id");
}

void handle_npc_popup(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "popup_id");
}

void handle_npc_waypoint(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "wp");
}

void handle_npc_hate(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushboolean(L, std::stoi(data) == 0 ? false : true);
	lua_setfield(L, -2, "joined");
}


void handle_npc_signal(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "signal");
}

void handle_npc_timer(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_npc_death(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "damage");

	int spell_id = std::stoi(sep.arg[1]);
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

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "skill_id");
}

void handle_npc_cast(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	int spell_id = std::stoi(data);
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

void handle_npc_area(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, *EQ::any_cast<int*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "area_id");

	lua_pushinteger(L, *EQ::any_cast<int*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "area_type");
}

void handle_npc_null(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
}

void handle_npc_loot_zone(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
	
	Lua_ItemInst l_item(EQ::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
	
	Lua_Corpse l_corpse(EQ::any_cast<Corpse*>(extra_pointers->at(1)));
	luabind::adl::object l_corpse_o = luabind::adl::object(L, l_corpse);
	l_corpse_o.push(L);
	lua_setfield(L, -2, "corpse");
}

//Player
void handle_player_say(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
					   std::vector<EQ::Any> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "message");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "language");
}

void handle_player_environmental_damage(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any> *extra_pointers){
	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "env_damage");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "env_damage_type");

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "env_final_damage");
}

void handle_player_death(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						 std::vector<EQ::Any> *extra_pointers) {
	Seperator sep(data.c_str());

	Mob *o = entity_list.GetMobID(std::stoi(sep.arg[0]));
	Lua_Mob l_mob(o);
	luabind::adl::object l_mob_o = luabind::adl::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "damage");

	int spell_id = std::stoi(sep.arg[2]);
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

	lua_pushinteger(L, std::stoi(sep.arg[3]));
	lua_setfield(L, -2, "skill");
}

void handle_player_timer(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						 std::vector<EQ::Any> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_player_discover_item(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
								 std::vector<EQ::Any> *extra_pointers) {
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

void handle_player_fish_forage_success(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
									   std::vector<EQ::Any> *extra_pointers) {
	Lua_ItemInst l_item(EQ::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_player_click_object(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
								std::vector<EQ::Any> *extra_pointers) {
	Lua_Object l_object(EQ::any_cast<Object*>(extra_pointers->at(0)));
	luabind::adl::object l_object_o = luabind::adl::object(L, l_object);
	l_object_o.push(L);
	lua_setfield(L, -2, "object");
}

void handle_player_click_door(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
							  std::vector<EQ::Any> *extra_pointers) {
	Lua_Door l_door(EQ::any_cast<Doors*>(extra_pointers->at(0)));
	luabind::adl::object l_door_o = luabind::adl::object(L, l_door);
	l_door_o.push(L);
	lua_setfield(L, -2, "door");
}

void handle_player_signal(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "signal");
}

void handle_player_popup_response(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
								  std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "popup_id");
}

void handle_player_pick_up(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						   std::vector<EQ::Any> *extra_pointers) {
	Lua_ItemInst l_item(EQ::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_player_cast(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
	Seperator sep(data.c_str());

	int spell_id = std::stoi(sep.arg[0]);
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
	
	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "caster_id");
	
	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "caster_level");
}

void handle_player_task_fail(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
							 std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "task_id");
}

void handle_player_zone(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
	Seperator sep(data.c_str());

	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "from_zone_id");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "zone_id");
}

void handle_player_duel_win(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
							std::vector<EQ::Any> *extra_pointers) {
	Lua_Client l_client(EQ::any_cast<Client*>(extra_pointers->at(1)));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_duel_loss(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
							 std::vector<EQ::Any> *extra_pointers) {
	Lua_Client l_client(EQ::any_cast<Client*>(extra_pointers->at(0)));
	luabind::adl::object l_client_o = luabind::adl::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_loot(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
	Lua_ItemInst l_item(EQ::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	Lua_Corpse l_corpse(EQ::any_cast<Corpse*>(extra_pointers->at(1)));
	luabind::adl::object l_corpse_o = luabind::adl::object(L, l_corpse);
	l_corpse_o.push(L);
	lua_setfield(L, -2, "corpse");
}

void handle_player_task_stage_complete(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
									   std::vector<EQ::Any> *extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "task_id");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "activity_id");
}

void handle_player_task_update(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
								 std::vector<EQ::Any> *extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "count");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "activity_id");

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "task_id");
}

void handle_player_command(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						   std::vector<EQ::Any> *extra_pointers) {
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

void handle_player_combine(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						   std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "recipe_id");

	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "recipe_name");	
}

void handle_player_feign(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
	Lua_NPC l_npc(EQ::any_cast<NPC*>(extra_pointers->at(0)));
	luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
	l_npc_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_area(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, *EQ::any_cast<int*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "area_id");

	lua_pushinteger(L, *EQ::any_cast<int*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "area_type");
}

void handle_player_respawn(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "option");

	lua_pushboolean(L, extra_data == 1 ? true : false);
	lua_setfield(L, -2, "resurrect");
}

void handle_player_packet(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
	Lua_Packet l_packet(EQ::any_cast<EQApplicationPacket*>(extra_pointers->at(0)));
	luabind::adl::object l_packet_o = luabind::adl::object(L, l_packet);
	l_packet_o.push(L);
	lua_setfield(L, -2, "packet");

	lua_pushboolean(L, extra_data == 1 ? true : false);
	lua_setfield(L, -2, "connecting");
}

void handle_player_null(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<EQ::Any> *extra_pointers) {
}

void handle_player_use_skill(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any> *extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "skill_id");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "skill_level");
}

void handle_test_buff(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any>* extra_pointers) {
}

void handle_player_combine_validate(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
									std::vector<EQ::Any>* extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "recipe_id");

	lua_pushstring(L, sep.arg[0]);
	lua_setfield(L, -2, "validate_type");

	int zone_id = -1;
	int tradeskill_id = -1;
	if (strcmp(sep.arg[0], "check_zone") == 0) {
		zone_id = std::stoi(sep.arg[1]);
	}
	else if (strcmp(sep.arg[0], "check_tradeskill") == 0) {
		tradeskill_id = std::stoi(sep.arg[1]);
	}

	lua_pushinteger(L, zone_id);
	lua_setfield(L, -2, "zone_id");

	lua_pushinteger(L, tradeskill_id);
	lua_setfield(L, -2, "tradeskill_id");
}

void handle_player_bot_command(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any>* extra_pointers) {
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

void handle_player_warp(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any>* extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushnumber(L, std::stof(sep.arg[0]));
	lua_setfield(L, -2, "from_x");

	lua_pushnumber(L, std::stof(sep.arg[1]));
	lua_setfield(L, -2, "from_y");

	lua_pushnumber(L, std::stof(sep.arg[2]));
	lua_setfield(L, -2, "from_z");
}

void handle_player_quest_combine(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any>* extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "container_slot");
 }
 
void handle_player_consider(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any>* extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "entity_id");
}

void handle_player_consider_corpse(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any>* extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "corpse_entity_id");
}

//Item
void handle_item_click(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_timer(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<EQ::Any> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_item_proc(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<EQ::Any> *extra_pointers) {

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

void handle_item_loot(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<EQ::Any> *extra_pointers) {
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

void handle_item_equip(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<EQ::Any> *extra_pointers) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<EQ::Any> *extra_pointers) {
	Lua_ItemInst l_item(EQ::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "aug");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment_insert(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<EQ::Any> *extra_pointers) {
	Lua_ItemInst l_item(EQ::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment_remove(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<EQ::Any> *extra_pointers) {
	Lua_ItemInst l_item(EQ::any_cast<EQ::ItemInstance*>(extra_pointers->at(0)));
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");

	lua_pushboolean(L, *EQ::any_cast<bool*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "destroyed");
}

void handle_item_null(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<EQ::Any> *extra_pointers) {
}

//Spell
void handle_spell_event(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data, std::vector<EQ::Any> *extra_pointers) {
	if(npc) {
		Lua_Mob l_npc(npc);
		luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
		l_npc_o.push(L);
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

	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "caster_id");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "tics_remaining");

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "caster_level");

	lua_pushinteger(L, std::stoi(sep.arg[3]));
	lua_setfield(L, -2, "buff_slot");
	
	Lua_Spell l_spell(spell_id);
	luabind::adl::object l_spell_o = luabind::adl::object(L, l_spell);
	l_spell_o.push(L);
	lua_setfield(L, -2, "spell");
}

void handle_translocate_finish(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data, std::vector<EQ::Any> *extra_pointers) {
	if(npc) {
		Lua_Mob l_npc(npc);
		luabind::adl::object l_npc_o = luabind::adl::object(L, l_npc);
		l_npc_o.push(L);
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

void handle_player_equip_item(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any> *extra_pointers) {
	lua_pushnumber(L, extra_data);
	lua_setfield(L, -2, "item_id");

	Seperator sep(data.c_str());

	lua_pushnumber(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "item_quantity");

	lua_pushnumber(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "slot_id");
	
	Lua_ItemInst l_item(extra_data);
	luabind::adl::object l_item_o = luabind::adl::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_spell_null(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data, std::vector<EQ::Any> *extra_pointers) { }

void handle_encounter_timer(QuestInterface *parse, lua_State* L, Encounter* encounter, std::string data, uint32 extra_data,
							std::vector<EQ::Any> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_encounter_load(QuestInterface *parse, lua_State* L, Encounter* encounter, std::string data, uint32 extra_data,
									 std::vector<EQ::Any> *extra_pointers) {
	if (encounter) {
		Lua_Encounter l_enc(encounter);
		luabind::adl::object l_enc_o = luabind::adl::object(L, l_enc);
		l_enc_o.push(L);
		lua_setfield(L, -2, "encounter");
	}
	if (extra_pointers) {
		std::string *str = EQ::any_cast<std::string*>(extra_pointers->at(0));
		lua_pushstring(L, str->c_str());
		lua_setfield(L, -2, "data");
	}
}

void handle_encounter_unload(QuestInterface *parse, lua_State* L, Encounter* encounter, std::string data, uint32 extra_data,
	std::vector<EQ::Any> *extra_pointers) {
	if (extra_pointers) {
		std::string *str = EQ::any_cast<std::string*>(extra_pointers->at(0));
		lua_pushstring(L, str->c_str());
		lua_setfield(L, -2, "data");
	}
}

void handle_encounter_null(QuestInterface *parse, lua_State* L, Encounter* encounter, std::string data, uint32 extra_data,
						   std::vector<EQ::Any> *extra_pointers) {

}

void handle_player_skill_up(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any>* extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "skill_id");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "skill_value");

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "skill_max");

	lua_pushinteger(L, std::stoi(sep.arg[3]));
	lua_setfield(L, -2, "is_tradeskill");
}

void handle_player_language_skill_up(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data, std::vector<EQ::Any>* extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "skill_id");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "skill_value");

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "skill_max");
}

#endif
