#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include <ctype.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

#include "../common/spdat.h"
#include "masterentity.h"
#include "questmgr.h"
#include "zone.h"
#include "zone_config.h"

#include "lua_parser.h"
#include "lua_bit.h"
#include "lua_entity.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_hate_list.h"
#include "lua_client.h"
#include "lua_inventory.h"
#include "lua_npc.h"
#include "lua_spell.h"
#include "lua_entity_list.h"
#include "lua_group.h"
#include "lua_raid.h"
#include "lua_corpse.h"
#include "lua_object.h"
#include "lua_door.h"
#include "lua_spawn.h"
#include "lua_packet.h"
#include "lua_general.h"
#include "lua_encounter.h"
#include "lua_stat_bonuses.h"

const char *LuaEvents[_LargestEventID] = {
	"event_say",
	"event_trade",
	"event_death",
	"event_spawn",
	"event_attack",
	"event_combat",
	"event_aggro",
	"event_slay",
	"event_npc_slay",
	"event_waypoint_arrive",
	"event_waypoint_depart",
	"event_timer",
	"event_signal",
	"event_hp",
	"event_enter",
	"event_exit",
	"event_enter_zone",
	"event_click_door",
	"event_loot",
	"event_zone",
	"event_level_up",
	"event_killed_merit",
	"event_cast_on",
	"event_task_accepted",
	"event_task_stage_complete",
	"event_task_update",
	"event_task_complete",
	"event_task_fail",
	"event_aggro_say",
	"event_player_pickup",
	"event_popup_response",
	"event_environmental_damage",
	"event_proximity_say",
	"event_cast",
	"event_cast_begin",
	"event_scale_calc",
	"event_item_enter_zone",
	"event_target_change",
	"event_hate_list",
	"event_spell_effect",
	"event_spell_effect",
	"event_spell_buff_tic",
	"event_spell_buff_tic",
	"event_spell_fade",
	"event_spell_effect_translocate_complete",
	"event_combine_success",
	"event_combine_failure",
	"event_item_click",
	"event_item_click_cast",
	"event_group_change",
	"event_forage_success",
	"event_forage_failure",
	"event_fish_start",
	"event_fish_success",
	"event_fish_failure",
	"event_click_object",
	"event_discover_item",
	"event_disconnect",
	"event_connect",
	"event_item_tick",
	"event_duel_win",
	"event_duel_lose",
	"event_encounter_load",
	"event_encounter_unload",
	"event_command",
	"event_drop_item",
	"event_destroy_item",
	"event_feign_death",
	"event_weapon_proc",
	"event_equip_item",
	"event_unequip_item",
	"event_augment_item",
	"event_unaugment_item",
	"event_augment_insert",
	"event_augment_remove",
	"event_enter_area",
	"event_leave_area",
	"event_respawn",
	"event_death_complete",
	"event_unhandled_opcode",
	"event_tick",
	"event_spawn_zone",
	"event_death_zone",
	"event_use_skill"
};

extern Zone *zone;

struct LuaParser::Implementation
{
	std::map<std::string, std::string> vars;
	std::map<std::string, bool> loaded;
	std::vector<LuaMod> mods;
	lua_State *state;
	NPCArgumentHandler NPCArgumentDispatch[_LargestEventID];
	PlayerArgumentHandler PlayerArgumentDispatch[_LargestEventID];
	ItemArgumentHandler ItemArgumentDispatch[_LargestEventID];
	SpellArgumentHandler SpellArgumentDispatch[_LargestEventID];
	EncounterArgumentHandler EncounterArgumentDispatch[_LargestEventID];
};

struct lua_registered_event {
	std::string encounter_name;
	luabind::adl::object lua_reference;
	QuestEventID event_id;
};

std::map<std::string, std::list<lua_registered_event>> lua_encounter_events_registered;
std::map<std::string, bool> lua_encounters_loaded;
std::map<std::string, Encounter *> lua_encounters;

