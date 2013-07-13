#ifdef LUA_EQEMU
#include <sstream>

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "QuestParserCollection.h"
#include "QuestInterface.h"

#include "masterentity.h"
#include "../common/seperator.h"
#include "../common/MiscFunctions.h"
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
#include "zone.h"
#include "lua_parser_events.h"

//NPC
void handle_npc_event_say(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	npc->DoQuestPause(init);

	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "message");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "language");
}

void handle_npc_event_trade(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
	
	lua_createtable(L, 0, 0);
	std::stringstream ident;
	ident << npc->GetNPCTypeID();
	
	if(extra_pointers) {
		for(size_t i = 0; i < extra_pointers->size(); ++i) {
			std::string prefix = "item" + std::to_string(static_cast<long long>(i + 1));
			Lua_ItemInst l_inst = reinterpret_cast<ItemInst*>(extra_pointers->at(i));
			luabind::object l_inst_o = luabind::object(L, l_inst);
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
						  std::vector<void*> *extra_pointers) {
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
						  std::vector<void*> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_single_client(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_single_npc(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	Lua_NPC l_npc(reinterpret_cast<NPC*>(init));
	luabind::object l_npc_o = luabind::object(L, l_npc);
	l_npc_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_npc_task_accepted(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(init));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "task_id");
}

void handle_npc_popup(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "popup_id");
}

void handle_npc_waypoint(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "wp");
}

void handle_npc_hate(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushboolean(L, std::stoi(data) == 0 ? false : true);
	lua_setfield(L, -2, "joined");
}


void handle_npc_signal(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "signal");
}

void handle_npc_timer(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_npc_death(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	Lua_Mob l_mob(init);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "damage");

	int spell_id = std::stoi(sep.arg[1]);
	if(IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "skill_id");
}

void handle_npc_cast(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	int spell_id = std::stoi(data);
	if(IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}
}

void handle_npc_area(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, *reinterpret_cast<int*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "area_id");

	lua_pushinteger(L, *reinterpret_cast<int*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "area_type");
}

void handle_npc_null(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
}

//Player
void handle_player_say(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
					   std::vector<void*> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "message");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "language");
}

void handle_player_death(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						 std::vector<void*> *extra_pointers) {
	Seperator sep(data.c_str());

	Mob *o = entity_list.GetMobID(std::stoi(sep.arg[0]));
	Lua_Mob l_mob(o);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "other");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "damage");

	int spell_id = std::stoi(sep.arg[2]);
	if(IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}

	lua_pushinteger(L, std::stoi(sep.arg[3]));
	lua_setfield(L, -2, "skill");
}

void handle_player_timer(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						 std::vector<void*> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_player_discover_item(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
								 std::vector<void*> *extra_pointers) {
	const Item_Struct *item = database.GetItem(extra_data);
	if(item) {
		Lua_Item l_item(item);
		luabind::object l_item_o = luabind::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "item");
	} else {
		Lua_Item l_item(nullptr);
		luabind::object l_item_o = luabind::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "item");
	}
}

void handle_player_fish_forage_success(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
									   std::vector<void*> *extra_pointers) {
	Lua_ItemInst l_item(reinterpret_cast<ItemInst*>(extra_pointers->at(0)));
	luabind::object l_item_o = luabind::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_player_click_object(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
								std::vector<void*> *extra_pointers) {
	Lua_Object l_object(reinterpret_cast<Object*>(extra_pointers->at(0)));
	luabind::object l_object_o = luabind::object(L, l_object);
	l_object_o.push(L);
	lua_setfield(L, -2, "object");
}

void handle_player_click_door(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
							  std::vector<void*> *extra_pointers) {
	Lua_Door l_door(reinterpret_cast<Doors*>(extra_pointers->at(0)));
	luabind::object l_door_o = luabind::object(L, l_door);
	l_door_o.push(L);
	lua_setfield(L, -2, "door");
}

void handle_player_signal(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						  std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "signal");
}

void handle_player_popup_response(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
								  std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "popup_id");
}

void handle_player_pick_up(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						   std::vector<void*> *extra_pointers) {
	Lua_ItemInst l_item(reinterpret_cast<ItemInst*>(extra_pointers->at(0)));
	luabind::object l_item_o = luabind::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");
}

void handle_player_cast(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<void*> *extra_pointers) {
	int spell_id = std::stoi(data);
	if(IsValidSpell(spell_id)) {
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
	}

	lua_setfield(L, -2, "spell");
}

