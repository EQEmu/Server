#ifdef LUA_EQEMU
#include <sstream>
#include <string>

#include <sol/sol.hpp>

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
void handle_npc_event_say(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	npc->DoQuestPause(init);

	Lua_Client client(reinterpret_cast<Client*>(init));
	L["other"] = client;

	L["message"] = data;

	L["language"] = extra_data;
}

void handle_npc_event_trade(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	Lua_Client client(reinterpret_cast<Client*>(init));
	L["other"] = client;

	auto ident = std::to_string(npc->GetNPCTypeID());

	auto trade = sv.create_table();

	if (extra_pointers) {
		auto sz = extra_pointers->size();
		for(size_t i = 0; i < sz; ++i) {
			std::string prefix = "item" + std::to_string(i + 1);
			EQ::ItemInstance *inst = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(i));

			trade[prefix] = inst;
		}
	}

	L["platinum"] = std::stoul(parse->GetVar("platinum." + ident));

	L["gold"] = std::stoul(parse->GetVar("gold." + ident));

	L["silver"] = std::stoul(parse->GetVar("silver." + ident));

	L["copper"] = std::stoul(parse->GetVar("copper." + ident));

	L["trade"] = trade;
}

void handle_npc_event_hp(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	if(extra_data == 1) {
		L["hp_event"] = -1;
		L["inc_hp_event"] = std::stoi(data);
	}
	else
	{
		L["hp_event"] = std::stoi(data);
		L["inc_hp_event"] = -1;
	}
}

void handle_npc_single_mob(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["other"] = init;
}

void handle_npc_single_client(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	Lua_Client client(reinterpret_cast<Client*>(init));
	L["other"] = client;
}

void handle_npc_single_npc(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	Lua_NPC l_npc(reinterpret_cast<NPC*>(init));
	L["other"] = l_npc;
}

void handle_npc_task_accepted(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	Lua_Client client(reinterpret_cast<Client*>(init));
	L["other"] = client;

	L["task_id"] = std::stoi(data);
}

void handle_npc_popup(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["other"] = init;

	L["popup_id"] = std::stoi(data);
}

void handle_npc_waypoint(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["other"] = init;

	L["wp"] = std::stoi(data);
}

void handle_npc_hate(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["other"] = init;

	L["joined"] = std::stoi(data) == 0 ? false : true;
}


void handle_npc_signal(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["signal"] = std::stoi(data);
}

void handle_npc_timer(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["timer"] = data;
}

void handle_npc_death(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["other"] = init;

	Seperator sep(data.c_str());

	L["damage"] = std::stoi(sep.arg[0]);

	int spell_id = std::stoi(sep.arg[1]);
	L["spell"] = IsValidSpell(spell_id) ? &spells[spell_id] : nullptr;

	L["skill_id"] = std::stoi(sep.arg[2]);
}

void handle_npc_cast(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	int spell_id = std::stoi(data);
	L["spell"] = IsValidSpell(spell_id) ? &spells[spell_id] : nullptr;
}

void handle_npc_area(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["area_id"] = *std::any_cast<int*>(extra_pointers->at(0));

	L["area_type"] = *std::any_cast<int*>(extra_pointers->at(1));
}

void handle_npc_null(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
}

void handle_npc_loot_zone(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
	Lua_Client client(reinterpret_cast<Client*>(init));
	L["other"] = client;

	L["item"] = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));

	L["corpse"] = std::any_cast<Corpse*>(extra_pointers->at(1));
}

//Player
void handle_player_say(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
					   std::vector<std::any> *extra_pointers)
{
	L["message"] = data;

	L["language"] = extra_data;
}

void handle_player_environmental_damage(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers)
{
	Seperator sep(data.c_str());

	L["env_damage"] = std::stoi(sep.arg[0]);

	L["env_damage_type"] = std::stoi(sep.arg[1]);

	L["env_final_damage"] = std::stoi(sep.arg[2]);
}

void handle_player_death(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						 std::vector<std::any> *extra_pointers)
{
	Seperator sep(data.c_str());

	L["other"] = entity_list.GetMobID(std::stoi(sep.arg[0]));

	L["damage"] = std::stoi(sep.arg[1]);

	int spell_id = std::stoi(sep.arg[2]);
	L["spell"] = IsValidSpell(spell_id) ? &spells[spell_id] : nullptr;

	L["skill"] = std::stoi(sep.arg[3]);
}

void handle_player_timer(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						 std::vector<std::any> *extra_pointers)
{
	L["timer"] = data;
}

void handle_player_discover_item(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
								 std::vector<std::any> *extra_pointers)
{
	L["item"] = database.GetItem(extra_data);
}

void handle_player_fish_forage_success(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
									   std::vector<std::any> *extra_pointers)
{
	L["item"] = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));
}

void handle_player_click_object(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
								std::vector<std::any> *extra_pointers)
{
	L["object"] = std::any_cast<Object*>(extra_pointers->at(0));
}

void handle_player_click_door(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
							  std::vector<std::any> *extra_pointers)
{
	L["door"] = std::any_cast<Doors*>(extra_pointers->at(0));
}