LuaParser::LuaParser() {
	mImpl.reset(new Implementation());

	for(int i = 0; i < _LargestEventID; ++i) {
		mImpl->NPCArgumentDispatch[i] = handle_npc_null;
		mImpl->PlayerArgumentDispatch[i] = handle_player_null;
		mImpl->ItemArgumentDispatch[i] = handle_item_null;
		mImpl->SpellArgumentDispatch[i] = handle_spell_null;
		mImpl->EncounterArgumentDispatch[i] = handle_encounter_null;
	}

	mImpl->NPCArgumentDispatch[EVENT_SAY] = handle_npc_event_say;
	mImpl->NPCArgumentDispatch[EVENT_AGGRO_SAY] = handle_npc_event_say;
	mImpl->NPCArgumentDispatch[EVENT_PROXIMITY_SAY] = handle_npc_event_say;
	mImpl->NPCArgumentDispatch[EVENT_TRADE] = handle_npc_event_trade;
	mImpl->NPCArgumentDispatch[EVENT_HP] = handle_npc_event_hp;
	mImpl->NPCArgumentDispatch[EVENT_TARGET_CHANGE] = handle_npc_single_mob;
	mImpl->NPCArgumentDispatch[EVENT_CAST_ON] = handle_npc_cast;
	mImpl->NPCArgumentDispatch[EVENT_KILLED_MERIT] = handle_npc_single_client;
	mImpl->NPCArgumentDispatch[EVENT_SLAY] = handle_npc_single_mob;
	mImpl->NPCArgumentDispatch[EVENT_ENTER] = handle_npc_single_client;
	mImpl->NPCArgumentDispatch[EVENT_EXIT] = handle_npc_single_client;
	mImpl->NPCArgumentDispatch[EVENT_TASK_ACCEPTED] = handle_npc_task_accepted;
	mImpl->NPCArgumentDispatch[EVENT_POPUP_RESPONSE] = handle_npc_popup;
	mImpl->NPCArgumentDispatch[EVENT_WAYPOINT_ARRIVE] = handle_npc_waypoint;
	mImpl->NPCArgumentDispatch[EVENT_WAYPOINT_DEPART] = handle_npc_waypoint;
	mImpl->NPCArgumentDispatch[EVENT_HATE_LIST] = handle_npc_hate;
	mImpl->NPCArgumentDispatch[EVENT_COMBAT] = handle_npc_hate;
	mImpl->NPCArgumentDispatch[EVENT_SIGNAL] = handle_npc_signal;
	mImpl->NPCArgumentDispatch[EVENT_TIMER] = handle_npc_timer;
	mImpl->NPCArgumentDispatch[EVENT_DEATH] = handle_npc_death;
	mImpl->NPCArgumentDispatch[EVENT_DEATH_COMPLETE] = handle_npc_death;
	mImpl->NPCArgumentDispatch[EVENT_CAST] = handle_npc_cast;
	mImpl->NPCArgumentDispatch[EVENT_CAST_BEGIN] = handle_npc_cast;
	mImpl->NPCArgumentDispatch[EVENT_FEIGN_DEATH] = handle_npc_single_client;
	mImpl->NPCArgumentDispatch[EVENT_ENTER_AREA] = handle_npc_area;
	mImpl->NPCArgumentDispatch[EVENT_LEAVE_AREA] = handle_npc_area;

	mImpl->PlayerArgumentDispatch[EVENT_SAY] = handle_player_say;
	mImpl->PlayerArgumentDispatch[EVENT_ENVIRONMENTAL_DAMAGE] = handle_player_environmental_damage;
	mImpl->PlayerArgumentDispatch[EVENT_DEATH] = handle_player_death;
	mImpl->PlayerArgumentDispatch[EVENT_DEATH_COMPLETE] = handle_player_death;
	mImpl->PlayerArgumentDispatch[EVENT_TIMER] = handle_player_timer;
	mImpl->PlayerArgumentDispatch[EVENT_DISCOVER_ITEM] = handle_player_discover_item;
	mImpl->PlayerArgumentDispatch[EVENT_FISH_SUCCESS] = handle_player_fish_forage_success;
	mImpl->PlayerArgumentDispatch[EVENT_FORAGE_SUCCESS] = handle_player_fish_forage_success;
	mImpl->PlayerArgumentDispatch[EVENT_CLICK_OBJECT] = handle_player_click_object;
	mImpl->PlayerArgumentDispatch[EVENT_CLICK_DOOR] = handle_player_click_door;
	mImpl->PlayerArgumentDispatch[EVENT_SIGNAL] = handle_player_signal;
	mImpl->PlayerArgumentDispatch[EVENT_POPUP_RESPONSE] = handle_player_popup_response;
	mImpl->PlayerArgumentDispatch[EVENT_PLAYER_PICKUP] = handle_player_pick_up;
	mImpl->PlayerArgumentDispatch[EVENT_CAST] = handle_player_cast;
	mImpl->PlayerArgumentDispatch[EVENT_CAST_BEGIN] = handle_player_cast;
	mImpl->PlayerArgumentDispatch[EVENT_TASK_FAIL] = handle_player_task_fail;
	mImpl->PlayerArgumentDispatch[EVENT_ZONE] = handle_player_zone;
	mImpl->PlayerArgumentDispatch[EVENT_DUEL_WIN] = handle_player_duel_win;
	mImpl->PlayerArgumentDispatch[EVENT_DUEL_LOSE] = handle_player_duel_loss;
	mImpl->PlayerArgumentDispatch[EVENT_LOOT] = handle_player_loot;
	mImpl->PlayerArgumentDispatch[EVENT_TASK_STAGE_COMPLETE] = handle_player_task_stage_complete;
	mImpl->PlayerArgumentDispatch[EVENT_TASK_COMPLETE] = handle_player_task_update;
	mImpl->PlayerArgumentDispatch[EVENT_TASK_UPDATE] = handle_player_task_update;
	mImpl->PlayerArgumentDispatch[EVENT_COMMAND] = handle_player_command;
	mImpl->PlayerArgumentDispatch[EVENT_COMBINE_SUCCESS] = handle_player_combine;
	mImpl->PlayerArgumentDispatch[EVENT_COMBINE_FAILURE] = handle_player_combine;
	mImpl->PlayerArgumentDispatch[EVENT_FEIGN_DEATH] = handle_player_feign;
	mImpl->PlayerArgumentDispatch[EVENT_ENTER_AREA] = handle_player_area;
	mImpl->PlayerArgumentDispatch[EVENT_LEAVE_AREA] = handle_player_area;
	mImpl->PlayerArgumentDispatch[EVENT_RESPAWN] = handle_player_respawn;
	mImpl->PlayerArgumentDispatch[EVENT_UNHANDLED_OPCODE] = handle_player_packet;
	mImpl->PlayerArgumentDispatch[EVENT_USE_SKILL] = handle_player_use_skill;

	mImpl->ItemArgumentDispatch[EVENT_ITEM_CLICK] = handle_item_click;
	mImpl->ItemArgumentDispatch[EVENT_ITEM_CLICK_CAST] = handle_item_click;
	mImpl->ItemArgumentDispatch[EVENT_TIMER] = handle_item_timer;
	mImpl->ItemArgumentDispatch[EVENT_WEAPON_PROC] = handle_item_proc;
	mImpl->ItemArgumentDispatch[EVENT_LOOT] = handle_item_loot;
	mImpl->ItemArgumentDispatch[EVENT_EQUIP_ITEM] = handle_item_equip;
	mImpl->ItemArgumentDispatch[EVENT_UNEQUIP_ITEM] = handle_item_equip;
	mImpl->ItemArgumentDispatch[EVENT_AUGMENT_ITEM] = handle_item_augment;
	mImpl->ItemArgumentDispatch[EVENT_UNAUGMENT_ITEM] = handle_item_augment;
	mImpl->ItemArgumentDispatch[EVENT_AUGMENT_INSERT] = handle_item_augment_insert;
	mImpl->ItemArgumentDispatch[EVENT_AUGMENT_REMOVE] = handle_item_augment_remove;

	mImpl->SpellArgumentDispatch[EVENT_SPELL_EFFECT_CLIENT] = handle_spell_effect;
	mImpl->SpellArgumentDispatch[EVENT_SPELL_BUFF_TIC_CLIENT] = handle_spell_tic;
	mImpl->SpellArgumentDispatch[EVENT_SPELL_FADE] = handle_spell_fade;
	mImpl->SpellArgumentDispatch[EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE] = handle_translocate_finish;

	mImpl->EncounterArgumentDispatch[EVENT_TIMER] = handle_encounter_timer;
	mImpl->EncounterArgumentDispatch[EVENT_ENCOUNTER_LOAD] = handle_encounter_load;
	mImpl->EncounterArgumentDispatch[EVENT_ENCOUNTER_UNLOAD] = handle_encounter_unload;

	mImpl->state = nullptr;
}