void handle_player_task_fail(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
							 std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "task_id");
}

void handle_player_zone(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "zone_id");
}

void handle_player_duel_win(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
							std::vector<void*> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(extra_pointers->at(1)));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_duel_loss(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
							 std::vector<void*> *extra_pointers) {
	Lua_Client l_client(reinterpret_cast<Client*>(extra_pointers->at(0)));
	luabind::object l_client_o = luabind::object(L, l_client);
	l_client_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_loot(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<void*> *extra_pointers) {
	Lua_ItemInst l_item(reinterpret_cast<ItemInst*>(extra_pointers->at(0)));
	luabind::object l_item_o = luabind::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	Lua_Corpse l_corpse(reinterpret_cast<Corpse*>(extra_pointers->at(1)));
	luabind::object l_corpse_o = luabind::object(L, l_corpse);
	l_corpse_o.push(L);
	lua_setfield(L, -2, "corpse");
}

void handle_player_task_stage_complete(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
									   std::vector<void*> *extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "task_id");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "activity_id");
}

void handle_player_task_update(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
								 std::vector<void*> *extra_pointers) {
	Seperator sep(data.c_str());
	lua_pushinteger(L, std::stoi(sep.arg[0]));
	lua_setfield(L, -2, "count");

	lua_pushinteger(L, std::stoi(sep.arg[1]));
	lua_setfield(L, -2, "activity_id");

	lua_pushinteger(L, std::stoi(sep.arg[2]));
	lua_setfield(L, -2, "task_id");
}

void handle_player_command(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						   std::vector<void*> *extra_pointers) {
	Seperator sep(data.c_str(), ' ', 10, 100, true);
	std::string command(sep.arg[0] + 1);
	lua_pushstring(L, command.c_str());
	lua_setfield(L, -2, "command");

	luabind::object args = luabind::newtable(L);
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
						   std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "recipe_id");

	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "recipe_name");	
}

void handle_player_feign(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<void*> *extra_pointers) {
	Lua_NPC l_npc(reinterpret_cast<NPC*>(extra_pointers->at(0)));
	luabind::object l_npc_o = luabind::object(L, l_npc);
	l_npc_o.push(L);
	lua_setfield(L, -2, "other");
}

void handle_player_area(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, *reinterpret_cast<int*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "area_id");

	lua_pushinteger(L, *reinterpret_cast<int*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "area_type");
}

void handle_player_respawn(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, std::stoi(data));
	lua_setfield(L, -2, "option");

	lua_pushboolean(L, extra_data == 1 ? true : false);
	lua_setfield(L, -2, "resurrect");
}

void handle_player_packet(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<void*> *extra_pointers) {
	Lua_Packet l_packet(reinterpret_cast<EQApplicationPacket*>(extra_pointers->at(0)));
	luabind::object l_packet_o = luabind::object(L, l_packet);
	l_packet_o.push(L);
	lua_setfield(L, -2, "packet");

	lua_pushboolean(L, extra_data == 1 ? true : false);
	lua_setfield(L, -2, "connecting");
}

void handle_player_null(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
						std::vector<void*> *extra_pointers) {
}

//Item
void handle_item_click(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_timer(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<void*> *extra_pointers) {
	lua_pushstring(L, data.c_str());
	lua_setfield(L, -2, "timer");
}

void handle_item_proc(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<void*> *extra_pointers) {

	Lua_Mob l_mob(mob);
	luabind::object l_mob_o = luabind::object(L, l_mob);
	l_mob_o.push(L);
	lua_setfield(L, -2, "target");

	if(IsValidSpell(extra_data)) {
		Lua_Spell l_spell(&spells[extra_data]);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	} else {
		Lua_Spell l_spell(nullptr);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "spell");
	}
}

void handle_item_loot(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<void*> *extra_pointers) {
	if(mob && mob->IsCorpse()) {
		Lua_Corpse l_corpse(mob->CastToCorpse());
		luabind::object l_corpse_o = luabind::object(L, l_corpse);
		l_corpse_o.push(L);
		lua_setfield(L, -2, "corpse");
	} else {
		Lua_Corpse l_corpse(nullptr);
		luabind::object l_corpse_o = luabind::object(L, l_corpse);
		l_corpse_o.push(L);
		lua_setfield(L, -2, "corpse");
	}
}