void handle_player_signal(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers)
{
	L["signal"] = std::stoi(data);
}

void handle_player_popup_response(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
								  std::vector<std::any> *extra_pointers)
{
	L["popup_id"] = std::stoi(data);
}

void handle_player_pick_up(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						   std::vector<std::any> *extra_pointers)
{
	L["item"] = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));
}

void handle_player_cast(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
	Seperator sep(data.c_str());

	int spell_id = std::stoi(sep.arg[0]);
	L["spell"] = IsValidSpell(spell_id) ? &spells[spell_id] : nullptr;

	L["caster_id"] = std::stoi(sep.arg[1]);

	L["caster_level"] = std::stoi(sep.arg[2]);
}

void handle_player_task_fail(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
							 std::vector<std::any> *extra_pointers)
{
	L["task_id"] = std::stoi(data);
}

void handle_player_zone(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
	Seperator sep(data.c_str());

	L["from_zone_id"] = std::stoi(sep.arg[0]);

	L["zone_id"] = std::stoi(sep.arg[1]);
}

void handle_player_duel_win(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
							std::vector<std::any> *extra_pointers)
{
	L["other"] = std::any_cast<Client*>(extra_pointers->at(1));
}

void handle_player_duel_loss(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
							 std::vector<std::any> *extra_pointers)
{
	L["other"] = std::any_cast<Client*>(extra_pointers->at(0));
}

void handle_player_loot(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
	L["item"] = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));

	L["corpse"] = std::any_cast<Corpse*>(extra_pointers->at(1));
}

void handle_player_task_stage_complete(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
									   std::vector<std::any> *extra_pointers)
{
	Seperator sep(data.c_str());

	L["task_id"] = std::stoi(sep.arg[0]);

	L["activity_id"] = std::stoi(sep.arg[1]);
}

void handle_player_task_update(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
								 std::vector<std::any> *extra_pointers)
{
	Seperator sep(data.c_str());

	L["count"] = std::stoi(sep.arg[0]);

	L["activity_id"] = std::stoi(sep.arg[1]);

	L["task_id"] = std::stoi(sep.arg[2]);
}

void handle_player_command(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						   std::vector<std::any> *extra_pointers)
{
	Seperator sep(data.c_str(), ' ', 10, 100, true);
	std::string command(sep.arg[0] + 1);

	L["command"] = command;

	auto table = sv.create_table();
	int max_args = sep.GetMaxArgNum();
	for (int i = 1; i < max_args; ++i) {
		if (sep.arg[i][0] != '\0') {
			table[i] = std::string(sep.arg[i]);
		}
	}

	L["args"] = table;
}

void handle_player_combine(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						   std::vector<std::any> *extra_pointers)
{
	L["recipe_id"] = extra_data;

	L["recipe_name"] = data;
}

void handle_player_feign(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
	L["other"] = std::any_cast<NPC*>(extra_pointers->at(0));
}

void handle_player_area(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
	L["area_id"] = *std::any_cast<int*>(extra_pointers->at(0));

	L["area_type"] = *std::any_cast<int*>(extra_pointers->at(1));
}

void handle_player_respawn(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
	L["option"] = std::stoi(data);

	L["resurrect"] = extra_data == 1 ? true : false;
}

void handle_player_packet(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
	L["packet"] = std::any_cast<EQApplicationPacket*>(extra_pointers->at(0));

	L["connecting"] = extra_data == 1 ? true : false;
}

void handle_player_null(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
						std::vector<std::any> *extra_pointers)
{
}

void handle_player_use_skill(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers)
{
	Seperator sep(data.c_str());

	L["skill_id"] = std::stoi(sep.arg[0]);

	L["skill_level"] = std::stoi(sep.arg[1]);
}

void handle_test_buff(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any>* extra_pointers)
{
}

void handle_player_combine_validate(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
									std::vector<std::any>* extra_pointers)
{
	Seperator sep(data.c_str());

	L["recipe_id"] = extra_data;

	L["validate_type"] = sep.arg[0];

	int zone_id = -1;
	int tradeskill_id = -1;
	if (strcmp(sep.arg[0], "check_zone") == 0) {
		zone_id = std::stoi(sep.arg[1]);
	} else if (strcmp(sep.arg[0], "check_tradeskill") == 0) {
		tradeskill_id = std::stoi(sep.arg[1]);
	}

	L["zone_id"] = zone_id;

	L["tradeskill_id"] = tradeskill_id;
}

void handle_player_bot_command(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers)
{
	Seperator sep(data.c_str(), ' ', 10, 100, true);
	std::string bot_command(sep.arg[0] + 1);

	L["bot_command"] = bot_command;

	auto table = sv.create_table();
	int max_args = sep.GetMaxArgNum();
	for (int i = 1; i < max_args; ++i) {
		if (sep.arg[i][0] != '\0') {
			table[i] = std::string(sep.arg[i]);
		}
	}

	L["args"] = table;
}

void handle_player_warp(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any>* extra_pointers)
{
	Seperator sep(data.c_str());

	L["from_x"] = std::stof(sep.arg[0]);

	L["from_y"] = std::stof(sep.arg[1]);

	L["from_z"] = std::stof(sep.arg[2]);
}