LuaParser::~LuaParser() {
	// valgrind didn't like when we didn't clean these up :P
	lua_encounters.clear();
	lua_encounter_events_registered.clear();
	lua_encounters_loaded.clear();
	if(mImpl->state) {
		lua_close(mImpl->state);
	}
}

int LuaParser::EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc) {
		return 0;
	}

	if(!HasQuestSub(npc->GetNPCTypeID(), evt)) {
		return 0;
	}

	std::string package_name = "npc_" + std::to_string(npc->GetNPCTypeID());
	return _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
							  std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc) {
		return 0;
	}

	if(!HasGlobalQuestSub(evt)) {
		return 0;
	}

	return _EventNPC("global_npc", evt, npc, init, data, extra_data, extra_pointers);
}

int LuaParser::_EventNPC(std::string package_name, QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						 std::vector<EQEmu::Any> *extra_pointers, luabind::adl::object *l_func) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(mImpl->state);

	try {
		int npop = 1;
		if(l_func != nullptr) {
			l_func->push(mImpl->state);
		} else {
			lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(mImpl->state, -1, sub_name);
			npop = 2;
		}

		lua_createtable(mImpl->state, 0, 0);
		//always push self
		Lua_NPC l_npc(npc);
		luabind::adl::object l_npc_o = luabind::adl::object(mImpl->state, l_npc);
		l_npc_o.push(mImpl->state);
		lua_setfield(mImpl->state, -2, "self");

		auto arg_function = mImpl->NPCArgumentDispatch[evt];
		arg_function(this, mImpl->state, npc, init, data, extra_data, extra_pointers);
		Client *c = (init && init->IsClient()) ? init->CastToClient() : nullptr;

		quest_manager.StartQuest(npc, c, nullptr);
		if(lua_pcall(mImpl->state, 1, 1, 0)) {
			std::string error = lua_tostring(mImpl->state, -1);
			AddError(error);
			quest_manager.EndQuest();
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(mImpl->state, -1)) {
			int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
			lua_pop(mImpl->state, npop);
			return ret;
		}

		lua_pop(mImpl->state, npop);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(mImpl->state);
		int n = end - start;
		if(n > 0) {
			lua_pop(mImpl->state, n);
		}
	}

	return 0;
}

