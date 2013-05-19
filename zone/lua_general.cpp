#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <sstream>
#include <list>
#include <map>

#include "lua_parser.h"
#include "QuestParserCollection.h"

struct Events { };

struct lua_registered_event {
	std::string encounter_name;
	luabind::object lua_reference;
	QuestEventID event_id;
};

extern std::map<std::string, std::list<lua_registered_event>> lua_encounter_events_registered;

void load_encounter(std::string name) {
	parse->EventEncounter(EVENT_ENCOUNTER_LOAD, name, 0);
}

void unload_encounter(std::string name) {
	parse->EventEncounter(EVENT_ENCOUNTER_UNLOAD, name, 0);
}

void register_npc_event(std::string name, int evt, int npc_id, luabind::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "npc_" << npc_id;

		lua_registered_event e;
		e.encounter_name = name;
		e.lua_reference = func;
		e.event_id = static_cast<QuestEventID>(evt);
		
		auto liter = lua_encounter_events_registered.find(package_name.str());
		if(liter == lua_encounter_events_registered.end()) {
			std::list<lua_registered_event> elist;
			elist.push_back(e);
			lua_encounter_events_registered[package_name.str()] = elist;
		} else {
			std::list<lua_registered_event> elist = liter->second;
			elist.push_back(e);
			lua_encounter_events_registered[package_name.str()] = elist;
		}
	}
}

luabind::scope lua_register_general() {
	return luabind::namespace_("eq")
	[
		luabind::def("load_encounter", &load_encounter),
		luabind::def("unload_encounter", &unload_encounter),
		luabind::def("register_npc_event", &register_npc_event)
	];
}

luabind::scope lua_register_events() {
	return luabind::class_<Events>("Event")
		.enum_("constants")
		[
			luabind::value("say", static_cast<int>(EVENT_SAY)),
			luabind::value("trade", static_cast<int>(EVENT_TRADE)),
			luabind::value("death", static_cast<int>(EVENT_DEATH)),
			luabind::value("spawn", static_cast<int>(EVENT_SPAWN)),
			luabind::value("attack", static_cast<int>(EVENT_ATTACK)),
			luabind::value("combat", static_cast<int>(EVENT_COMBAT)),
			luabind::value("aggro", static_cast<int>(EVENT_AGGRO)),
			luabind::value("slay", static_cast<int>(EVENT_SLAY)),
			luabind::value("npc_slay", static_cast<int>(EVENT_NPC_SLAY)),
			luabind::value("waypoint_arrive", static_cast<int>(EVENT_WAYPOINT_ARRIVE)),
			luabind::value("waypoint_depart", static_cast<int>(EVENT_WAYPOINT_DEPART)),
			luabind::value("timer", static_cast<int>(EVENT_TIMER)),
			luabind::value("signal", static_cast<int>(EVENT_SIGNAL)),
			luabind::value("hp", static_cast<int>(EVENT_HP)),
			luabind::value("enter", static_cast<int>(EVENT_ENTER)),
			luabind::value("exit", static_cast<int>(EVENT_EXIT)),
			luabind::value("enter_zone", static_cast<int>(EVENT_ENTER_ZONE)),
			luabind::value("click_door", static_cast<int>(EVENT_CLICK_DOOR)),
			luabind::value("loot", static_cast<int>(EVENT_LOOT)),
			luabind::value("zone", static_cast<int>(EVENT_ZONE)),
			luabind::value("level_up", static_cast<int>(EVENT_LEVEL_UP)),
			luabind::value("killed_merit ", static_cast<int>(EVENT_KILLED_MERIT )),
			luabind::value("cast_on", static_cast<int>(EVENT_CAST_ON)),
			luabind::value("task_accepted", static_cast<int>(EVENT_TASK_ACCEPTED)),
			luabind::value("task_stage_complete", static_cast<int>(EVENT_TASK_STAGE_COMPLETE)),
			luabind::value("task_update", static_cast<int>(EVENT_TASK_UPDATE)),
			luabind::value("task_complete", static_cast<int>(EVENT_TASK_COMPLETE)),
			luabind::value("task_fail", static_cast<int>(EVENT_TASK_FAIL)),
			luabind::value("aggro_say", static_cast<int>(EVENT_AGGRO_SAY)),
			luabind::value("player_pickup", static_cast<int>(EVENT_PLAYER_PICKUP)),
			luabind::value("popup_response", static_cast<int>(EVENT_POPUP_RESPONSE)),
			luabind::value("proximity_say", static_cast<int>(EVENT_PROXIMITY_SAY)),
			luabind::value("cast", static_cast<int>(EVENT_CAST)),
			luabind::value("scale_calc", static_cast<int>(EVENT_SCALE_CALC)),
			luabind::value("item_enter_zone", static_cast<int>(EVENT_ITEM_ENTER_ZONE)),
			luabind::value("target_change", static_cast<int>(EVENT_TARGET_CHANGE)),
			luabind::value("hate_list", static_cast<int>(EVENT_HATE_LIST)),
			luabind::value("spell_effect_client", static_cast<int>(EVENT_SPELL_EFFECT_CLIENT)),
			luabind::value("spell_effect_npc", static_cast<int>(EVENT_SPELL_EFFECT_NPC)),
			luabind::value("spell_effect_buff_tic_client", static_cast<int>(EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT)),
			luabind::value("spell_effect_buff_tic_npc", static_cast<int>(EVENT_SPELL_EFFECT_BUFF_TIC_NPC)),
			luabind::value("spell_effect_translocate_complete", static_cast<int>(EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE)),
			luabind::value("combine_success ", static_cast<int>(EVENT_COMBINE_SUCCESS )),
			luabind::value("combine_failure ", static_cast<int>(EVENT_COMBINE_FAILURE )),
			luabind::value("item_click", static_cast<int>(EVENT_ITEM_CLICK)),
			luabind::value("item_click_cast", static_cast<int>(EVENT_ITEM_CLICK_CAST)),
			luabind::value("group_change", static_cast<int>(EVENT_GROUP_CHANGE)),
			luabind::value("forage_success", static_cast<int>(EVENT_FORAGE_SUCCESS)),
			luabind::value("forage_failure", static_cast<int>(EVENT_FORAGE_FAILURE)),
			luabind::value("fish_start", static_cast<int>(EVENT_FISH_START)),
			luabind::value("fish_success", static_cast<int>(EVENT_FISH_SUCCESS)),
			luabind::value("fish_failure", static_cast<int>(EVENT_FISH_FAILURE)),
			luabind::value("click_object", static_cast<int>(EVENT_CLICK_OBJECT)),
			luabind::value("discover_item", static_cast<int>(EVENT_DISCOVER_ITEM)),
			luabind::value("disconnect", static_cast<int>(EVENT_DISCONNECT)),
			luabind::value("connect", static_cast<int>(EVENT_CONNECT)),
			luabind::value("item_tick", static_cast<int>(EVENT_ITEM_TICK)),
			luabind::value("duel_win", static_cast<int>(EVENT_DUEL_WIN)),
			luabind::value("duel_lose", static_cast<int>(EVENT_DUEL_LOSE)),
			luabind::value("encounter_load", static_cast<int>(EVENT_ENCOUNTER_LOAD)),
			luabind::value("encounter_unload", static_cast<int>(EVENT_ENCOUNTER_UNLOAD))
		];
}

#endif