void handle_player_quest_combine(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any>* extra_pointers)
{
	L["container_slot"] = std::stoi(data);
 }
 
void handle_player_consider(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any>* extra_pointers)
{
	L["entity_id"] = std::stoi(data);
}

void handle_player_consider_corpse(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any>* extra_pointers)
{
	L["corpse_entity_id"] = std::stoi(data);
}

//Item
void handle_item_click(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<std::any> *extra_pointers)
{
	L["slot_id"] = extra_data;
}

void handle_item_timer(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<std::any> *extra_pointers)
{
	L["timer"] = data;
}

void handle_item_proc(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<std::any> *extra_pointers)
{
	L["target"]= mob;

	L["spell"] = IsValidSpell(extra_data) ? &spells[extra_data] : nullptr;
}

void handle_item_loot(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<std::any> *extra_pointers)
{
	L["corpse"] = mob && mob->IsCorpse() ? mob->CastToCorpse() : nullptr;
}

void handle_item_equip(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					   std::vector<std::any> *extra_pointers)
{
	L["slot_id"] = extra_data;
}

void handle_item_augment(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<std::any> *extra_pointers)
{
	L["aug"] = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));

	L["slot_id"] = extra_data;
}

void handle_item_augment_insert(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<std::any> *extra_pointers)
{
	L["item"] = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));

	L["slot_id"] = extra_data;
}

void handle_item_augment_remove(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<std::any> *extra_pointers)
{
	L["item"] = std::any_cast<EQ::ItemInstance*>(extra_pointers->at(0));

	L["slot_id"] = extra_data;

	L["destroyed"] = *std::any_cast<bool*>(extra_pointers->at(1));
}

void handle_item_null(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
					  std::vector<std::any> *extra_pointers)
{
}

//Spell
void handle_spell_event(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers)
{
	Lua_Mob mob(nullptr);
	if (npc) {
		mob = reinterpret_cast<Mob*>(npc);
	} else if (client) {
		mob = reinterpret_cast<Mob*>(client);
	}

	L["target"] = mob;

	L["spell_id"] = spell_id;

	Seperator sep(data.c_str());

	L["caster_id"] = std::stoi(sep.arg[0]);

	L["tics_remaining"] = std::stoi(sep.arg[1]);

	L["caster_level"] = std::stoi(sep.arg[2]);

	L["buff_slot"] = std::stoi(sep.arg[3]);

	L["spell"] = IsValidSpell(spell_id) ? &spells[spell_id] : nullptr;
}

void handle_translocate_finish(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers)
{
	if (npc) {
		L["target"] = reinterpret_cast<Mob*>(npc);
	} else if (client) {
		L["target"] = reinterpret_cast<Mob*>(client);
	} else {
		Mob *mob = nullptr;
		L["target"] = mob;
	}
}

void handle_player_equip_item(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers)
{
	L["item_id"] = extra_data;

	Seperator sep(data.c_str());

	L["item_quantity"] = std::stoi(sep.arg[0]);

	L["slot_id"] = std::stoi(sep.arg[1]);

	L["item"] = extra_data;
}

void handle_spell_null(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers)
{
}

void handle_encounter_timer(QuestInterface *parse, sol::state_view sv, sol::table& L, Encounter* encounter, std::string data, uint32 extra_data,
							std::vector<std::any> *extra_pointers)
{
	L["timer"] = data;
}

void handle_encounter_load(QuestInterface *parse, sol::state_view sv, sol::table& L, Encounter* encounter, std::string data, uint32 extra_data,
									 std::vector<std::any> *extra_pointers)
{
	if (encounter) {
		L["encounter"] = encounter;
	}

	if (extra_pointers) {
		std::string *str = std::any_cast<std::string*>(extra_pointers->at(0));
		L["data"] = *str;
	}
}

void handle_encounter_unload(QuestInterface *parse, sol::state_view sv, sol::table& L, Encounter* encounter, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers)
{
	if (extra_pointers) {
		std::string *str = std::any_cast<std::string*>(extra_pointers->at(0));
		L["data"] = *str;
	}
}

void handle_encounter_null(QuestInterface *parse, sol::state_view sv, sol::table& L, Encounter* encounter, std::string data, uint32 extra_data,
						   std::vector<std::any> *extra_pointers)
{
}

void handle_player_skill_up(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any>* extra_pointers)
{
	Seperator sep(data.c_str());

	L["skill_id"] = std::stoi(sep.arg[0]);

	L["skill_value"] = std::stoi(sep.arg[1]);

	L["skill_max"] = std::stoi(sep.arg[2]);

	L["is_tradeskill"] = std::stoi(sep.arg[3]);
}

void handle_player_language_skill_up(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data, std::vector<std::any>* extra_pointers)
{
	Seperator sep(data.c_str());

	L["skill_id"] = std::stoi(sep.arg[0]);

	L["skill_value"] = std::stoi(sep.arg[1]);

	L["skill_max"] = std::stoi(sep.arg[2]);
}

#endif