int LuaParser::EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!client) {
		return 0;
	}

	if(!PlayerHasQuestSub(evt)) {
		return 0;
	}

	return _EventPlayer("player", evt, client, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!client) {
		return 0;
	}

	if(!GlobalPlayerHasQuestSub(evt)) {
		return 0;
	}

	return _EventPlayer("global_player", evt, client, data, extra_data, extra_pointers);
}

int LuaParser::_EventPlayer(std::string package_name, QuestEventID evt, Client *client, std::string data, uint32 extra_data,
							std::vector<EQEmu::Any> *extra_pointers, luabind::adl::object *l_func) {
	const char *sub_name = LuaEvents[evt];
	int start = lua_gettop(mImpl->state);

	try {
		int npop = 1;
		if(l_func != nullptr) {
			l_func->push(mImpl->state);
		} else {
			lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(mImpl->state, -1, sub_name);
			npop = 2;
		}

		lua_createtable(mImpl->state, 0, 0);
		//push self
		Lua_Client l_client(client);
		luabind::adl::object l_client_o = luabind::adl::object(mImpl->state, l_client);
		l_client_o.push(mImpl->state);
		lua_setfield(mImpl->state, -2, "self");

		auto arg_function = mImpl->PlayerArgumentDispatch[evt];
		arg_function(this, mImpl->state, client, data, extra_data, extra_pointers);

		quest_manager.StartQuest(client, client, nullptr);
		if(lua_pcall(mImpl->state, 1, 1, 0)) {
			std::string error = lua_tostring(mImpl->state, -1);
			AddError(error);
			quest_manager.EndQuest();
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(mImpl->state, -1)) {
			int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
			lua_pop(mImpl->state, npop);
			return ret;
		}

		lua_pop(mImpl->state, npop);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(mImpl->state);
		int n = end - start;
		if(n > 0) {
			lua_pop(mImpl->state, n);
		}
	}

	return 0;
}

int LuaParser::EventItem(QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!item) {
		return 0;
	}

	if(!ItemHasQuestSub(item, evt)) {
		return 0;
	}

	std::string package_name = "item_";
	package_name += std::to_string(item->GetID());
	return _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers);
}

int LuaParser::_EventItem(std::string package_name, QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob,
						  std::string data, uint32 extra_data, std::vector<EQEmu::Any> *extra_pointers, luabind::adl::object *l_func) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(mImpl->state);

	try {
		int npop = 1;
		if(l_func != nullptr) {
			l_func->push(mImpl->state);
		} else {
			lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(mImpl->state, -1, sub_name);
		}

		lua_createtable(mImpl->state, 0, 0);
		//always push self
		Lua_ItemInst l_item(item);
		luabind::adl::object l_item_o = luabind::adl::object(mImpl->state, l_item);
		l_item_o.push(mImpl->state);
		lua_setfield(mImpl->state, -2, "self");

		Lua_Client l_client(client);
		luabind::adl::object l_client_o = luabind::adl::object(mImpl->state, l_client);
		l_client_o.push(mImpl->state);
		lua_setfield(mImpl->state, -2, "owner");

		//redo this arg function
		auto arg_function = mImpl->ItemArgumentDispatch[evt];
		arg_function(this, mImpl->state, client, item, mob, data, extra_data, extra_pointers);

		quest_manager.StartQuest(client, client, item);
		if(lua_pcall(mImpl->state, 1, 1, 0)) {
			std::string error = lua_tostring(mImpl->state, -1);
			AddError(error);
			quest_manager.EndQuest();
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(mImpl->state, -1)) {
			int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
			lua_pop(mImpl->state, npop);
			return ret;
		}

		lua_pop(mImpl->state, npop);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(mImpl->state);
		int n = end - start;
		if(n > 0) {
			lua_pop(mImpl->state, n);
		}
	}

	return 0;
}

int LuaParser::EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
						  std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "spell_" + std::to_string(spell_id);

	if(!SpellHasQuestSub(spell_id, evt)) {
		return 0;
	}

	return _EventSpell(package_name, evt, npc, client, spell_id, extra_data, extra_pointers);
}

int LuaParser::_EventSpell(std::string package_name, QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
						   std::vector<EQEmu::Any> *extra_pointers, luabind::adl::object *l_func) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(mImpl->state);

	try {
		int npop = 1;
		if(l_func != nullptr) {
			l_func->push(mImpl->state);
		} else {
			lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(mImpl->state, -1, sub_name);
			npop = 2;
		}

		lua_createtable(mImpl->state, 0, 0);

		//always push self even if invalid
		if(IsValidSpell(spell_id)) {
			Lua_Spell l_spell(&spells[spell_id]);
			luabind::adl::object l_spell_o = luabind::adl::object(mImpl->state, l_spell);
			l_spell_o.push(mImpl->state);
		} else {
			Lua_Spell l_spell(nullptr);
			luabind::adl::object l_spell_o = luabind::adl::object(mImpl->state, l_spell);
			l_spell_o.push(mImpl->state);
		}
		lua_setfield(mImpl->state, -2, "self");

		auto arg_function = mImpl->SpellArgumentDispatch[evt];
		arg_function(this, mImpl->state, npc, client, spell_id, extra_data, extra_pointers);

		quest_manager.StartQuest(npc, client, nullptr);
		if(lua_pcall(mImpl->state, 1, 1, 0)) {
			std::string error = lua_tostring(mImpl->state, -1);
			AddError(error);
			quest_manager.EndQuest();
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(mImpl->state, -1)) {
			int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
			lua_pop(mImpl->state, npop);
			return ret;
		}

		lua_pop(mImpl->state, npop);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(mImpl->state);
		int n = end - start;
		if(n > 0) {
			lua_pop(mImpl->state, n);
		}
	}

	return 0;
}