void handle_item_equip(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<void*> *extra_pointers) {
	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<void*> *extra_pointers) {
	Lua_ItemInst l_item(reinterpret_cast<ItemInst*>(extra_pointers->at(0)));
	luabind::object l_item_o = luabind::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "aug");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment_insert(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<void*> *extra_pointers) {
	Lua_ItemInst l_item(reinterpret_cast<ItemInst*>(extra_pointers->at(0)));
	luabind::object l_item_o = luabind::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");
}

void handle_item_augment_remove(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<void*> *extra_pointers) {
	Lua_ItemInst l_item(reinterpret_cast<ItemInst*>(extra_pointers->at(0)));
	luabind::object l_item_o = luabind::object(L, l_item);
	l_item_o.push(L);
	lua_setfield(L, -2, "item");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "slot_id");

	lua_pushboolean(L, *reinterpret_cast<bool*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "destroyed");
}

void handle_item_null(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<void*> *extra_pointers) {
}

//Spell
void handle_spell_effect(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, uint32 extra_data,
						 std::vector<void*> *extra_pointers) {
	if(npc) {
		Lua_Mob l_npc(npc);
		luabind::object l_npc_o = luabind::object(L, l_npc);
		l_npc_o.push(L);
	} else if(client) {
		Lua_Mob l_client(client);
		luabind::object l_client_o = luabind::object(L, l_client);
		l_client_o.push(L);
	} else {
		Lua_Mob l_mob(nullptr);
		luabind::object l_mob_o = luabind::object(L, l_mob);
		l_mob_o.push(L);
	}

	lua_setfield(L, -2, "target");

	lua_pushinteger(L, *reinterpret_cast<int*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "buff_slot");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "caster_id");
}

void handle_spell_tic(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, uint32 extra_data,
						 std::vector<void*> *extra_pointers) {
	if(npc) {
		Lua_Mob l_npc(npc);
		luabind::object l_npc_o = luabind::object(L, l_npc);
		l_npc_o.push(L);
	} else if(client) {
		Lua_Mob l_client(client);
		luabind::object l_client_o = luabind::object(L, l_client);
		l_client_o.push(L);
	} else {
		Lua_Mob l_mob(nullptr);
		luabind::object l_mob_o = luabind::object(L, l_mob);
		l_mob_o.push(L);
	}

	lua_setfield(L, -2, "target");

	lua_pushinteger(L, *reinterpret_cast<int*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "tics_remaining");

	lua_pushinteger(L, *reinterpret_cast<uint8*>(extra_pointers->at(1)));
	lua_setfield(L, -2, "caster_level");

	lua_pushinteger(L, *reinterpret_cast<int*>(extra_pointers->at(2)));
	lua_setfield(L, -2, "buff_slot");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "caster_id");
}

void handle_spell_fade(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, uint32 extra_data,
					   std::vector<void*> *extra_pointers) {
	if(npc) {
		Lua_Mob l_npc(npc);
		luabind::object l_npc_o = luabind::object(L, l_npc);
		l_npc_o.push(L);
	} else if(client) {
		Lua_Mob l_client(client);
		luabind::object l_client_o = luabind::object(L, l_client);
		l_client_o.push(L);
	} else {
		Lua_Mob l_mob(nullptr);
		luabind::object l_mob_o = luabind::object(L, l_mob);
		l_mob_o.push(L);
	}

	lua_setfield(L, -2, "target");

	lua_pushinteger(L, extra_data);
	lua_setfield(L, -2, "buff_slot");

	lua_pushinteger(L, *reinterpret_cast<uint16*>(extra_pointers->at(0)));
	lua_setfield(L, -2, "caster_id");
}

void handle_translocate_finish(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, uint32 extra_data,
					   std::vector<void*> *extra_pointers) {
	if(npc) {
		Lua_Mob l_npc(npc);
		luabind::object l_npc_o = luabind::object(L, l_npc);
		l_npc_o.push(L);
	} else if(client) {
		Lua_Mob l_client(client);
		luabind::object l_client_o = luabind::object(L, l_client);
		l_client_o.push(L);
	} else {
		Lua_Mob l_mob(nullptr);
		luabind::object l_mob_o = luabind::object(L, l_mob);
		l_mob_o.push(L);
	}

	lua_setfield(L, -2, "target");
}

void handle_spell_null(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, uint32 extra_data,
					   std::vector<void*> *extra_pointers) {
}

#endif