int LuaParser::EventEncounter(QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data, std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "encounter_" + encounter_name;

	if(!EncounterHasQuestSub(encounter_name, evt)) {
		return 0;
	}

	return _EventEncounter(package_name, evt, encounter_name, data, extra_data, extra_pointers);
}

int LuaParser::_EventEncounter(std::string package_name, QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data,
							   std::vector<EQEmu::Any> *extra_pointers) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(mImpl->state);

	try {
		lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
		lua_getfield(mImpl->state, -1, sub_name);

		lua_createtable(mImpl->state, 0, 0);
		lua_pushstring(mImpl->state, encounter_name.c_str());
		lua_setfield(mImpl->state, -2, "name");

		Encounter *enc = lua_encounters[encounter_name];

		auto arg_function = mImpl->EncounterArgumentDispatch[evt];
		arg_function(this, mImpl->state, enc, data, extra_data, extra_pointers);

		quest_manager.StartQuest(enc, nullptr, nullptr, encounter_name);
		if(lua_pcall(mImpl->state, 1, 1, 0)) {
			std::string error = lua_tostring(mImpl->state, -1);
			AddError(error);
			quest_manager.EndQuest();
			return 0;
		}
		quest_manager.EndQuest();

		if(lua_isnumber(mImpl->state, -1)) {
			int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
			lua_pop(mImpl->state, 2);
			return ret;
		}

		lua_pop(mImpl->state, 2);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(mImpl->state);
		int n = end - start;
		if(n > 0) {
			lua_pop(mImpl->state, n);
		}
	}

	return 0;
}

bool LuaParser::HasQuestSub(uint32 npc_id, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "npc_" + std::to_string(npc_id);

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::HasGlobalQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_npc");
}

bool LuaParser::PlayerHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "player");
}

bool LuaParser::GlobalPlayerHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_player");
}

bool LuaParser::SpellHasQuestSub(uint32 spell_id, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "spell_" + std::to_string(spell_id);

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::ItemHasQuestSub(EQEmu::ItemInstance *itm, QuestEventID evt) {
	if (itm == nullptr) {
		return false;
	}
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "item_";
	package_name += std::to_string(itm->GetID());

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::EncounterHasQuestSub(std::string encounter_name, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "encounter_" + encounter_name;

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

void LuaParser::LoadNPCScript(std::string filename, int npc_id) {
	std::string package_name = "npc_" + std::to_string(npc_id);

	LoadScript(filename, package_name);
}

void LuaParser::LoadGlobalNPCScript(std::string filename) {
	LoadScript(filename, "global_npc");
}

void LuaParser::LoadPlayerScript(std::string filename) {
	LoadScript(filename, "player");
}

void LuaParser::LoadGlobalPlayerScript(std::string filename) {
	LoadScript(filename, "global_player");
}

void LuaParser::LoadItemScript(std::string filename, EQEmu::ItemInstance *item) {
	if (item == nullptr)
		return;
	std::string package_name = "item_";
	package_name += std::to_string(item->GetID());

	LoadScript(filename, package_name);
}

void LuaParser::LoadSpellScript(std::string filename, uint32 spell_id) {
	std::string package_name = "spell_" + std::to_string(spell_id);

	LoadScript(filename, package_name);
}

void LuaParser::LoadEncounterScript(std::string filename, std::string encounter_name) {
	std::string package_name = "encounter_" + encounter_name;

	LoadScript(filename, package_name);
}

void LuaParser::AddVar(std::string name, std::string val) {
	mImpl->vars[name] = val;
}

std::string LuaParser::GetVar(std::string name) {
	auto iter = mImpl->vars.find(name);
	if(iter != mImpl->vars.end()) {
		return iter->second;
	}

	return std::string();
}

void LuaParser::Init() {
	ReloadQuests();
}

void LuaParser::ReloadQuests() {
	mImpl->loaded.clear();
	errors_.clear();
	mImpl->mods.clear();
	lua_encounter_events_registered.clear();
	lua_encounters_loaded.clear();

	for (auto encounter : lua_encounters) {
		encounter.second->Depop();
	}

	lua_encounters.clear();
	// so the Depop function above depends on the Process being called again so ...
	// And there is situations where it wouldn't be :P
	entity_list.EncounterProcess();

	if(mImpl->state) {
		lua_close(mImpl->state);
	}

	mImpl->state = luaL_newstate();
	luaL_openlibs(mImpl->state);

	auto top = lua_gettop(mImpl->state);

	if(luaopen_bit(mImpl->state) != 1) {
		std::string error = lua_tostring(mImpl->state, -1);
		AddError(error);
	}

	if(luaL_dostring(mImpl->state, "math.randomseed(os.time())")) {
		std::string error = lua_tostring(mImpl->state, -1);
		AddError(error);
	}

#ifdef SANITIZE_LUA_LIBS
	//io
	lua_pushnil(mImpl->state);
	//lua_setglobal(mImpl->state, "io");

	//some os/debug are okay some are not
	lua_getglobal(mImpl->state, "os");
	lua_pushnil(mImpl->state);
	lua_setfield(mImpl->state, -2, "exit");
	lua_pushnil(mImpl->state);
	lua_setfield(mImpl->state, -2, "execute");
	lua_pushnil(mImpl->state);
	lua_setfield(mImpl->state, -2, "getenv");
	lua_pushnil(mImpl->state);
	lua_setfield(mImpl->state, -2, "remove");
	lua_pushnil(mImpl->state);
	lua_setfield(mImpl->state, -2, "rename");
	lua_pushnil(mImpl->state);
	lua_setfield(mImpl->state, -2, "setlocale");
	lua_pushnil(mImpl->state);
	lua_setfield(mImpl->state, -2, "tmpname");
	lua_pop(mImpl->state, 1);

	lua_pushnil(mImpl->state);
	lua_setglobal(mImpl->state, "collectgarbage");

	lua_pushnil(mImpl->state);
	lua_setglobal(mImpl->state, "loadfile");

#endif

	// lua 5.2+ defines these
#if defined(LUA_VERSION_MAJOR) && defined(LUA_VERSION_MINOR)
	const char lua_version[] = LUA_VERSION_MAJOR "." LUA_VERSION_MINOR;
#elif LUA_VERSION_NUM == 501
	const char lua_version[] = "5.1";
#else
#error Incompatible lua version
#endif

#ifdef WINDOWS
	const char libext[] = ".dll";
#else
	// lua doesn't care OSX doesn't use sonames
	const char libext[] = ".so";
#endif

	lua_getglobal(mImpl->state, "package");
	lua_getfield(mImpl->state, -1, "path");
	std::string module_path = lua_tostring(mImpl->state,-1);
	module_path += ";./" + Config->LuaModuleDir + "?.lua;./" + Config->LuaModuleDir + "?/init.lua";
	// luarock paths using lua_modules as tree
	// to path it adds foo/share/lua/5.1/?.lua and foo/share/lua/5.1/?/init.lua
	module_path += ";./" + Config->LuaModuleDir + "share/lua/" + lua_version + "/?.lua";
	module_path += ";./" + Config->LuaModuleDir + "share/lua/" + lua_version + "/?/init.lua";
	lua_pop(mImpl->state, 1);
	lua_pushstring(mImpl->state, module_path.c_str());
	lua_setfield(mImpl->state, -2, "path");
	lua_pop(mImpl->state, 1);

	lua_getglobal(mImpl->state, "package");
	lua_getfield(mImpl->state, -1, "cpath");
	module_path = lua_tostring(mImpl->state, -1);
	module_path += ";./" + Config->LuaModuleDir + "?" + libext;
	// luarock paths using lua_modules as tree
	// luarocks adds foo/lib/lua/5.1/?.so for cpath
	module_path += ";./" + Config->LuaModuleDir + "lib/lua/" + lua_version + "/?" + libext;
	lua_pop(mImpl->state, 1);
	lua_pushstring(mImpl->state, module_path.c_str());
	lua_setfield(mImpl->state, -2, "cpath");
	lua_pop(mImpl->state, 1);

	MapFunctions(mImpl->state);

	//load init
	std::string path = Config->QuestDir;
	path += "/";
	path += QUEST_GLOBAL_DIRECTORY;
	path += "/script_init.lua";

	FILE *f = fopen(path.c_str(), "r");
	if(f) {
		fclose(f);

		if(luaL_dofile(mImpl->state, path.c_str())) {
			std::string error = lua_tostring(mImpl->state, -1);
			AddError(error);
		}
	}

	//zone init - always loads after global
	if(zone) {
		std::string zone_script = Config->QuestDir;
		zone_script += "/";
		zone_script += zone->GetShortName();
		zone_script += "/script_init_v";
		zone_script += std::to_string(zone->GetInstanceVersion());
		zone_script += ".lua";
		f = fopen(zone_script.c_str(), "r");
		if(f) {
			fclose(f);

			if(luaL_dofile(mImpl->state, zone_script.c_str())) {
				std::string error = lua_tostring(mImpl->state, -1);
				AddError(error);
			}
		}
		else {
			zone_script = Config->QuestDir;
			zone_script += "/";
			zone_script += zone->GetShortName();
			zone_script += "/script_init.lua";
			f = fopen(zone_script.c_str(), "r");
			if (f) {
				fclose(f);

				if (luaL_dofile(mImpl->state, zone_script.c_str())) {
					std::string error = lua_tostring(mImpl->state, -1);
					AddError(error);
				}
			}
		}
	}

	FILE *load_order = fopen("mods/load_order.txt", "r");
	if (load_order) {
		char file_name[256] = { 0 };
		while (fgets(file_name, 256, load_order) != nullptr) {
			for (int i = 0; i < 256; ++i) {
				auto c = file_name[i];
				if (c == '\n' || c == '\r' || c == ' ') {
					file_name[i] = 0;
					break;
				}
			}

			LoadScript("mods/" + std::string(file_name), file_name);
			mImpl->mods.push_back(LuaMod(mImpl->state, this, file_name));
		}

		fclose(load_order);
	}

	auto end = lua_gettop(mImpl->state);
	int n = end - top;
	if (n > 0) {
		lua_pop(mImpl->state, n);
	}
}

void LuaParser::LoadScript(std::string filename, std::string package_name) {
	auto iter = mImpl->loaded.find(package_name);
	if(iter != mImpl->loaded.end()) {
		return;
	}

	auto top = lua_gettop(mImpl->state);
	if(luaL_loadfile(mImpl->state, filename.c_str())) {
		std::string error = lua_tostring(mImpl->state, -1);
		AddError(error);
		lua_pop(mImpl->state, 1);
		return;
	}

	//This makes an env table named: package_name
	//And makes it so we can see the global table _G from it
	//Then sets it so this script is called from that table as an env

	lua_createtable(mImpl->state, 0, 0); // anon table
	lua_getglobal(mImpl->state, "_G"); // get _G
	lua_setfield(mImpl->state, -2, "__index"); //anon table.__index = _G

	lua_pushvalue(mImpl->state, -1); //copy table to top of stack
	lua_setmetatable(mImpl->state, -2); //setmetatable(anon_table, copied table)

	lua_pushvalue(mImpl->state, -1); //put the table we made into the registry
	lua_setfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());

	lua_setfenv(mImpl->state, -2); //set the env to the table we made

	if(lua_pcall(mImpl->state, 0, 0, 0)) {
		std::string error = lua_tostring(mImpl->state, -1);
		AddError(error);
		lua_pop(mImpl->state, 1);
	}
	else {
		mImpl->loaded[package_name] = true;
	}

	auto end = lua_gettop(mImpl->state);
	int n = end - top;
	if (n > 0) {
		lua_pop(mImpl->state, n);
	}
}

bool LuaParser::HasFunction(std::string subname, std::string package_name) {
	//std::transform(subname.begin(), subname.end(), subname.begin(), ::tolower);

	auto iter = mImpl->loaded.find(package_name);
	if(iter == mImpl->loaded.end()) {
		return false;
	}

	lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
	lua_getfield(mImpl->state, -1, subname.c_str());

	if(lua_isfunction(mImpl->state, -1)) {
		lua_pop(mImpl->state, 2);
		return true;
	}

	lua_pop(mImpl->state, 2);
	return false;
}

void LuaParser::MapFunctions(lua_State *L) {

	try {
		luabind::open(L);

		luabind::module(L)
		[
			lua_register_general(),
			lua_register_random(),
			lua_register_events(),
			lua_register_faction(),
			lua_register_slot(),
			lua_register_material(),
			lua_register_client_version(),
			lua_register_appearance(),
			lua_register_classes(),
			lua_register_skills(),
			lua_register_bodytypes(),
			lua_register_filters(),
			lua_register_message_types(),
			lua_register_entity(),
			lua_register_encounter(),
			lua_register_mob(),
			lua_register_special_abilities(),
			lua_register_npc(),
			lua_register_client(),
			lua_register_inventory(),
			lua_register_inventory_where(),
			lua_register_iteminst(),
			lua_register_item(),
			lua_register_spell(),
			lua_register_spawn(),
			lua_register_hate_entry(),
			lua_register_hate_list(),
			lua_register_entity_list(),
			lua_register_mob_list(),
			lua_register_client_list(),
			lua_register_npc_list(),
			lua_register_corpse_list(),
			lua_register_object_list(),
			lua_register_door_list(),
			lua_register_spawn_list(),
			lua_register_group(),
			lua_register_raid(),
			lua_register_corpse(),
			lua_register_door(),
			lua_register_object(),
			lua_register_packet(),
			lua_register_packet_opcodes(),
			lua_register_stat_bonuses(),
			lua_register_rules_const(),
			lua_register_rulei(),
			lua_register_ruler(),
			lua_register_ruleb()
		];

	} catch(std::exception &ex) {
		std::string error = ex.what();
		AddError(error);
	}
}

int LuaParser::DispatchEventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
								 std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc)
		return 0;

	std::string package_name = "npc_" + std::to_string(npc->GetNPCTypeID());
	int ret = 0;

	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter != lua_encounter_events_registered.end()) {
		auto riter = iter->second.begin();
		while(riter != iter->second.end()) {
			if(riter->event_id == evt) {
				std::string package_name = "encounter_" + riter->encounter_name;
				int i = _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers, &riter->lua_reference);
                if(i != 0)
                    ret = i;
			}
			++riter;
		}
	}

	iter = lua_encounter_events_registered.find("npc_-1");
	if(iter == lua_encounter_events_registered.end()) {
		return ret;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			int i = _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers, &riter->lua_reference);
            if(i != 0)
                ret = i;
		}
		++riter;
	}

    return ret;
}

int LuaParser::DispatchEventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
									std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "player";

	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter == lua_encounter_events_registered.end()) {
		return 0;
	}

    int ret = 0;
	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			int i = _EventPlayer(package_name, evt, client, data, extra_data, extra_pointers, &riter->lua_reference);
            if(i != 0)
                ret = i;
		}
		++riter;
	}

    return ret;
}

int LuaParser::DispatchEventItem(QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
								  std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!item)
		return 0;

	std::string package_name = "item_";
	package_name += std::to_string(item->GetID());
	int ret = 0;

	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter != lua_encounter_events_registered.end()) {
		auto riter = iter->second.begin();
		while(riter != iter->second.end()) {
			if(riter->event_id == evt) {
				std::string package_name = "encounter_" + riter->encounter_name;
				int i = _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers, &riter->lua_reference);
                if(i != 0)
                    ret = i;
			}
			++riter;
		}
	}

	iter = lua_encounter_events_registered.find("item_-1");
	if(iter == lua_encounter_events_registered.end()) {
		return ret;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			int i = _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers, &riter->lua_reference);
            if(i != 0)
                ret = i;
		}
		++riter;
	}
    return ret;
}

int LuaParser::DispatchEventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
								   std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "spell_" + std::to_string(spell_id);

    int ret = 0;
	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter != lua_encounter_events_registered.end()) {
	    auto riter = iter->second.begin();
		while(riter != iter->second.end()) {
			if(riter->event_id == evt) {
				std::string package_name = "encounter_" + riter->encounter_name;
				int i = _EventSpell(package_name, evt, npc, client, spell_id, extra_data, extra_pointers, &riter->lua_reference);
                if(i != 0) {
                    ret = i;
                }
			}
			++riter;
		}
	}

	iter = lua_encounter_events_registered.find("spell_-1");
	if(iter == lua_encounter_events_registered.end()) {
		return ret;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			int i = _EventSpell(package_name, evt, npc, client, spell_id, extra_data, extra_pointers, &riter->lua_reference);
            if(i != 0)
                ret = i;
		}
		++riter;
	}
    return ret;
}

QuestEventID LuaParser::ConvertLuaEvent(QuestEventID evt) {
	switch(evt) {
	case EVENT_SLAY:
	case EVENT_NPC_SLAY:
		return EVENT_SLAY;
		break;
	case EVENT_SPELL_EFFECT_CLIENT:
	case EVENT_SPELL_EFFECT_NPC:
		return EVENT_SPELL_EFFECT_CLIENT;
		break;
	case EVENT_SPELL_BUFF_TIC_CLIENT:
	case EVENT_SPELL_BUFF_TIC_NPC:
		return EVENT_SPELL_BUFF_TIC_CLIENT;
		break;
	case EVENT_AGGRO:
	case EVENT_ATTACK:
		return _LargestEventID;
		break;
	default:
		return evt;
	}
}

#endif

void LuaParser::MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	for (auto &mod : mImpl->mods) {
		mod.MeleeMitigation(self, attacker, hit, opts, ignoreDefault);
	}
}

void LuaParser::ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault)
{
	for (auto &mod : mImpl->mods) {
		mod.ApplyDamageTable(self, hit, ignoreDefault);
	}
}

bool LuaParser::AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool & ignoreDefault)
{
	bool retval = false;
	for (auto &mod : mImpl->mods) {
		mod.AvoidDamage(self, other, hit, retval, ignoreDefault);
	}
	return retval;
}

bool LuaParser::CheckHitChance(Mob *self, Mob *other, DamageHitInfo &hit, bool &ignoreDefault)
{
	bool retval = false;
	for (auto &mod : mImpl->mods) {
		mod.CheckHitChance(self, other, hit, retval, ignoreDefault);
	}
	return retval;
}

void LuaParser::TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	for (auto &mod : mImpl->mods) {
		mod.TryCriticalHit(self, defender, hit, opts, ignoreDefault);
	}
}

void LuaParser::CommonOutgoingHitSuccess(Mob *self, Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	for (auto &mod : mImpl->mods) {
		mod.CommonOutgoingHitSuccess(self, other, hit, opts, ignoreDefault);
	}
}

uint32 LuaParser::GetRequiredAAExperience(Client *self, bool &ignoreDefault)
{
	uint32 retval = 0;
	for (auto &mod : mImpl->mods) {
		mod.GetRequiredAAExperience(self, retval, ignoreDefault);
	}
	return retval;
}

uint32 LuaParser::GetEXPForLevel(Client *self, uint16 level, bool &ignoreDefault)
{
	uint32 retval = 0;
	for (auto &mod : mImpl->mods) {
		mod.GetEXPForLevel(self, level, retval, ignoreDefault);
	}
	return retval;
}

uint32 LuaParser::GetExperienceForKill(Client *self, Mob *against, bool &ignoreDefault)
{
	uint32 retval = 0;
	for (auto &mod : mImpl->mods) {
		mod.GetExperienceForKill(self, against, retval, ignoreDefault);
	}
	return retval;
}
