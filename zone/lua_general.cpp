#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include <sstream>
#include <list>
#include <map>

#include "../common/content/world_content_service.h"
#include "../common/timer.h"
#include "../common/eqemu_logsys.h"
#include "../common/classes.h"
#include "../common/rulesys.h"
#include "lua_parser.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_entity_list.h"
#include "lua_expedition.h"
#include "lua_spell.h"
#include "quest_parser_collection.h"
#include "questmgr.h"
#include "qglobals.h"
#include "encounter.h"
#include "lua_encounter.h"
#include "data_bucket.h"
#include "expedition.h"

struct Events { };
struct Factions { };
struct Slots { };
struct Materials { };
struct ClientVersions { };
struct Appearances { };
struct Classes { };
struct Skills { };
struct BodyTypes { };
struct Filters { };
struct MessageTypes { };
struct Rule { };
struct Journal_SpeakMode { };
struct Journal_Mode { };

struct lua_registered_event {
	std::string encounter_name;
	luabind::adl::object lua_reference;
	QuestEventID event_id;
};

extern std::map<std::string, std::list<lua_registered_event>> lua_encounter_events_registered;
extern std::map<std::string, bool> lua_encounters_loaded;
extern std::map<std::string, Encounter *> lua_encounters;

extern void MapOpcodes();
extern void ClearMappedOpcode(EmuOpcode op);

void unregister_event(std::string package_name, std::string name, int evt);

void load_encounter(std::string name) {
	if(lua_encounters_loaded.count(name) > 0)
		return;
	auto enc = new Encounter(name.c_str());
	entity_list.AddEncounter(enc);
	lua_encounters[name] = enc;
	lua_encounters_loaded[name] = true;
	parse->EventEncounter(EVENT_ENCOUNTER_LOAD, name, "", 0);
}

void load_encounter_with_data(std::string name, std::string info_str) {
	if(lua_encounters_loaded.count(name) > 0)
		return;
	auto enc = new Encounter(name.c_str());
	entity_list.AddEncounter(enc);
	lua_encounters[name] = enc;
	lua_encounters_loaded[name] = true;
	std::vector<EQ::Any> info_ptrs;
	info_ptrs.push_back(&info_str);
	parse->EventEncounter(EVENT_ENCOUNTER_LOAD, name, "", 0, &info_ptrs);
}

void unload_encounter(std::string name) {
	if(lua_encounters_loaded.count(name) == 0)
		return;

	auto liter = lua_encounter_events_registered.begin();
	while(liter != lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> &elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if((*iter).encounter_name.compare(name) == 0) {
				iter = elist.erase(iter);
			} else {
				++iter;
			}
		}

		if(elist.size() == 0) {
			lua_encounter_events_registered.erase(liter++);
		} else {
			++liter;
		}
	}

	lua_encounters[name]->Depop();
	lua_encounters.erase(name);
	lua_encounters_loaded.erase(name);
	parse->EventEncounter(EVENT_ENCOUNTER_UNLOAD, name, "", 0);
}

void unload_encounter_with_data(std::string name, std::string info_str) {
	if(lua_encounters_loaded.count(name) == 0)
		return;

	auto liter = lua_encounter_events_registered.begin();
	while(liter != lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> &elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if((*iter).encounter_name.compare(name) == 0) {
				iter = elist.erase(iter);
			}
			else {
				++iter;
			}
		}

		if(elist.size() == 0) {
			lua_encounter_events_registered.erase(liter++);
		}
		else {
			++liter;
		}
	}

	lua_encounters[name]->Depop();
	lua_encounters.erase(name);
	lua_encounters_loaded.erase(name);
	std::vector<EQ::Any> info_ptrs;
	info_ptrs.push_back(&info_str);
	parse->EventEncounter(EVENT_ENCOUNTER_UNLOAD, name, "", 0, &info_ptrs);
}

void register_event(std::string package_name, std::string name, int evt, luabind::adl::object func) {
	if(lua_encounters_loaded.count(name) == 0)
		return;

	unregister_event(package_name, name, evt);

	lua_registered_event e;
	e.encounter_name = name;
	e.lua_reference = func;
	e.event_id = static_cast<QuestEventID>(evt);

	auto liter = lua_encounter_events_registered.find(package_name);
	if(liter == lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> elist;
		elist.push_back(e);
		lua_encounter_events_registered[package_name] = elist;
	} else {
		std::list<lua_registered_event> &elist = liter->second;
		elist.push_back(e);
	}
}

void unregister_event(std::string package_name, std::string name, int evt) {
	auto liter = lua_encounter_events_registered.find(package_name);
	if(liter != lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if(iter->event_id == evt && iter->encounter_name.compare(name) == 0) {
				iter = elist.erase(iter);
				break;
			}
			++iter;
		}
		lua_encounter_events_registered[package_name] = elist;
	}
}

void register_npc_event(std::string name, int evt, int npc_id, luabind::adl::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "npc_" << npc_id;

		register_event(package_name.str(), name, evt, func);
	}
}

void register_npc_event(int evt, int npc_id, luabind::adl::object func) {
	std::string name = quest_manager.GetEncounter();
	register_npc_event(name, evt, npc_id, func);
}

void unregister_npc_event(std::string name, int evt, int npc_id) {
	std::stringstream package_name;
	package_name << "npc_" << npc_id;

	unregister_event(package_name.str(), name, evt);
}

void unregister_npc_event(int evt, int npc_id) {
	std::string name = quest_manager.GetEncounter();
	unregister_npc_event(name, evt, npc_id);
}

void register_player_event(std::string name, int evt, luabind::adl::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		register_event("player", name, evt, func);
	}
}

void register_player_event(int evt, luabind::adl::object func) {
	std::string name = quest_manager.GetEncounter();
	register_player_event(name, evt, func);
}

void unregister_player_event(std::string name, int evt) {
	unregister_event("player", name, evt);
}

void unregister_player_event(int evt) {
	std::string name = quest_manager.GetEncounter();
	unregister_player_event(name, evt);
}

void register_item_event(std::string name, int evt, int item_id, luabind::adl::object func) {
	std::string package_name = "item_";
	package_name += std::to_string(item_id);

	if(luabind::type(func) == LUA_TFUNCTION) {
		register_event(package_name, name, evt, func);
	}
}

void register_item_event(int evt, int item_id, luabind::adl::object func) {
	std::string name = quest_manager.GetEncounter();
	register_item_event(name, evt, item_id, func);
}

void unregister_item_event(std::string name, int evt, int item_id) {
	std::string package_name = "item_";
	package_name += std::to_string(item_id);

	unregister_event(package_name, name, evt);
}

void unregister_item_event(int evt, int item_id) {
	std::string name = quest_manager.GetEncounter();
	unregister_item_event(name, evt, item_id);
}

void register_spell_event(std::string name, int evt, int spell_id, luabind::adl::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "spell_" << spell_id;

		register_event(package_name.str(), name, evt, func);
	}
}

void register_spell_event(int evt, int spell_id, luabind::adl::object func) {
	std::string name = quest_manager.GetEncounter();
	register_spell_event(name, evt, spell_id, func);
}

void unregister_spell_event(std::string name, int evt, int spell_id) {
	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	unregister_event(package_name.str(), name, evt);
}

void unregister_spell_event(int evt, int spell_id) {
	std::string name = quest_manager.GetEncounter();
	unregister_spell_event(name, evt, spell_id);
}

Lua_Mob lua_spawn2(int npc_type, int grid, int unused, double x, double y, double z, double heading) {
	auto position = glm::vec4(x, y, z, heading);
	return Lua_Mob(quest_manager.spawn2(npc_type, grid, unused, position));
}

Lua_Mob lua_unique_spawn(int npc_type, int grid, int unused, double x, double y, double z, double heading = 0.0) {
	auto position = glm::vec4(x, y, z, heading);
	return Lua_Mob(quest_manager.unique_spawn(npc_type, grid, unused, position));
}

Lua_Mob lua_spawn_from_spawn2(uint32 spawn2_id) {
	return Lua_Mob(quest_manager.spawn_from_spawn2(spawn2_id));
}

void lua_enable_spawn2(int spawn2_id) {
	quest_manager.enable_spawn2(spawn2_id);
}

void lua_disable_spawn2(int spawn2_id) {
	quest_manager.disable_spawn2(spawn2_id);
}

void lua_set_timer(const char *timer, int time_ms) {
	quest_manager.settimerMS(timer, time_ms);
}

void lua_set_timer(const char *timer, int time_ms, Lua_ItemInst inst) {
	quest_manager.settimerMS(timer, time_ms, inst);
}

void lua_set_timer(const char *timer, int time_ms, Lua_Mob mob) {
	quest_manager.settimerMS(timer, time_ms, mob);
}

void lua_set_timer(const char *timer, int time_ms, Lua_Encounter enc) {
	quest_manager.settimerMS(timer, time_ms, enc);
}

void lua_stop_timer(const char *timer) {
	quest_manager.stoptimer(timer);
}

void lua_stop_timer(const char *timer, Lua_ItemInst inst) {
	quest_manager.stoptimer(timer, inst);
}

void lua_stop_timer(const char *timer, Lua_Mob mob) {
	quest_manager.stoptimer(timer, mob);
}

void lua_stop_timer(const char *timer, Lua_Encounter enc) {
	quest_manager.stoptimer(timer, enc);
}

void lua_stop_all_timers() {
	quest_manager.stopalltimers();
}

void lua_stop_all_timers(Lua_ItemInst inst) {
	quest_manager.stopalltimers(inst);
}

void lua_stop_all_timers(Lua_Mob mob) {
	quest_manager.stopalltimers(mob);
}

void lua_stop_all_timers(Lua_Encounter enc) {
	quest_manager.stopalltimers(enc);
}

void lua_pause_timer(const char *timer) {
	quest_manager.pausetimer(timer);

}

void lua_resume_timer(const char *timer) {
	quest_manager.resumetimer(timer);
}

bool lua_is_paused_timer(const char *timer) {
	return quest_manager.ispausedtimer(timer);
}

bool lua_has_timer(const char *timer) {
	return quest_manager.hastimer(timer);
}

uint32 lua_get_remaining_time(const char *timer) {
	return quest_manager.getremainingtimeMS(timer);
}

uint32 lua_get_timer_duration(const char *timer) {
	return quest_manager.gettimerdurationMS(timer);
}

void lua_depop() {
	quest_manager.depop(0);
}

void lua_depop(int npc_type) {
	quest_manager.depop(npc_type);
}

void lua_depop_with_timer() {
	quest_manager.depop_withtimer(0);
}

void lua_depop_with_timer(int npc_type) {
	quest_manager.depop_withtimer(npc_type);
}

void lua_depop_all() {
	quest_manager.depopall(0);
}

void lua_depop_all(int npc_type) {
	quest_manager.depopall(npc_type);
}

void lua_depop_zone(bool start_spawn_status) {
	quest_manager.depopzone(start_spawn_status);
}

void lua_repop_zone() {
	quest_manager.repopzone();
}

void lua_process_mobs_while_zone_empty(bool on) {
	quest_manager.processmobswhilezoneempty(on);
}

bool lua_is_disc_tome(int item_id) {
	return quest_manager.isdisctome(item_id);
}

std::string lua_get_race_name(uint32 race_id) {
	return quest_manager.getracename(race_id);
}

std::string lua_get_spell_name(uint32 spell_id) {
	return quest_manager.getspellname(spell_id);
}

std::string lua_get_skill_name(int skill_id) {
	return quest_manager.getskillname(skill_id);
}

void lua_safe_move() {
	quest_manager.safemove();
}

void lua_rain(int weather) {
	quest_manager.rain(weather);
}

void lua_snow(int weather) {
	quest_manager.rain(weather);
}

int lua_scribe_spells(int max) {
	return quest_manager.scribespells(max);
}

int lua_scribe_spells(int max, int min) {
	return quest_manager.scribespells(max, min);
}

int lua_train_discs(int max) {
	return quest_manager.traindiscs(max);
}

int lua_train_discs(int max, int min) {
	return quest_manager.traindiscs(max, min);
}

void lua_set_sky(int sky) {
	quest_manager.setsky(sky);
}

void lua_set_guild(int guild_id, int rank) {
	quest_manager.setguild(guild_id, rank);
}

void lua_create_guild(const char *name, const char *leader) {
	quest_manager.CreateGuild(name, leader);
}

void lua_set_time(int hour, int min) {
	quest_manager.settime(hour, min, true);
}

void lua_set_time(int hour, int min, bool update_world) {
	quest_manager.settime(hour, min, update_world);
}

void lua_signal(int npc_id, int signal_id) {
	quest_manager.signalwith(npc_id, signal_id);
}

void lua_signal(int npc_id, int signal_id, int wait) {
	quest_manager.signalwith(npc_id, signal_id, wait);
}

void lua_set_global(const char *name, const char *value, int options, const char *duration) {
	quest_manager.setglobal(name, value, options, duration);
}

void lua_target_global(const char *name, const char *value, const char *duration, int npc_id, int char_id, int zone_id) {
	quest_manager.targlobal(name, value, duration, npc_id, char_id, zone_id);
}

void lua_delete_global(const char *name) {
	quest_manager.delglobal(name);
}

void lua_start(int wp) {
	quest_manager.start(wp);
}

void lua_stop() {
	quest_manager.stop();
}

void lua_pause(int duration) {
	quest_manager.pause(duration);
}

void lua_move_to(float x, float y, float z) {
	quest_manager.moveto(glm::vec4(x, y, z, 0.0f), false);
}

void lua_move_to(float x, float y, float z, float h) {
	quest_manager.moveto(glm::vec4(x, y, z, h), false);
}

void lua_move_to(float x, float y, float z, float h, bool save_guard_spot) {
	quest_manager.moveto(glm::vec4(x, y, z, h), save_guard_spot);
}

void lua_path_resume() {
	quest_manager.resume();
}

void lua_set_next_hp_event(int hp) {
	quest_manager.setnexthpevent(hp);
}

void lua_set_next_inc_hp_event(int hp) {
	quest_manager.setnextinchpevent(hp);
}

void lua_respawn(int npc_type, int grid) {
	quest_manager.respawn(npc_type, grid);
}

void lua_set_proximity(float min_x, float max_x, float min_y, float max_y) {
	quest_manager.set_proximity(min_x, max_x, min_y, max_y);
}

void lua_set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z) {
	quest_manager.set_proximity(min_x, max_x, min_y, max_y, min_z, max_z);
}

void lua_set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z, bool say) {
	quest_manager.set_proximity(min_x, max_x, min_y, max_y, min_z, max_z, say);
}

void lua_clear_proximity() {
	quest_manager.clear_proximity();
}

void lua_enable_proximity_say() {
	quest_manager.enable_proximity_say();
}

void lua_disable_proximity_say() {
	quest_manager.disable_proximity_say();
}

void lua_set_anim(int npc_type, int anim_num) {
	quest_manager.setanim(npc_type, anim_num);
}

void lua_spawn_condition(const char *zone, uint32 instance_id, int condition_id, int value) {
	quest_manager.spawn_condition(zone, instance_id, condition_id, value);
}

int lua_get_spawn_condition(const char *zone, uint32 instance_id, int condition_id) {
	return quest_manager.get_spawn_condition(zone, instance_id, condition_id);
}

void lua_toggle_spawn_event(int event_id, bool enable, bool strict, bool reset) {
	quest_manager.toggle_spawn_event(event_id, enable, strict, reset);
}

void lua_summon_buried_player_corpse(uint32 char_id, float x, float y, float z, float h) {
	quest_manager.summonburiedplayercorpse(char_id, glm::vec4(x, y, z, h));
}

void lua_summon_all_player_corpses(uint32 char_id, float x, float y, float z, float h) {
	quest_manager.summonallplayercorpses(char_id, glm::vec4(x, y, z, h));
}

int lua_get_player_corpse_count(uint32 char_id) {
	return database.CountCharacterCorpses(char_id);
}

int lua_get_player_corpse_count_by_zone_id(uint32 char_id, uint32 zone_id) {
	return database.CountCharacterCorpsesByZoneID(char_id, zone_id);
}

int lua_get_player_buried_corpse_count(uint32 char_id) {
	return quest_manager.getplayerburiedcorpsecount(char_id);
}

bool lua_bury_player_corpse(uint32 char_id) {
	return quest_manager.buryplayercorpse(char_id);
}

void lua_task_selector(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	int tasks[MAXCHOOSERENTRIES] = { 0 };
	int count = 0;

	for(int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto cur = table[i];
		int cur_value = 0;
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				cur_value = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		} else {
			count = i - 1;
			break;
		}

		tasks[i - 1] = cur_value;
	}
	quest_manager.taskselector(count, tasks);
}

void lua_task_set_selector(int task_set) {
	quest_manager.tasksetselector(task_set);
}

void lua_enable_task(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	int tasks[MAXCHOOSERENTRIES] = { 0 };
	int count = 0;

	for(int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto cur = table[i];
		int cur_value = 0;
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				cur_value = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		} else {
			count = i - 1;
			break;
		}

		tasks[i - 1] = cur_value;
	}

	quest_manager.enabletask(count, tasks);
}

void lua_disable_task(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	int tasks[MAXCHOOSERENTRIES] = { 0 };
	int count = 0;

	for(int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto cur = table[i];
		int cur_value = 0;
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				cur_value = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		} else {
			count = i - 1;
			break;
		}

		tasks[i - 1] = cur_value;
	}

	quest_manager.disabletask(count, tasks);
}

bool lua_is_task_enabled(int task) {
	return quest_manager.istaskenabled(task);
}

bool lua_is_task_active(int task) {
	return quest_manager.istaskactive(task);
}

bool lua_is_task_activity_active(int task, int activity) {
	return quest_manager.istaskactivityactive(task, activity);
}

int lua_get_task_activity_done_count(int task, int activity) {
	return quest_manager.gettaskactivitydonecount(task, activity);
}

void lua_update_task_activity(int task, int activity, int count) {
	quest_manager.updatetaskactivity(task, activity, count);
}

void lua_reset_task_activity(int task, int activity) {
	quest_manager.resettaskactivity(task, activity);
}

void lua_task_explored_area(int explore_id) {
	quest_manager.taskexploredarea(explore_id);
}

void lua_assign_task(int task_id) {
	quest_manager.assigntask(task_id);
}

void lua_fail_task(int task_id) {
	quest_manager.failtask(task_id);
}

int lua_task_time_left(int task_id) {
	return quest_manager.tasktimeleft(task_id);
}

int lua_is_task_completed(int task_id) {
	return quest_manager.istaskcompleted(task_id);
}

int lua_enabled_task_count(int task_set) {
	return quest_manager.enabledtaskcount(task_set);
}

int lua_first_task_in_set(int task_set) {
	return quest_manager.firsttaskinset(task_set);
}

int lua_last_task_in_set(int task_set) {
	return quest_manager.lasttaskinset(task_set);
}

int lua_next_task_in_set(int task_set, int task_id) {
	return quest_manager.nexttaskinset(task_set, task_id);
}

int lua_active_speak_task() {
	return quest_manager.activespeaktask();
}

int lua_active_speak_activity(int task_id) {
	return quest_manager.activespeakactivity(task_id);
}

int lua_active_tasks_in_set(int task_set) {
	return quest_manager.activetasksinset(task_set);
}

int lua_completed_tasks_in_set(int task_set) {
	return quest_manager.completedtasksinset(task_set);
}

bool lua_is_task_appropriate(int task) {
	return quest_manager.istaskappropriate(task);
}

std::string lua_get_task_name(uint32 task_id) {
	return quest_manager.gettaskname(task_id);
}

void lua_popup(const char *title, const char *text, uint32 id, uint32 buttons, uint32 duration) {
	quest_manager.popup(title, text, id, buttons, duration);
}

void lua_clear_spawn_timers() {
	quest_manager.clearspawntimers();
}

void lua_zone_emote(int type, const char *str) {
	quest_manager.ze(type, str);
}

void lua_world_emote(int type, const char *str) {
	quest_manager.we(type, str);
}

void lua_message(int color, const char *message) {
	quest_manager.message(color, message);
}

void lua_whisper(const char *message) {
	quest_manager.whisper(message);
}

int lua_get_level(int type) {
	return quest_manager.getlevel(type);
}

void lua_create_ground_object(uint32 item_id, float x, float y, float z, float h) {
	quest_manager.CreateGroundObject(item_id, glm::vec4(x, y, z, h));
}

void lua_create_ground_object(uint32 item_id, float x, float y, float z, float h, uint32 decay_time) {
	quest_manager.CreateGroundObject(item_id, glm::vec4(x, y, z, h), decay_time);
}

void lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h) {
	quest_manager.CreateGroundObjectFromModel(model, glm::vec4(x, y, z, h));
}

void lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h, int type) {
	quest_manager.CreateGroundObjectFromModel(model, glm::vec4(x, y, z, h), type);
}

void lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h, int type, uint32 decay_time) {
	quest_manager.CreateGroundObjectFromModel(model, glm::vec4(x, y, z, h), type, decay_time);
}

void lua_create_door(const char *model, float x, float y, float z, float h, int open_type, int size) {
	quest_manager.CreateDoor(model, x, y, z, h, open_type, size);
}

void lua_modify_npc_stat(const char *id, const char *value) {
	quest_manager.ModifyNPCStat(id, value);
}

int lua_collect_items(uint32 item_id, bool remove) {
	return quest_manager.collectitems(item_id, remove);
}

int lua_count_item(uint32 item_id) {
	return quest_manager.countitem(item_id);
}

void lua_remove_item(uint32 item_id) {
	quest_manager.removeitem(item_id);
}

void lua_remove_item(uint32 item_id, uint32 quantity) {
	quest_manager.removeitem(item_id, quantity);
}

void lua_update_spawn_timer(uint32 id, uint32 new_time) {
	quest_manager.UpdateSpawnTimer(id, new_time);
}

void lua_merchant_set_item(uint32 npc_id, uint32 item_id) {
	quest_manager.MerchantSetItem(npc_id, item_id);
}

void lua_merchant_set_item(uint32 npc_id, uint32 item_id, uint32 quantity) {
	quest_manager.MerchantSetItem(npc_id, item_id, quantity);
}

int lua_merchant_count_item(uint32 npc_id, uint32 item_id) {
	return quest_manager.MerchantCountItem(npc_id, item_id);
}

std::string lua_item_link(int item_id) {
	char text[250] = { 0 };

	return quest_manager.varlink(text, item_id);
}

std::string lua_get_item_name(uint32 item_id) {
	return quest_manager.getitemname(item_id);
}

std::string lua_say_link(const char *phrase, bool silent, const char *link_name) {
	char text[256] = { 0 };
	strncpy(text, phrase, 255);

	return quest_manager.saylink(text, silent, link_name);
}

std::string lua_say_link(const char *phrase, bool silent) {
	char text[256] = { 0 };
	strncpy(text, phrase, 255);

	return quest_manager.saylink(text, silent, text);
}

std::string lua_say_link(const char *phrase) {
	char text[256] = { 0 };
	strncpy(text, phrase, 255);

	return quest_manager.saylink(text, false, text);
}

void lua_set_rule(std::string rule_name, std::string rule_value) {
	RuleManager::Instance()->SetRule(rule_name.c_str(), rule_value.c_str());
}

std::string lua_get_rule(std::string rule_name) {
	std::string rule_value;
	RuleManager::Instance()->GetRule(rule_name.c_str(), rule_value);
	return rule_value;
}

std::string lua_get_data(std::string bucket_key) {
	return DataBucket::GetData(bucket_key);
}

std::string lua_get_data_expires(std::string bucket_key) {
	return DataBucket::GetDataExpires(bucket_key);
}

void lua_set_data(std::string bucket_key, std::string bucket_value) {
	DataBucket::SetData(bucket_key, bucket_value);
}

void lua_set_data(std::string bucket_key, std::string bucket_value, std::string expires_at) {
	DataBucket::SetData(bucket_key, bucket_value, expires_at);
}

bool lua_delete_data(std::string bucket_key) {
	return DataBucket::DeleteData(bucket_key);
}

std::string lua_get_char_name_by_id(uint32 char_id) {
	return database.GetCharNameByID(char_id);
}

uint32 lua_get_char_id_by_name(const char* name) {
	return quest_manager.getcharidbyname(name);
}

std::string lua_get_class_name(uint8 class_id) {
	return quest_manager.getclassname(class_id);
}

std::string lua_get_class_name(uint8 class_id, uint8 level) {
	return quest_manager.getclassname(class_id, level);
}

uint32 lua_get_currency_id(uint32 item_id) {
	return quest_manager.getcurrencyid(item_id);
}

uint32 lua_get_currency_item_id(uint32 currency_id) {
	return quest_manager.getcurrencyitemid(currency_id);
}

const char *lua_get_guild_name_by_id(uint32 guild_id) {
	return quest_manager.getguildnamebyid(guild_id);
}

int lua_get_guild_id_by_char_id(uint32 char_id) {
	return database.GetGuildIDByCharID(char_id);
}

int lua_get_group_id_by_char_id(uint32 char_id) {
	return database.GetGroupIDByCharID(char_id);
}

std::string lua_get_npc_name_by_id(uint32 npc_id) {
	return quest_manager.getnpcnamebyid(npc_id);
}

int lua_get_raid_id_by_char_id(uint32 char_id) {
	return database.GetRaidIDByCharID(char_id);
}

uint32 lua_create_instance(const char *zone, uint32 version, uint32 duration) {
	return quest_manager.CreateInstance(zone, version, duration);
}

void lua_destroy_instance(uint32 instance_id) {
	quest_manager.DestroyInstance(instance_id);
}

void lua_update_instance_timer(uint16 instance_id, uint32 new_duration) {
	quest_manager.UpdateInstanceTimer(instance_id, new_duration);
}

uint32 lua_get_instance_timer() {
	return quest_manager.GetInstanceTimer();
}

uint32 lua_get_instance_timer_by_id(uint16 instance_id) {
	return quest_manager.GetInstanceTimerByID(instance_id);
}

int lua_get_instance_id(const char *zone, uint32 version) {
	return quest_manager.GetInstanceID(zone, version);
}

int lua_get_instance_id_by_char_id(const char *zone, uint32 version, uint32 char_id) {
	return quest_manager.GetInstanceIDByCharID(zone, version, char_id);
}

void lua_assign_to_instance(uint32 instance_id) {
	quest_manager.AssignToInstance(instance_id);
}

void lua_assign_to_instance_by_char_id(uint32 instance_id, uint32 char_id) {
	quest_manager.AssignToInstanceByCharID(instance_id, char_id);
}

void lua_assign_group_to_instance(uint32 instance_id) {
	quest_manager.AssignGroupToInstance(instance_id);
}

void lua_assign_raid_to_instance(uint32 instance_id) {
	quest_manager.AssignRaidToInstance(instance_id);
}

void lua_remove_from_instance(uint32 instance_id) {
	quest_manager.RemoveFromInstance(instance_id);
}

void lua_remove_from_instance_by_char_id(uint32 instance_id, uint32 char_id) {
	quest_manager.RemoveFromInstanceByCharID(instance_id, char_id);
}

bool lua_check_instance_by_char_id(uint32 instance_id, uint32 char_id) {
	return quest_manager.CheckInstanceByCharID(instance_id, char_id);
}

void lua_remove_all_from_instance(uint32 instance_id) {
	quest_manager.RemoveAllFromInstance(instance_id);
}

void lua_flag_instance_by_group_leader(uint32 zone, uint32 version) {
	quest_manager.FlagInstanceByGroupLeader(zone, version);
}

void lua_flag_instance_by_raid_leader(uint32 zone, uint32 version) {
	quest_manager.FlagInstanceByRaidLeader(zone, version);
}

void lua_fly_mode(int flymode) {
	quest_manager.FlyMode(static_cast<GravityBehavior>(flymode));
}

int lua_faction_value() {
	return quest_manager.FactionValue();
}

void lua_check_title(uint32 title_set) {
	quest_manager.checktitle(title_set);
}

void lua_enable_title(uint32 title_set) {
	quest_manager.enabletitle(title_set);
}

void lua_remove_title(uint32 title_set) {
	quest_manager.removetitle(title_set);
}

void lua_wear_change(uint32 slot, uint32 texture) {
	quest_manager.wearchange(slot, texture);
}

void lua_voice_tell(const char *str, uint32 macro_num, uint32 race_num, uint32 gender_num) {
	quest_manager.voicetell(str, macro_num, race_num, gender_num);
}

void lua_send_mail(const char *to, const char *from, const char *subject, const char *message) {
	quest_manager.SendMail(to, from, subject, message);
}

luabind::adl::object lua_get_qglobals(lua_State *L, Lua_NPC npc, Lua_Client client) {
	luabind::adl::object ret = luabind::newtable(L);

	NPC *n = npc;
	Client *c = client;

	std::list<QGlobal> global_map;
	QGlobalCache::GetQGlobals(global_map, n, c, zone);
	auto iter = global_map.begin();
	while(iter != global_map.end()) {
		ret[(*iter).name] = (*iter).value;
		++iter;
	}
	return ret;
}

luabind::adl::object lua_get_qglobals(lua_State *L, Lua_Client client) {
	luabind::adl::object ret = luabind::newtable(L);

	NPC *n = nullptr;
	Client *c = client;

	std::list<QGlobal> global_map;
	QGlobalCache::GetQGlobals(global_map, n, c, zone);
	auto iter = global_map.begin();
	while (iter != global_map.end()) {
		ret[(*iter).name] = (*iter).value;
		++iter;
	}
	return ret;
}

luabind::adl::object lua_get_qglobals(lua_State *L, Lua_NPC npc) {
	luabind::adl::object ret = luabind::newtable(L);

	NPC *n = npc;
	Client *c = nullptr;

	std::list<QGlobal> global_map;
	QGlobalCache::GetQGlobals(global_map, n, c, zone);
	auto iter = global_map.begin();
	while (iter != global_map.end()) {
		ret[(*iter).name] = (*iter).value;
		++iter;
	}
	return ret;
}

luabind::adl::object lua_get_qglobals(lua_State *L) {
	luabind::adl::object ret = luabind::newtable(L);

	NPC *n = nullptr;
	Client *c = nullptr;

	std::list<QGlobal> global_map;
	QGlobalCache::GetQGlobals(global_map, n, c, zone);
	auto iter = global_map.begin();
	while (iter != global_map.end()) {
		ret[(*iter).name] = (*iter).value;
		++iter;
	}
	return ret;
}

Lua_EntityList lua_get_entity_list() {
	return Lua_EntityList(&entity_list);
}

void lua_zone(const char* zone_name) {
	quest_manager.Zone(zone_name);
}

void lua_zone_group(const char* zone_name) {
	quest_manager.ZoneGroup(zone_name);
}

void lua_zone_raid(const char* zone_name) {
	quest_manager.ZoneRaid(zone_name);
}

int lua_get_zone_id() {
	if(!zone)
		return 0;

	return zone->GetZoneID();
}

int lua_get_zone_id_by_name(const char* zone_name) {
	return ZoneID(zone_name);
}

const char *lua_get_zone_long_name() {
	if(!zone)
		return "";

	return zone->GetLongName();
}

const char *lua_get_zone_long_name_by_name(const char* zone_name) {
	return ZoneLongName(
		ZoneID(zone_name),
		true
	);
}

const char *lua_get_zone_long_name_by_id(uint32 zone_id) {
	return ZoneLongName(zone_id, true);
}

const char *lua_get_zone_short_name() {
	if(!zone)
		return "";

	return zone->GetShortName();
}

const char *lua_get_zone_short_name_by_id(uint32 zone_id) {
	return ZoneName(zone_id, true);
}

int lua_get_zone_instance_id() {
	if(!zone)
		return 0;

	return zone->GetInstanceID();
}

int lua_get_zone_instance_version() {
	if(!zone)
		return 0;

	return zone->GetInstanceVersion();
}

luabind::adl::object lua_get_characters_in_instance(lua_State *L, uint16 instance_id) {
	luabind::adl::object ret = luabind::newtable(L);

	std::list<uint32> charid_list;
	uint16 i = 1;
	database.GetCharactersInInstance(instance_id,charid_list);
	auto iter = charid_list.begin();
	while(iter != charid_list.end()) {
		ret[i] = *iter;
		++i;
		++iter;
	}
	return ret;
}

int lua_get_zone_weather() {
	if(!zone)
		return 0;

	return zone->zone_weather;
}

luabind::adl::object lua_get_zone_time(lua_State *L) {
	TimeOfDay_Struct eqTime;
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), &eqTime);

	luabind::adl::object ret = luabind::newtable(L);
	ret["zone_hour"] = eqTime.hour - 1;
	ret["zone_minute"] = eqTime.minute;
	ret["zone_time"] = (eqTime.hour - 1) * 100 + eqTime.minute;
	return ret;
}

void lua_add_area(int id, int type, float min_x, float max_x, float min_y, float max_y, float min_z, float max_z) {
	entity_list.AddArea(id, type, min_x, max_x, min_y, max_y, min_z, max_z);
}

void lua_remove_area(int id) {
	entity_list.RemoveArea(id);
}

void lua_clear_areas() {
	entity_list.ClearAreas();
}

void lua_remove_spawn_point(uint32 spawn2_id) {
	if(zone) {
		LinkedListIterator<Spawn2*> iter(zone->spawn2_list);
		iter.Reset();

		while(iter.MoreElements()) {
			Spawn2* cur = iter.GetData();
			if(cur->GetID() == spawn2_id) {
				cur->ForceDespawn();
				iter.RemoveCurrent(true);
				return;
			}

			iter.Advance();
		}
	}
}

void lua_add_spawn_point(luabind::adl::object table) {
	if(!zone)
		return;

	if(luabind::type(table) == LUA_TTABLE) {
		uint32 spawn2_id;
		uint32 spawngroup_id;
		float x;
		float y;
		float z;
		float heading;
		uint32 respawn;
		uint32 variance;
		uint32 timeleft = 0;
		uint32 grid = 0;
		bool path_when_zone_idle = false;
		int condition_id = 0;
		int condition_min_value = 0;
		bool enabled = true;
		int animation = 0;

		auto cur = table["spawn2_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				spawn2_id = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["spawngroup_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				spawngroup_id = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["x"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				x = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["y"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				y = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["z"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				z = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["heading"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				heading = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["respawn"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				respawn = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["variance"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				variance = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
				return;
			}
		} else {
			return;
		}

		cur = table["timeleft"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				timeleft = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["grid"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				grid = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["path_when_zone_idle"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				path_when_zone_idle = luabind::object_cast<bool>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["condition_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				condition_id = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["condition_min_value"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				condition_min_value = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["enabled"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				enabled = luabind::object_cast<bool>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		cur = table["animation"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				animation = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed &) {
			}
		}

		lua_remove_spawn_point(spawn2_id);

		auto t = new Spawn2(spawn2_id, spawngroup_id, x, y, z, heading, respawn,
			variance, timeleft, grid, path_when_zone_idle, condition_id,
			condition_min_value, enabled, static_cast<EmuAppearance>(animation));

		zone->spawn2_list.Insert(t);
	}
}

void lua_attack(const char *client_name) {
	quest_manager.attack(client_name);
}

void lua_attack_npc(int entity_id) {
	quest_manager.attacknpc(entity_id);
}

void lua_attack_npc_type(int npc_type) {
	quest_manager.attacknpctype(npc_type);
}

void lua_follow(int entity_id) {
	quest_manager.follow(entity_id, 10);
}

void lua_follow(int entity_id, int distance) {
	quest_manager.follow(entity_id, distance);
}

void lua_stop_follow() {
	quest_manager.sfollow();
}

Lua_Client lua_get_initiator() {
	return quest_manager.GetInitiator();
}

Lua_Mob lua_get_owner() {
	return quest_manager.GetOwner();
}

Lua_ItemInst lua_get_quest_item() {
	return quest_manager.GetQuestItem();
}

Lua_Spell lua_get_quest_spell() {
	return quest_manager.GetQuestSpell();
}

std::string lua_get_encounter() {
	return quest_manager.GetEncounter();
}

void lua_map_opcodes() {
	MapOpcodes();
}

void lua_clear_opcode(int op) {
	ClearMappedOpcode(static_cast<EmuOpcode>(op));
}

bool lua_enable_recipe(uint32 recipe_id) {
	return quest_manager.EnableRecipe(recipe_id);
}

bool lua_disable_recipe(uint32 recipe_id) {
	return quest_manager.DisableRecipe(recipe_id);
}

void lua_clear_npctype_cache(int npctype_id) {
	quest_manager.ClearNPCTypeCache(npctype_id);
}

void lua_reloadzonestaticdata() {
	quest_manager.ReloadZoneStaticData();
}

double lua_clock() {
	timeval read_time;
	gettimeofday(&read_time, nullptr);
	uint32 t = read_time.tv_sec * 1000 + read_time.tv_usec / 1000;
	return static_cast<double>(t) / 1000.0;
}

void lua_log(int category, std::string message) {
	if (category < Logs::None || category >= Logs::MaxCategoryID)
		return;

	Log(Logs::General, static_cast<Logs::LogCategory>(category), message.c_str());
}

void lua_debug(std::string message) {
	Log(Logs::General, Logs::QuestDebug, message.c_str());
}

void lua_debug(std::string message, int level) {
	if (level < Logs::General || level > Logs::Detail)
		return;

	Log(static_cast<Logs::DebugLevel>(level), Logs::QuestDebug, message.c_str());
}

void lua_log_combat(std::string message) {
	Log(Logs::General, Logs::Combat, message.c_str());
}

void lua_update_zone_header(std::string type, std::string value) {
	quest_manager.UpdateZoneHeader(type, value);
}

/**
 * Expansions
 */

bool lua_is_classic_enabled() {
	return content_service.IsClassicEnabled();
}

bool lua_is_the_ruins_of_kunark_enabled() {
	return content_service.IsTheRuinsOfKunarkEnabled();
}

bool lua_is_the_scars_of_velious_enabled() {
	return content_service.IsTheScarsOfVeliousEnabled();
}

bool lua_is_the_shadows_of_luclin_enabled() {
	return content_service.IsTheShadowsOfLuclinEnabled();
}

bool lua_is_the_planes_of_power_enabled() {
	return content_service.IsThePlanesOfPowerEnabled();
}

bool lua_is_the_legacy_of_ykesha_enabled() {
	return content_service.IsTheLegacyOfYkeshaEnabled();
}

bool lua_is_lost_dungeons_of_norrath_enabled() {
	return content_service.IsLostDungeonsOfNorrathEnabled();
}

bool lua_is_gates_of_discord_enabled() {
	return content_service.IsGatesOfDiscordEnabled();
}

bool lua_is_omens_of_war_enabled() {
	return content_service.IsOmensOfWarEnabled();
}

bool lua_is_dragons_of_norrath_enabled() {
	return content_service.IsDragonsOfNorrathEnabled();
}

bool lua_is_depths_of_darkhollow_enabled() {
	return content_service.IsDepthsOfDarkhollowEnabled();
}

bool lua_is_prophecy_of_ro_enabled() {
	return content_service.IsProphecyOfRoEnabled();
}

bool lua_is_the_serpents_spine_enabled() {
	return content_service.IsTheSerpentsSpineEnabled();
}

bool lua_is_the_buried_sea_enabled() {
	return content_service.IsTheBuriedSeaEnabled();
}

bool lua_is_secrets_of_faydwer_enabled() {
	return content_service.IsSecretsOfFaydwerEnabled();
}

bool lua_is_seeds_of_destruction_enabled() {
	return content_service.IsSeedsOfDestructionEnabled();
}

bool lua_is_underfoot_enabled() {
	return content_service.IsUnderfootEnabled();
}

bool lua_is_house_of_thule_enabled() {
	return content_service.IsHouseOfThuleEnabled();
}

bool lua_is_veil_of_alaris_enabled() {
	return content_service.IsVeilOfAlarisEnabled();
}

bool lua_is_rain_of_fear_enabled() {
	return content_service.IsRainOfFearEnabled();
}

bool lua_is_call_of_the_forsaken_enabled() {
	return content_service.IsCallOfTheForsakenEnabled();
}

bool lua_is_the_darkend_sea_enabled() {
	return content_service.IsTheDarkendSeaEnabled();
}

bool lua_is_the_broken_mirror_enabled() {
	return content_service.IsTheBrokenMirrorEnabled();
}

bool lua_is_empires_of_kunark_enabled() {
	return content_service.IsEmpiresOfKunarkEnabled();
}

bool lua_is_ring_of_scale_enabled() {
	return content_service.IsRingOfScaleEnabled();
}

bool lua_is_the_burning_lands_enabled() {
	return content_service.IsTheBurningLandsEnabled();
}

bool lua_is_torment_of_velious_enabled() {
	return content_service.IsTormentOfVeliousEnabled();
}

bool lua_is_current_expansion_classic() {
	return content_service.IsCurrentExpansionClassic();
}

bool lua_is_current_expansion_the_ruins_of_kunark() {
	return content_service.IsCurrentExpansionTheRuinsOfKunark();
}

bool lua_is_current_expansion_the_scars_of_velious() {
	return content_service.IsCurrentExpansionTheScarsOfVelious();
}

bool lua_is_current_expansion_the_shadows_of_luclin() {
	return content_service.IsCurrentExpansionTheShadowsOfLuclin();
}

bool lua_is_current_expansion_the_planes_of_power() {
	return content_service.IsCurrentExpansionThePlanesOfPower();
}

bool lua_is_current_expansion_the_legacy_of_ykesha() {
	return content_service.IsCurrentExpansionTheLegacyOfYkesha();
}

bool lua_is_current_expansion_lost_dungeons_of_norrath() {
	return content_service.IsCurrentExpansionLostDungeonsOfNorrath();
}

bool lua_is_current_expansion_gates_of_discord() {
	return content_service.IsCurrentExpansionGatesOfDiscord();
}

bool lua_is_current_expansion_omens_of_war() {
	return content_service.IsCurrentExpansionOmensOfWar();
}

bool lua_is_current_expansion_dragons_of_norrath() {
	return content_service.IsCurrentExpansionDragonsOfNorrath();
}

bool lua_is_current_expansion_depths_of_darkhollow() {
	return content_service.IsCurrentExpansionDepthsOfDarkhollow();
}

bool lua_is_current_expansion_prophecy_of_ro() {
	return content_service.IsCurrentExpansionProphecyOfRo();
}

bool lua_is_current_expansion_the_serpents_spine() {
	return content_service.IsCurrentExpansionTheSerpentsSpine();
}

bool lua_is_current_expansion_the_buried_sea() {
	return content_service.IsCurrentExpansionTheBuriedSea();
}

bool lua_is_current_expansion_secrets_of_faydwer() {
	return content_service.IsCurrentExpansionSecretsOfFaydwer();
}

bool lua_is_current_expansion_seeds_of_destruction() {
	return content_service.IsCurrentExpansionSeedsOfDestruction();
}

bool lua_is_current_expansion_underfoot() {
	return content_service.IsCurrentExpansionUnderfoot();
}

bool lua_is_current_expansion_house_of_thule() {
	return content_service.IsCurrentExpansionHouseOfThule();
}

bool lua_is_current_expansion_veil_of_alaris() {
	return content_service.IsCurrentExpansionVeilOfAlaris();
}

bool lua_is_current_expansion_rain_of_fear() {
	return content_service.IsCurrentExpansionRainOfFear();
}

bool lua_is_current_expansion_call_of_the_forsaken() {
	return content_service.IsCurrentExpansionCallOfTheForsaken();
}

bool lua_is_current_expansion_the_darkend_sea() {
	return content_service.IsCurrentExpansionTheDarkendSea();
}

bool lua_is_current_expansion_the_broken_mirror() {
	return content_service.IsCurrentExpansionTheBrokenMirror();
}

bool lua_is_current_expansion_empires_of_kunark() {
	return content_service.IsCurrentExpansionEmpiresOfKunark();
}

bool lua_is_current_expansion_ring_of_scale() {
	return content_service.IsCurrentExpansionRingOfScale();
}

bool lua_is_current_expansion_the_burning_lands() {
	return content_service.IsCurrentExpansionTheBurningLands();
}

bool lua_is_current_expansion_torment_of_velious() {
	return content_service.IsCurrentExpansionTormentOfVelious();
}

bool lua_is_content_flag_enabled(std::string content_flag){
	return content_service.IsContentFlagEnabled(content_flag);
}

void lua_set_content_flag(std::string flag_name, bool enabled){
	content_service.SetContentFlag(flag_name, enabled);
}

Lua_Expedition lua_get_expedition() {
	if (zone && zone->GetInstanceID() != 0)
	{
		return Expedition::FindCachedExpeditionByZoneInstance(zone->GetZoneID(), zone->GetInstanceID());
	}
	return nullptr;
}

Lua_Expedition lua_get_expedition_by_char_id(uint32 char_id) {
	return Expedition::FindCachedExpeditionByCharacterID(char_id);
}

Lua_Expedition lua_get_expedition_by_dz_id(uint32 dz_id) {
	return Expedition::FindCachedExpeditionByDynamicZoneID(dz_id);
}

Lua_Expedition lua_get_expedition_by_zone_instance(uint32 zone_id, uint32 instance_id) {
	return Expedition::FindCachedExpeditionByZoneInstance(zone_id, instance_id);
}

luabind::object lua_get_expedition_lockout_by_char_id(lua_State* L, uint32 char_id, std::string expedition_name, std::string event_name) {
	luabind::adl::object lua_table = luabind::newtable(L);

	auto lockouts = Expedition::GetExpeditionLockoutsByCharacterID(char_id);

	auto it = std::find_if(lockouts.begin(), lockouts.end(), [&](const ExpeditionLockoutTimer& lockout) {
		return lockout.IsSameLockout(expedition_name, event_name);
	});

	if (it != lockouts.end())
	{
		lua_table["remaining"] = it->GetSecondsRemaining();
		lua_table["uuid"] = it->GetExpeditionUUID();
	}

	return lua_table;
}

luabind::object lua_get_expedition_lockouts_by_char_id(lua_State* L, uint32 char_id) {
	luabind::adl::object lua_table = luabind::newtable(L);

	auto lockouts = Expedition::GetExpeditionLockoutsByCharacterID(char_id);
	for (const auto& lockout : lockouts)
	{
		auto lockout_table = lua_table[lockout.GetExpeditionName()];
		if (luabind::type(lockout_table) != LUA_TTABLE)
		{
			lockout_table = luabind::newtable(L);
		}

		auto event_table = lockout_table[lockout.GetEventName()];
		if (luabind::type(event_table) != LUA_TTABLE)
		{
			event_table = luabind::newtable(L);
		}

		event_table["remaining"] = lockout.GetSecondsRemaining();
		event_table["uuid"] = lockout.GetExpeditionUUID();
	}
	return lua_table;
}

luabind::object lua_get_expedition_lockouts_by_char_id(lua_State* L, uint32 char_id, std::string expedition_name) {
	luabind::adl::object lua_table = luabind::newtable(L);

	auto lockouts = Expedition::GetExpeditionLockoutsByCharacterID(char_id);
	for (const auto& lockout : lockouts)
	{
		if (lockout.GetExpeditionName() == expedition_name)
		{
			auto event_table = lua_table[lockout.GetEventName()];
			if (luabind::type(event_table) != LUA_TTABLE)
			{
				event_table = luabind::newtable(L);
			}
			event_table["remaining"] = lockout.GetSecondsRemaining();
			event_table["uuid"] = lockout.GetExpeditionUUID();
		}
	}
	return lua_table;
}

void lua_add_expedition_lockout_all_clients(std::string expedition_name, std::string event_name, uint32 seconds) {
	auto lockout = ExpeditionLockoutTimer::CreateLockout(expedition_name, event_name, seconds);
	Expedition::AddLockoutClients(lockout);
}

void lua_add_expedition_lockout_all_clients(std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid) {
	auto lockout = ExpeditionLockoutTimer::CreateLockout(expedition_name, event_name, seconds, uuid);
	Expedition::AddLockoutClients(lockout);
}

void lua_add_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name, uint32 seconds) {
	Expedition::AddLockoutByCharacterID(char_id, expedition_name, event_name, seconds);
}

void lua_add_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid) {
	Expedition::AddLockoutByCharacterID(char_id, expedition_name, event_name, seconds, uuid);
}

void lua_remove_expedition_lockout_by_char_id(uint32 char_id, std::string expedition_name, std::string event_name) {
	Expedition::RemoveLockoutsByCharacterID(char_id, expedition_name, event_name);
}

void lua_remove_all_expedition_lockouts_by_char_id(uint32 char_id) {
	Expedition::RemoveLockoutsByCharacterID(char_id);
}

void lua_remove_all_expedition_lockouts_by_char_id(uint32 char_id, std::string expedition_name) {
	Expedition::RemoveLockoutsByCharacterID(char_id, expedition_name);
}

std::string lua_seconds_to_time(int duration) {
	return quest_manager.secondstotime(duration);
}

std::string lua_get_hex_color_code(std::string color_name) {
	return quest_manager.gethexcolorcode(color_name);
}

double lua_get_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id) {
	return database.GetAAEXPModifier(character_id, zone_id);
}

double lua_get_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id) {
	return database.GetEXPModifier(character_id, zone_id);
}

void lua_set_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, double aa_modifier) {
	database.SetAAEXPModifier(character_id, zone_id, aa_modifier);
}

void lua_set_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, double exp_modifier) {
	database.SetEXPModifier(character_id, zone_id, exp_modifier);
}

void lua_add_ldon_loss(uint32 theme_id) {
	quest_manager.addldonloss(theme_id);
}

void lua_add_ldon_points(uint32 theme_id, int points) {
	quest_manager.addldonpoints(theme_id, points);
}

void lua_add_ldon_win(uint32 theme_id) {
	quest_manager.addldonwin(theme_id);
}

void lua_remove_ldon_loss(uint32 theme_id) {
	quest_manager.removeldonloss(theme_id);
}

void lua_remove_ldon_win(uint32 theme_id) {
	quest_manager.addldonwin(theme_id);
}

std::string lua_get_clean_npc_name_by_id(uint32 npc_id) {
	return quest_manager.getcleannpcnamebyid(npc_id);
}

std::string lua_get_gender_name(uint32 gender_id) {
	return quest_manager.getgendername(gender_id);
}

std::string lua_get_deity_name(uint32 deity_id) {
	return quest_manager.getdeityname(deity_id);
}

std::string lua_get_inventory_slot_name(int16 slot_id) {
	return quest_manager.getinventoryslotname(slot_id);
}

void lua_rename(std::string name) {
	quest_manager.rename(name);
}

std::string lua_get_data_remaining(std::string bucket_name) {
	return DataBucket::GetDataRemaining(bucket_name);
}

int lua_get_item_stat(uint32 item_id, std::string stat_identifier) {
	return quest_manager.getitemstat(item_id, stat_identifier);
}

int lua_get_spell_stat(uint32 spell_id, std::string stat_identifier) {
	return quest_manager.getspellstat(spell_id, stat_identifier);
}

int lua_get_spell_stat(uint32 spell_id, std::string stat_identifier, uint8 slot) {
	return quest_manager.getspellstat(spell_id, stat_identifier, slot);
}

void lua_cross_zone_add_ldon_loss_by_char_id(int character_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_group_id(int group_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_raid_id(int raid_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_guild_id(int guild_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_expedition_id(uint32 expedition_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
}

void lua_cross_zone_add_ldon_loss_by_client_name(const char* client_name, uint32 theme_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddLoss;
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_add_ldon_points_by_char_id(int character_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_group_id(int group_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_raid_id(int raid_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_guild_id(int guild_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_expedition_id(uint32 expedition_id, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id, points);
}

void lua_cross_zone_add_ldon_points_by_client_name(const char* client_name, uint32 theme_id, int points) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddPoints;
	int update_identifier = 0;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_add_ldon_win_by_char_id(int character_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_group_id(int group_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_raid_id(int raid_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_guild_id(int guild_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_expedition_id(uint32 expedition_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
}

void lua_cross_zone_add_ldon_win_by_client_name(const char* client_name, uint32 theme_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_AddWin;
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_assign_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_char_id(int character_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_group_id(int group_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_raid_id(int raid_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_guild_id(int guild_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_expedition_id(uint32 expedition_id, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_assign_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_assign_task_by_client_name(const char* client_name, uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_AssignTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_cast_spell_by_char_id(int character_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, character_id, spell_id);
}

void lua_cross_zone_cast_spell_by_group_id(int group_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, group_id, spell_id);
}

void lua_cross_zone_cast_spell_by_raid_id(int raid_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, raid_id, spell_id);
}

void lua_cross_zone_cast_spell_by_guild_id(int guild_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, guild_id, spell_id);
}

void lua_cross_zone_cast_spell_by_expedition_id(uint32 expedition_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	quest_manager.CrossZoneSpell(update_type, update_subtype, expedition_id, spell_id);
}

void lua_cross_zone_cast_spell_by_client_name(const char* client_name, uint32 spell_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZSpellUpdateSubtype_Cast;
	int update_identifier = 0;
	quest_manager.CrossZoneSpell(update_type, update_subtype, update_identifier, spell_id, client_name);
}

void lua_cross_zone_dialogue_window_by_char_id(int character_id, const char* message) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneDialogueWindow(update_type, character_id, message);
}

void lua_cross_zone_dialogue_window_by_group_id(int group_id, const char* message) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneDialogueWindow(update_type, group_id, message);
}

void lua_cross_zone_dialogue_window_by_raid_id(int raid_id, const char* message) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneDialogueWindow(update_type, raid_id, message);
}

void lua_cross_zone_dialogue_window_by_guild_id(int guild_id, const char* message) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneDialogueWindow(update_type, guild_id, message);
}

void lua_cross_zone_dialogue_window_by_expedition_id(uint32 expedition_id, const char* message) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneDialogueWindow(update_type, expedition_id, message);
}

void lua_cross_zone_dialogue_window_by_client_name(const char* client_name, const char* message) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneDialogueWindow(update_type, update_identifier, message, client_name);
}

void lua_cross_zone_disable_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_disable_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_DisableTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_enable_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_enable_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_EnableTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_fail_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_fail_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_FailTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_marquee_by_char_id(int character_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneMarquee(update_type, character_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_group_id(int group_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneMarquee(update_type, group_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_raid_id(int raid_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneMarquee(update_type, raid_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_guild_id(int guild_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneMarquee(update_type, guild_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_expedition_id(uint32 expedition_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneMarquee(update_type, expedition_id, type, priority, fade_in, fade_out, duration, message);
}

void lua_cross_zone_marquee_by_client_name(const char* client_name, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneMarquee(update_type, update_identifier, type, priority, fade_in, fade_out, duration, message, client_name);
}

void lua_cross_zone_message_player_by_char_id(uint32 type, int character_id, const char* message) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneMessage(update_type, character_id, type, message);
}

void lua_cross_zone_message_player_by_group_id(uint32 type, int group_id, const char* message) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneMessage(update_type, group_id, type, message);
}

void lua_cross_zone_message_player_by_raid_id(uint32 type, int raid_id, const char* message) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneMessage(update_type, raid_id, type, message);
}

void lua_cross_zone_message_player_by_guild_id(uint32 type, int guild_id, const char* message) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneMessage(update_type, guild_id, type, message);
}

void lua_cross_zone_message_player_by_expedition_id(uint32 type, int expedition_id, const char* message) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneMessage(update_type, expedition_id, type, message);
}

void lua_cross_zone_message_player_by_name(uint32 type, const char* client_name, const char* message) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneMessage(update_type, update_identifier, type, message, client_name);
}

void lua_cross_zone_move_player_by_char_id(int character_id, const char* zone_short_name) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZone;
	uint16 instance_id = 0;
	quest_manager.CrossZoneMove(update_type, update_subtype, character_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_player_by_group_id(int group_id, const char* zone_short_name) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZone;
	uint16 instance_id = 0;
	quest_manager.CrossZoneMove(update_type, update_subtype, group_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_player_by_raid_id(int raid_id, const char* zone_short_name) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZone;
	uint16 instance_id = 0;
	quest_manager.CrossZoneMove(update_type, update_subtype, raid_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_player_by_guild_id(int guild_id, const char* zone_short_name) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZone;
	uint16 instance_id = 0;
	quest_manager.CrossZoneMove(update_type, update_subtype, guild_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_player_by_expedition_id(int expedition_id, const char* zone_short_name) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZone;
	uint16 instance_id = 0;
	quest_manager.CrossZoneMove(update_type, update_subtype, expedition_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_player_by_client_name(const char* client_name, const char* zone_short_name) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZone;
	int update_identifier = 0;
	uint16 instance_id = 0;
	quest_manager.CrossZoneMove(update_type, update_subtype, update_identifier, zone_short_name, instance_id, client_name);
}

void lua_cross_zone_move_instance_by_char_id(int character_id, uint16 instance_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.CrossZoneMove(update_type, update_subtype, character_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_instance_by_group_id(int group_id, uint16 instance_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.CrossZoneMove(update_type, update_subtype, group_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_instance_by_raid_id(int raid_id, uint16 instance_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.CrossZoneMove(update_type, update_subtype, raid_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_instance_by_guild_id(int guild_id, uint16 instance_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.CrossZoneMove(update_type, update_subtype, guild_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_instance_by_expedition_id(uint32 expedition_id, uint16 instance_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.CrossZoneMove(update_type, update_subtype, expedition_id, zone_short_name, instance_id);
}

void lua_cross_zone_move_instance_by_client_name(const char* client_name, uint16 instance_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZMoveUpdateSubtype_MoveZoneInstance;
	int update_identifier = 0;
	const char* zone_short_name = "";
	quest_manager.CrossZoneMove(update_type, update_subtype, update_identifier, zone_short_name, instance_id, client_name);
}

void lua_cross_zone_remove_ldon_loss_by_char_id(int character_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_group_id(int group_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_raid_id(int raid_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_guild_id(int guild_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_expedition_id(uint32 expedition_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
}

void lua_cross_zone_remove_ldon_loss_by_client_name(const char* client_name, uint32 theme_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveLoss;
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_remove_ldon_win_by_char_id(int character_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_group_id(int group_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_raid_id(int raid_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_guild_id(int guild_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_expedition_id(uint32 expedition_id, uint32 theme_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
}

void lua_cross_zone_remove_ldon_win_by_client_name(const char* client_name, uint32 theme_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZLDoNUpdateSubtype_RemoveWin;
	int update_identifier = 0;
	int points = 1;
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, update_identifier, theme_id, points, client_name);
}

void lua_cross_zone_remove_spell_by_char_id(int character_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, character_id, spell_id);
}

void lua_cross_zone_remove_spell_by_group_id(int group_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, group_id, spell_id);
}

void lua_cross_zone_remove_spell_by_raid_id(int raid_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, raid_id, spell_id);
}

void lua_cross_zone_remove_spell_by_guild_id(int guild_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, guild_id, spell_id);
}

void lua_cross_zone_remove_spell_by_expedition_id(uint32 expedition_id, uint32 spell_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	quest_manager.CrossZoneSpell(update_type, update_subtype, expedition_id, spell_id);
}

void lua_cross_zone_remove_spell_by_client_name(const char* client_name, uint32 spell_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZSpellUpdateSubtype_Remove;
	int update_identifier = 0;
	quest_manager.CrossZoneSpell(update_type, update_subtype, update_identifier, spell_id, client_name);
}

void lua_cross_zone_remove_task_by_char_id(int character_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_group_id(int group_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_raid_id(int raid_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_guild_id(int guild_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_expedition_id(uint32 expedition_id, uint32 task_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_cross_zone_remove_task_by_client_name(const char* client_name, uint32 task_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_RemoveTask;
	int update_identifier = 0;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, task_subidentifier, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_reset_activity_by_char_id(int character_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_group_id(int group_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_raid_id(int raid_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_guild_id(int guild_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_expedition_id(uint32 expedition_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_reset_activity_by_client_name(const char* client_name, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityReset;
	int update_identifier = 0;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, activity_id, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_set_entity_variable_by_char_id(int character_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneSetEntityVariable(update_type, character_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_group_id(int group_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneSetEntityVariable(update_type, group_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_raid_id(int raid_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneSetEntityVariable(update_type, raid_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_guild_id(int guild_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneSetEntityVariable(update_type, guild_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_expedition_id(uint32 expedition_id, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneSetEntityVariable(update_type, expedition_id, variable_name, variable_value);
}

void lua_cross_zone_set_entity_variable_by_client_name(const char* character_name, const char* variable_name, const char* variable_value) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneSetEntityVariable(update_type, update_identifier, variable_name, variable_value, character_name);
}

void lua_cross_zone_signal_client_by_char_id(uint32 character_id, int signal) {
	uint8 update_type = CZUpdateType_Character;
	quest_manager.CrossZoneSignal(update_type, character_id, signal);
}

void lua_cross_zone_signal_client_by_group_id(uint32 group_id, int signal) {
	uint8 update_type = CZUpdateType_Group;
	quest_manager.CrossZoneSignal(update_type, group_id, signal);
}

void lua_cross_zone_signal_client_by_raid_id(uint32 raid_id, int signal) {
	uint8 update_type = CZUpdateType_Raid;
	quest_manager.CrossZoneSignal(update_type, raid_id, signal);
}

void lua_cross_zone_signal_client_by_guild_id(uint32 guild_id, int signal) {
	uint8 update_type = CZUpdateType_Guild;
	quest_manager.CrossZoneSignal(update_type, guild_id, signal);
}

void lua_cross_zone_signal_client_by_expedition_id(uint32 expedition_id, int signal) {
	uint8 update_type = CZUpdateType_Expedition;
	quest_manager.CrossZoneSignal(update_type, expedition_id, signal);
}

void lua_cross_zone_signal_client_by_name(const char* client_name, int signal) {
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	quest_manager.CrossZoneSignal(update_type, update_identifier, signal, client_name);
}

void lua_cross_zone_signal_npc_by_npctype_id(uint32 npctype_id, int signal) {
	uint8 update_type = CZUpdateType_NPC;
	quest_manager.CrossZoneSignal(update_type, npctype_id, signal);
}

void lua_cross_zone_update_activity_by_char_id(int character_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_char_id(int character_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Character;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, character_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_group_id(int group_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_group_id(int group_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Group;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, group_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_raid_id(int raid_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_raid_id(int raid_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Raid;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, raid_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_guild_id(int guild_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_guild_id(int guild_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Guild;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, guild_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_expedition_id(uint32 expedition_id, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, activity_id, update_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_expedition_id(uint32 expedition_id, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_Expedition;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, expedition_id, task_id, activity_id, activity_count, enforce_level_requirement);
}

void lua_cross_zone_update_activity_by_client_name(const char* client_name, uint32 task_id, int activity_id) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_identifier = 0;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, activity_id, update_count, enforce_level_requirement, client_name);
}

void lua_cross_zone_update_activity_by_client_name(const char* client_name, uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = CZUpdateType_ClientName;
	uint8 update_subtype = CZTaskUpdateSubtype_ActivityUpdate;
	int update_identifier = 0;
	bool enforce_level_requirement = false;
	quest_manager.CrossZoneTaskUpdate(update_type, update_subtype, update_identifier, task_id, activity_id, activity_count, enforce_level_requirement, client_name);
}

void lua_world_wide_add_ldon_loss(uint32 theme_id) {
	uint8 update_type = WWLDoNUpdateType_AddLoss;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id);
}

void lua_world_wide_add_ldon_loss(uint32 theme_id, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_AddLoss;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_add_ldon_loss(uint32 theme_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_AddLoss;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_add_ldon_points(uint32 theme_id, int points) {
	uint8 update_type = WWLDoNUpdateType_AddPoints;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points);
}

void lua_world_wide_add_ldon_points(uint32 theme_id, int points, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_AddPoints;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_add_ldon_points(uint32 theme_id, int points, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_AddPoints;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_add_ldon_win(uint32 theme_id) {
	uint8 update_type = WWLDoNUpdateType_AddWin;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id);
}

void lua_world_wide_add_ldon_win(uint32 theme_id, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_AddWin;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_add_ldon_win(uint32 theme_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_AddWin;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_assign_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_AssignTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_assign_task(uint32 task_id, bool enforce_level_requirement) {
	uint8 update_type = WWTaskUpdateType_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement);
}

void lua_world_wide_assign_task(uint32 task_id, bool enforce_level_requirement, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_assign_task(uint32 task_id, bool enforce_level_requirement, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_AssignTask;
	int task_subidentifier = -1;
	int update_count = 1;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_cast_spell(uint32 spell_id) {
	uint8 update_type = WWSpellUpdateType_Cast;
	quest_manager.WorldWideSpell(update_type, spell_id);
}

void lua_world_wide_cast_spell(uint32 spell_id, uint8 min_status) {
	uint8 update_type = WWSpellUpdateType_Cast;
	quest_manager.WorldWideSpell(update_type, spell_id, min_status);
}

void lua_world_wide_cast_spell(uint32 spell_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWSpellUpdateType_Cast;
	quest_manager.WorldWideSpell(update_type, spell_id, min_status, max_status);
}

void lua_world_wide_dialogue_window(const char* message) {
	quest_manager.WorldWideDialogueWindow(message);
}

void lua_world_wide_dialogue_window(const char* message, uint8 min_status) {
	quest_manager.WorldWideDialogueWindow(message, min_status);
}

void lua_world_wide_dialogue_window(const char* message, uint8 min_status, uint8 max_status) {
	quest_manager.WorldWideDialogueWindow(message, min_status, max_status);
}

void lua_world_wide_disable_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_DisableTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_disable_task(uint32 task_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_disable_task(uint32 task_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_DisableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_enable_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_EnableTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_enable_task(uint32 task_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_enable_task(uint32 task_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_EnableTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_fail_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_FailTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_fail_task(uint32 task_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_fail_task(uint32 task_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_FailTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message) {
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message);
}

void lua_world_wide_marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status) {
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message, min_status);
}

void lua_world_wide_marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status, uint8 max_status) {
	quest_manager.WorldWideMarquee(type, priority, fade_in, fade_out, duration, message, min_status, max_status);
}

void lua_world_wide_message(uint32 type, const char* message) {
	quest_manager.WorldWideMessage(type, message);
}

void lua_world_wide_message(uint32 type, const char* message, uint8 min_status) {
	quest_manager.WorldWideMessage(type, message, min_status);
}

void lua_world_wide_message(uint32 type, const char* message, uint8 min_status, uint8 max_status) {
	quest_manager.WorldWideMessage(type, message, min_status, max_status);
}

void lua_world_wide_move(const char* zone_short_name) {
	uint8 update_type = WWMoveUpdateType_MoveZone;
	quest_manager.WorldWideMove(update_type, zone_short_name);
}

void lua_world_wide_move(const char* zone_short_name, uint8 min_status) {
	uint8 update_type = WWMoveUpdateType_MoveZone;
	uint16 instance_id = 0;
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id, min_status);
}

void lua_world_wide_move(const char* zone_short_name, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWMoveUpdateType_MoveZone;
	uint16 instance_id = 0;
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id, min_status, max_status);
}

void lua_world_wide_move_instance(uint16 instance_id) {
	uint8 update_type = WWMoveUpdateType_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id);
}

void lua_world_wide_move_instance(uint16 instance_id, uint8 min_status) {
	uint8 update_type = WWMoveUpdateType_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id, min_status);
}

void lua_world_wide_move_instance(uint16 instance_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWMoveUpdateType_MoveZoneInstance;
	const char* zone_short_name = "";
	quest_manager.WorldWideMove(update_type, zone_short_name, instance_id, min_status, max_status);
}

void lua_world_wide_remove_ldon_loss(uint32 theme_id) {
	uint8 update_type = WWLDoNUpdateType_RemoveLoss;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id);
}

void lua_world_wide_remove_ldon_loss(uint32 theme_id, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_RemoveLoss;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_remove_ldon_loss(uint32 theme_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_RemoveLoss;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_remove_ldon_win(uint32 theme_id) {
	uint8 update_type = WWLDoNUpdateType_RemoveWin;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id);
}

void lua_world_wide_remove_ldon_win(uint32 theme_id, uint8 min_status) {
	uint8 update_type = WWLDoNUpdateType_RemoveWin;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status);
}

void lua_world_wide_remove_ldon_win(uint32 theme_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWLDoNUpdateType_RemoveWin;
	int points = 1;
	quest_manager.WorldWideLDoNUpdate(update_type, theme_id, points, min_status, max_status);
}

void lua_world_wide_remove_spell(uint32 spell_id) {
	uint8 update_type = WWSpellUpdateType_Remove;
	quest_manager.WorldWideSpell(update_type, spell_id);
}

void lua_world_wide_remove_spell(uint32 spell_id, uint8 min_status) {
	uint8 update_type = WWSpellUpdateType_Remove;
	quest_manager.WorldWideSpell(update_type, spell_id, min_status);
}

void lua_world_wide_remove_spell(uint32 spell_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWSpellUpdateType_Remove;
	quest_manager.WorldWideSpell(update_type, spell_id, min_status, max_status);
}

void lua_world_wide_remove_task(uint32 task_id) {
	uint8 update_type = WWTaskUpdateType_RemoveTask;
	quest_manager.WorldWideTaskUpdate(update_type, task_id);
}

void lua_world_wide_remove_task(uint32 task_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_remove_task(uint32 task_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_RemoveTask;
	int task_subidentifier = -1;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, task_subidentifier, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_reset_activity(uint32 task_id, int activity_id) {
	uint8 update_type = WWTaskUpdateType_ActivityReset;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id);
}

void lua_world_wide_reset_activity(uint32 task_id, int activity_id, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, update_count, enforce_level_requirement, min_status);
}

void lua_world_wide_reset_activity(uint32 task_id, int activity_id, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_ActivityReset;
	int update_count = 1;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, update_count, enforce_level_requirement, min_status, max_status);
}

void lua_world_wide_set_entity_variable_client(const char* variable_name, const char* variable_value) {
	uint8 update_type = WWSetEntityVariableUpdateType_Character;
	quest_manager.WorldWideSetEntityVariable(update_type, variable_name, variable_value);
}

void lua_world_wide_set_entity_variable_client(const char* variable_name, const char* variable_value, uint8 min_status) {
	uint8 update_type = WWSetEntityVariableUpdateType_Character;
	quest_manager.WorldWideSetEntityVariable(update_type, variable_name, variable_value, min_status);
}

void lua_world_wide_set_entity_variable_client(const char* variable_name, const char* variable_value, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWSetEntityVariableUpdateType_Character;
	quest_manager.WorldWideSetEntityVariable(update_type, variable_name, variable_value, min_status, max_status);
}

void lua_world_wide_set_entity_variable_npc(const char* variable_name, const char* variable_value) {
	uint8 update_type = WWSetEntityVariableUpdateType_NPC;
	quest_manager.WorldWideSetEntityVariable(update_type, variable_name, variable_value);
}

void lua_world_wide_signal_client(uint32 signal) {
	uint8 update_type = WWSignalUpdateType_Character;
	quest_manager.WorldWideSignal(update_type, signal);
}

void lua_world_wide_signal_client(uint32 signal, uint8 min_status) {
	uint8 update_type = WWSignalUpdateType_Character;
	quest_manager.WorldWideSignal(update_type, signal, min_status);
}

void lua_world_wide_signal_client(uint32 signal, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWSignalUpdateType_Character;
	quest_manager.WorldWideSignal(update_type, signal, min_status, max_status);
}

void lua_world_wide_signal_npc(uint32 signal) {
	uint8 update_type = WWSignalUpdateType_NPC;
	quest_manager.WorldWideSignal(update_type, signal);
}

void lua_world_wide_update_activity(uint32 task_id, int activity_id) {
	uint8 update_type = WWTaskUpdateType_ActivityUpdate;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id);
}

void lua_world_wide_update_activity(uint32 task_id, int activity_id, int activity_count) {
	uint8 update_type = WWTaskUpdateType_ActivityUpdate;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, activity_count);
}

void lua_world_wide_update_activity(uint32 task_id, int activity_id, int activity_count, uint8 min_status) {
	uint8 update_type = WWTaskUpdateType_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, activity_count, enforce_level_requirement, min_status);
}

void lua_world_wide_update_activity(uint32 task_id, int activity_id, int activity_count, uint8 min_status, uint8 max_status) {
	uint8 update_type = WWTaskUpdateType_ActivityUpdate;
	bool enforce_level_requirement = false;
	quest_manager.WorldWideTaskUpdate(update_type, task_id, activity_id, activity_count, enforce_level_requirement, min_status, max_status);
}

bool lua_is_npc_spawned(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return false;
	}

	std::vector<uint32> npc_ids;
	int index = 1;
	while (luabind::type(table[index]) != LUA_TNIL) {
		auto current_id = table[index];
		uint32 npc_id = 0;
		if(luabind::type(current_id) != LUA_TNIL) {
			try {
				npc_id = luabind::object_cast<int>(current_id);
			} catch(luabind::cast_failed &) {
			}
		} else {
			break;
		}

		npc_ids.push_back(npc_id);
		++index;
	}

	if (npc_ids.empty()) {
		return false;
	}

	return entity_list.IsNPCSpawned(npc_ids);
}

uint32 lua_count_spawned_npcs(luabind::adl::object table) {
	if(luabind::type(table) != LUA_TTABLE) {
		return 0;
	}

	std::vector<uint32> npc_ids;
	int index = 1;
	while (luabind::type(table[index]) != LUA_TNIL) {
		auto current_id = table[index];
		uint32 npc_id = 0;
		if(luabind::type(current_id) != LUA_TNIL) {
			try {
				npc_id = luabind::object_cast<int>(current_id);
			} catch(luabind::cast_failed &) {
			}
		} else {
			break;
		}

		npc_ids.push_back(npc_id);
		++index;
	}

	if (npc_ids.empty()) {
		return 0;
	}

	return entity_list.CountSpawnedNPCs(npc_ids);
}

Lua_Spell lua_get_spell(uint32 spell_id) {
	return Lua_Spell(spell_id);
}

std::string lua_get_ldon_theme_name(uint32 theme_id) {
	return quest_manager.getldonthemename(theme_id);
}

std::string lua_get_faction_name(int faction_id) {
	return quest_manager.getfactionname(faction_id);
}

std::string lua_get_language_name(int language_id) {
	return quest_manager.getlanguagename(language_id);
}

std::string lua_get_body_type_name(uint32 bodytype_id) {
	return quest_manager.getbodytypename(bodytype_id);
}

std::string lua_get_consider_level_name(uint8 consider_level) {
	return quest_manager.getconsiderlevelname(consider_level);
}

std::string lua_get_environmental_damage_name(uint8 damage_type) {
	return quest_manager.getenvironmentaldamagename(damage_type);
}

#define LuaCreateNPCParse(name, c_type, default_value) do { \
	cur = table[#name]; \
	if(luabind::type(cur) != LUA_TNIL) { \
		try { \
			npc_type->name = luabind::object_cast<c_type>(cur); \
		} \
		catch(luabind::cast_failed &) { \
			npc_type->size = default_value; \
		} \
	} \
	else { \
		npc_type->size = default_value; \
	} \
} while(0)

#define LuaCreateNPCParseString(name, str_length, default_value) do { \
	cur = table[#name]; \
	if(luabind::type(cur) != LUA_TNIL) { \
		try { \
			std::string tmp = luabind::object_cast<std::string>(cur); \
			strncpy(npc_type->name, tmp.c_str(), str_length); \
		} \
		catch(luabind::cast_failed &) { \
			strncpy(npc_type->name, default_value, str_length); \
		} \
	} \
	else { \
		strncpy(npc_type->name, default_value, str_length); \
	} \
} while(0)

void lua_create_npc(luabind::adl::object table, float x, float y, float z, float heading) {
	if(luabind::type(table) != LUA_TTABLE) {
		return;
	}

	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));


	luabind::adl::index_proxy<luabind::adl::object> cur = table["name"];
	LuaCreateNPCParseString(name, 64, "_");
	LuaCreateNPCParseString(lastname, 64, "");
	LuaCreateNPCParse(current_hp, int32, 30);
	LuaCreateNPCParse(max_hp, int32, 30);
	LuaCreateNPCParse(size, float, 6.0f);
	LuaCreateNPCParse(runspeed, float, 1.25f);
	LuaCreateNPCParse(gender, uint8, 0);
	LuaCreateNPCParse(race, uint16, 1);
	LuaCreateNPCParse(class_, uint8, WARRIOR);
	LuaCreateNPCParse(bodytype, uint8, 0);
	LuaCreateNPCParse(deity, uint8, 0);
	LuaCreateNPCParse(level, uint8, 1);
	LuaCreateNPCParse(npc_id, uint32, 1);
	LuaCreateNPCParse(texture, uint8, 0);
	LuaCreateNPCParse(helmtexture, uint8, 0);
	LuaCreateNPCParse(loottable_id, uint32, 0);
	LuaCreateNPCParse(npc_spells_id, uint32, 0);
	LuaCreateNPCParse(npc_spells_effects_id, uint32, 0);
	LuaCreateNPCParse(npc_faction_id, int32, 0);
	LuaCreateNPCParse(merchanttype, uint32, 0);
	LuaCreateNPCParse(alt_currency_type, uint32, 0);
	LuaCreateNPCParse(adventure_template, uint32, 0);
	LuaCreateNPCParse(trap_template, uint32, 0);
	LuaCreateNPCParse(light, uint8, 0);
	LuaCreateNPCParse(AC, uint32, 0);
	LuaCreateNPCParse(Mana, uint32, 0);
	LuaCreateNPCParse(ATK, uint32, 0);
	LuaCreateNPCParse(STR, uint32, 0);
	LuaCreateNPCParse(STA, uint32, 0);
	LuaCreateNPCParse(DEX, uint32, 0);
	LuaCreateNPCParse(AGI, uint32, 0);
	LuaCreateNPCParse(INT, uint32, 0);
	LuaCreateNPCParse(WIS, uint32, 0);
	LuaCreateNPCParse(CHA, uint32, 0);
	LuaCreateNPCParse(MR, int32, 0);
	LuaCreateNPCParse(FR, int32, 0);
	LuaCreateNPCParse(CR, int32, 0);
	LuaCreateNPCParse(PR, int32, 0);
	LuaCreateNPCParse(DR, int32, 0);
	LuaCreateNPCParse(Corrup, int32, 0);
	LuaCreateNPCParse(PhR, int32, 0);
	LuaCreateNPCParse(haircolor, uint8, 0);
	LuaCreateNPCParse(beardcolor, uint8, 0);
	LuaCreateNPCParse(eyecolor1, uint8, 0);
	LuaCreateNPCParse(eyecolor2, uint8, 0);
	LuaCreateNPCParse(hairstyle, uint8, 0);
	LuaCreateNPCParse(luclinface, uint8, 0);
	LuaCreateNPCParse(beard, uint8, 0);
	LuaCreateNPCParse(drakkin_heritage, uint32, 0);
	LuaCreateNPCParse(drakkin_tattoo, uint32, 0);
	LuaCreateNPCParse(drakkin_details, uint32, 0);
	LuaCreateNPCParse(armor_tint.Head.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Chest.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Arms.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Wrist.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Hands.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Legs.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Feet.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Primary.Color, uint32, 0);
	LuaCreateNPCParse(armor_tint.Secondary.Color, uint32, 0);
	LuaCreateNPCParse(min_dmg, uint32, 2);
	LuaCreateNPCParse(max_dmg, uint32, 4);
	LuaCreateNPCParse(attack_count, int16, 0);
	LuaCreateNPCParseString(special_abilities, 512, "");
	LuaCreateNPCParse(d_melee_texture1, uint16, 0);
	LuaCreateNPCParse(d_melee_texture2, uint16, 0);
	LuaCreateNPCParseString(ammo_idfile, 30, "");
	LuaCreateNPCParse(prim_melee_type, uint8, 0);
	LuaCreateNPCParse(sec_melee_type, uint8, 0);
	LuaCreateNPCParse(ranged_type, uint8, 0);
	LuaCreateNPCParse(hp_regen, int32, 1);
	LuaCreateNPCParse(mana_regen, int32, 1);
	LuaCreateNPCParse(aggroradius, int32, 0);
	LuaCreateNPCParse(assistradius, int32, 0);
	LuaCreateNPCParse(see_invis, uint8, 0);
	LuaCreateNPCParse(see_invis_undead, bool, false);
	LuaCreateNPCParse(see_hide, bool, false);
	LuaCreateNPCParse(see_improved_hide, bool, false);
	LuaCreateNPCParse(qglobal, bool, false);
	LuaCreateNPCParse(npc_aggro, bool, false);
	LuaCreateNPCParse(spawn_limit, uint8, false);
	LuaCreateNPCParse(mount_color, uint8, false);
	LuaCreateNPCParse(attack_speed, float, 0);
	LuaCreateNPCParse(attack_delay, uint8, 30);
	LuaCreateNPCParse(accuracy_rating, int, 0);
	LuaCreateNPCParse(avoidance_rating, int, 0);
	LuaCreateNPCParse(findable, bool, false);
	LuaCreateNPCParse(trackable, bool, false);
	LuaCreateNPCParse(slow_mitigation, int16, 0);
	LuaCreateNPCParse(maxlevel, uint8, 0);
	LuaCreateNPCParse(scalerate, uint32, 0);
	LuaCreateNPCParse(private_corpse, bool, false);
	LuaCreateNPCParse(unique_spawn_by_name, bool, false);
	LuaCreateNPCParse(underwater, bool, false);
	LuaCreateNPCParse(emoteid, uint32, 0);
	LuaCreateNPCParse(spellscale, float, 0);
	LuaCreateNPCParse(healscale, float, 0);
	LuaCreateNPCParse(no_target_hotkey, bool, false);
	LuaCreateNPCParse(raid_target, bool, false);

	NPC* npc = new NPC(npc_type, nullptr, glm::vec4(x, y, z, heading), GravityBehavior::Water);
	npc->GiveNPCTypeData(npc_type);
	entity_list.AddNPC(npc);
}

int random_int(int low, int high) {
	return zone->random.Int(low, high);
}

double random_real(double low, double high) {
	return zone->random.Real(low, high);
}

bool random_roll_int(int required) {
	return zone->random.Roll(required);
}

bool random_roll_real(double required) {
	return zone->random.Roll(required);
}

int random_roll0(int max) {
	return zone->random.Roll0(max);
}

int get_rulei(int rule) {
	return RuleManager::Instance()->GetIntRule((RuleManager::IntType)rule);
}

float get_ruler(int rule) {
	return RuleManager::Instance()->GetRealRule((RuleManager::RealType)rule);
}

bool get_ruleb(int rule) {
	return RuleManager::Instance()->GetBoolRule((RuleManager::BoolType)rule);
}

luabind::scope lua_register_general() {
	return luabind::namespace_("eq")
	[
		luabind::def("load_encounter", &load_encounter),
		luabind::def("unload_encounter", &unload_encounter),
		luabind::def("load_encounter_with_data", &load_encounter_with_data),
		luabind::def("unload_encounter_with_data", &unload_encounter_with_data),
		luabind::def("register_npc_event", (void(*)(std::string, int, int, luabind::adl::object))&register_npc_event),
		luabind::def("register_npc_event", (void(*)(int, int, luabind::adl::object))&register_npc_event),
		luabind::def("unregister_npc_event", (void(*)(std::string, int, int))&unregister_npc_event),
		luabind::def("unregister_npc_event", (void(*)(int, int))&unregister_npc_event),
		luabind::def("register_player_event", (void(*)(std::string, int, luabind::adl::object))&register_player_event),
		luabind::def("register_player_event", (void(*)(int, luabind::adl::object))&register_player_event),
		luabind::def("unregister_player_event", (void(*)(std::string, int))&unregister_player_event),
		luabind::def("unregister_player_event", (void(*)(int))&unregister_player_event),
		luabind::def("register_item_event", (void(*)(std::string, int, int, luabind::adl::object))&register_item_event),
		luabind::def("register_item_event", (void(*)(int, int, luabind::adl::object))&register_item_event),
		luabind::def("unregister_item_event", (void(*)(std::string, int, int))&unregister_item_event),
		luabind::def("unregister_item_event", (void(*)(int, int))&unregister_item_event),
		luabind::def("register_spell_event", (void(*)(std::string, int, int, luabind::adl::object func))&register_spell_event),
		luabind::def("register_spell_event", (void(*)(int, int, luabind::adl::object func))&register_spell_event),
		luabind::def("unregister_spell_event", (void(*)(std::string, int, int))&unregister_spell_event),
		luabind::def("unregister_spell_event", (void(*)(int, int))&unregister_spell_event),
		luabind::def("spawn2", (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_spawn2),
		luabind::def("unique_spawn", (Lua_Mob(*)(int,int,int,double,double,double))&lua_unique_spawn),
		luabind::def("unique_spawn", (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_unique_spawn),
		luabind::def("spawn_from_spawn2", (Lua_Mob(*)(uint32))&lua_spawn_from_spawn2),
		luabind::def("enable_spawn2", &lua_enable_spawn2),
		luabind::def("disable_spawn2", &lua_disable_spawn2),
		luabind::def("has_timer", (bool(*)(const char*))&lua_has_timer),
		luabind::def("get_remaining_time", (uint32(*)(const char*))&lua_get_remaining_time),
		luabind::def("get_timer_duration", (uint32(*)(const char*))&lua_get_timer_duration),
		luabind::def("set_timer", (void(*)(const char*, int))&lua_set_timer),
		luabind::def("set_timer", (void(*)(const char*, int, Lua_ItemInst))&lua_set_timer),
		luabind::def("set_timer", (void(*)(const char*, int, Lua_Mob))&lua_set_timer),
		luabind::def("set_timer", (void(*)(const char*, int, Lua_Encounter))&lua_set_timer),
		luabind::def("stop_timer", (void(*)(const char*))&lua_stop_timer),
		luabind::def("stop_timer", (void(*)(const char*, Lua_ItemInst))&lua_stop_timer),
		luabind::def("stop_timer", (void(*)(const char*, Lua_Mob))&lua_stop_timer),
		luabind::def("stop_timer", (void(*)(const char*, Lua_Encounter))&lua_stop_timer),
		luabind::def("pause_timer", (void(*)(const char*))&lua_pause_timer),
		luabind::def("resume_timer", (void(*)(const char*))&lua_resume_timer),
		luabind::def("is_paused_timer", (bool(*)(const char*))&lua_is_paused_timer),
		luabind::def("stop_all_timers", (void(*)(void))&lua_stop_all_timers),
		luabind::def("stop_all_timers", (void(*)(Lua_ItemInst))&lua_stop_all_timers),
		luabind::def("stop_all_timers", (void(*)(Lua_Mob))&lua_stop_all_timers),
		luabind::def("stop_all_timers", (void(*)(Lua_Encounter))&lua_stop_all_timers),
		luabind::def("depop", (void(*)(void))&lua_depop),
		luabind::def("depop", (void(*)(int))&lua_depop),
		luabind::def("depop_with_timer", (void(*)(void))&lua_depop_with_timer),
		luabind::def("depop_with_timer", (void(*)(int))&lua_depop_with_timer),
		luabind::def("depop_all", (void(*)(void))&lua_depop_all),
		luabind::def("depop_all", (void(*)(int))&lua_depop_all),
		luabind::def("depop_zone", &lua_depop_zone),
		luabind::def("repop_zone", &lua_repop_zone),
		luabind::def("process_mobs_while_zone_empty", &lua_process_mobs_while_zone_empty),
		luabind::def("is_disc_tome", &lua_is_disc_tome),
		luabind::def("get_race_name", (std::string(*)(uint16))&lua_get_race_name),
		luabind::def("get_spell_name", (std::string(*)(uint32))&lua_get_spell_name),
		luabind::def("get_skill_name", (std::string(*)(int))&lua_get_skill_name),
		luabind::def("safe_move", &lua_safe_move),
		luabind::def("rain", &lua_rain),
		luabind::def("snow", &lua_snow),
		luabind::def("scribe_spells", (int(*)(int))&lua_scribe_spells),
		luabind::def("scribe_spells", (int(*)(int,int))&lua_scribe_spells),
		luabind::def("train_discs", (int(*)(int))&lua_train_discs),
		luabind::def("train_discs", (int(*)(int,int))&lua_train_discs),
		luabind::def("set_sky", &lua_set_sky),
		luabind::def("set_guild", &lua_set_guild),
		luabind::def("create_guild", &lua_create_guild),
		luabind::def("set_time", (void(*)(int, int))&lua_set_time),
		luabind::def("set_time", (void(*)(int, int, bool))&lua_set_time),
		luabind::def("signal", (void(*)(int,int))&lua_signal),
		luabind::def("signal", (void(*)(int,int,int))&lua_signal),
		luabind::def("set_global", &lua_set_global),
		luabind::def("target_global", &lua_target_global),
		luabind::def("delete_global", &lua_delete_global),
		luabind::def("start", &lua_start),
		luabind::def("stop", &lua_stop),
		luabind::def("pause", &lua_pause),
		luabind::def("move_to", (void(*)(float,float,float))&lua_move_to),
		luabind::def("move_to", (void(*)(float,float,float,float))&lua_move_to),
		luabind::def("move_to", (void(*)(float,float,float,float,bool))&lua_move_to),
		luabind::def("resume", &lua_path_resume),
		luabind::def("set_next_hp_event", &lua_set_next_hp_event),
		luabind::def("set_next_inc_hp_event", &lua_set_next_inc_hp_event),
		luabind::def("respawn", &lua_respawn),
		luabind::def("set_proximity", (void(*)(float,float,float,float))&lua_set_proximity),
		luabind::def("set_proximity", (void(*)(float,float,float,float,float,float))&lua_set_proximity),
		luabind::def("set_proximity", (void(*)(float,float,float,float,float,float,bool))&lua_set_proximity),
		luabind::def("clear_proximity", &lua_clear_proximity),
		luabind::def("enable_proximity_say", &lua_enable_proximity_say),
		luabind::def("disable_proximity_say", &lua_disable_proximity_say),
		luabind::def("set_anim", &lua_set_anim),
		luabind::def("spawn_condition", &lua_spawn_condition),
		luabind::def("get_spawn_condition", &lua_get_spawn_condition),
		luabind::def("toggle_spawn_event", &lua_toggle_spawn_event),
		luabind::def("summon_buried_player_corpse", &lua_summon_buried_player_corpse),
		luabind::def("summon_all_player_corpses", &lua_summon_all_player_corpses),
		luabind::def("get_player_corpse_count", &lua_get_player_corpse_count),
		luabind::def("get_player_corpse_count_by_zone_id", &lua_get_player_corpse_count_by_zone_id),
		luabind::def("get_player_buried_corpse_count", &lua_get_player_buried_corpse_count),
		luabind::def("bury_player_corpse", &lua_bury_player_corpse),
		luabind::def("task_selector", &lua_task_selector),
		luabind::def("task_set_selector", &lua_task_set_selector),
		luabind::def("enable_task", &lua_enable_task),
		luabind::def("disable_task", &lua_disable_task),
		luabind::def("is_task_enabled", &lua_is_task_enabled),
		luabind::def("is_task_active", &lua_is_task_active),
		luabind::def("is_task_activity_active", &lua_is_task_activity_active),
		luabind::def("get_task_activity_done_count", &lua_get_task_activity_done_count),
		luabind::def("update_task_activity", &lua_update_task_activity),
		luabind::def("reset_task_activity", &lua_reset_task_activity),
		luabind::def("task_explored_area", &lua_task_explored_area),
		luabind::def("assign_task", &lua_assign_task),
		luabind::def("fail_task", &lua_fail_task),
		luabind::def("task_time_left", &lua_task_time_left),
		luabind::def("is_task_completed", &lua_is_task_completed),
		luabind::def("enabled_task_count", &lua_enabled_task_count),
		luabind::def("first_task_in_set", &lua_first_task_in_set),
		luabind::def("last_task_in_set", &lua_last_task_in_set),
		luabind::def("next_task_in_set", &lua_next_task_in_set),
		luabind::def("active_speak_task", &lua_active_speak_task),
		luabind::def("active_speak_activity", &lua_active_speak_activity),
		luabind::def("active_tasks_in_set", &lua_active_tasks_in_set),
		luabind::def("completed_tasks_in_set", &lua_completed_tasks_in_set),
		luabind::def("is_task_appropriate", &lua_is_task_appropriate),
		luabind::def("get_task_name", (std::string(*)(uint32))&lua_get_task_name),
		luabind::def("popup", &lua_popup),
		luabind::def("clear_spawn_timers", &lua_clear_spawn_timers),
		luabind::def("zone_emote", &lua_zone_emote),
		luabind::def("world_emote", &lua_world_emote),
		luabind::def("message", &lua_message),
		luabind::def("whisper", &lua_whisper),
		luabind::def("get_level", &lua_get_level),
		luabind::def("create_ground_object", (void(*)(uint32,float,float,float,float))&lua_create_ground_object),
		luabind::def("create_ground_object", (void(*)(uint32,float,float,float,float,uint32))&lua_create_ground_object),
		luabind::def("create_ground_object_from_model", (void(*)(const char*,float,float,float,float))&lua_create_ground_object_from_model),
		luabind::def("create_ground_object_from_model", (void(*)(const char*,float,float,float,float,int))&lua_create_ground_object_from_model),
		luabind::def("create_ground_object_from_model", (void(*)(const char*,float,float,float,float,int,uint32))&lua_create_ground_object_from_model),
		luabind::def("create_door", &lua_create_door),
		luabind::def("modify_npc_stat", &lua_modify_npc_stat),
		luabind::def("collect_items", &lua_collect_items),
		luabind::def("count_item", &lua_count_item),
		luabind::def("remove_item", (void(*)(uint32))&lua_remove_item),
		luabind::def("remove_item", (void(*)(uint32,uint32))&lua_remove_item),
		luabind::def("update_spawn_timer", &lua_update_spawn_timer),
		luabind::def("merchant_set_item", (void(*)(uint32,uint32))&lua_merchant_set_item),
		luabind::def("merchant_set_item", (void(*)(uint32,uint32,uint32))&lua_merchant_set_item),
		luabind::def("merchant_count_item", &lua_merchant_count_item),
		luabind::def("item_link", &lua_item_link),
		luabind::def("get_item_name", (std::string(*)(uint32))&lua_get_item_name),
		luabind::def("say_link", (std::string(*)(const char*,bool,const char*))&lua_say_link),
		luabind::def("say_link", (std::string(*)(const char*,bool))&lua_say_link),
		luabind::def("say_link", (std::string(*)(const char*))&lua_say_link),
		luabind::def("set_rule", (void(*)(std::string, std::string))&lua_set_rule),
		luabind::def("get_rule", (std::string(*)(std::string))&lua_get_rule),
		luabind::def("get_data", (std::string(*)(std::string))&lua_get_data),
		luabind::def("get_data_expires", (std::string(*)(std::string))&lua_get_data_expires),
		luabind::def("set_data", (void(*)(std::string, std::string))&lua_set_data),
		luabind::def("set_data", (void(*)(std::string, std::string, std::string))&lua_set_data),
		luabind::def("delete_data", (bool(*)(std::string))&lua_delete_data),
		luabind::def("get_char_name_by_id", &lua_get_char_name_by_id),
		luabind::def("get_char_id_by_name", (uint32(*)(const char*))&lua_get_char_id_by_name),
		luabind::def("get_class_name", (std::string(*)(uint8))&lua_get_class_name),
		luabind::def("get_class_name", (std::string(*)(uint8,uint8))&lua_get_class_name),
		luabind::def("get_clean_npc_name_by_id", &lua_get_clean_npc_name_by_id),
		luabind::def("get_currency_id", &lua_get_currency_id),
		luabind::def("get_currency_item_id", &lua_get_currency_item_id),
		luabind::def("get_guild_name_by_id", &lua_get_guild_name_by_id),
		luabind::def("get_guild_id_by_char_id", &lua_get_guild_id_by_char_id),
		luabind::def("get_group_id_by_char_id", &lua_get_group_id_by_char_id),
		luabind::def("get_npc_name_by_id", &lua_get_npc_name_by_id),
		luabind::def("get_raid_id_by_char_id", &lua_get_raid_id_by_char_id),
		luabind::def("create_instance", &lua_create_instance),
		luabind::def("destroy_instance", &lua_destroy_instance),
		luabind::def("update_instance_timer", &lua_update_instance_timer),
		luabind::def("get_instance_id", &lua_get_instance_id),
		luabind::def("get_instance_id_by_char_id", &lua_get_instance_id_by_char_id),
		luabind::def("get_instance_timer", &lua_get_instance_timer),
		luabind::def("get_instance_timer_by_id", &lua_get_instance_timer_by_id),
		luabind::def("get_characters_in_instance", &lua_get_characters_in_instance),
		luabind::def("assign_to_instance", &lua_assign_to_instance),
		luabind::def("assign_to_instance_by_char_id", &lua_assign_to_instance_by_char_id),
		luabind::def("assign_group_to_instance", &lua_assign_group_to_instance),
		luabind::def("assign_raid_to_instance", &lua_assign_raid_to_instance),
		luabind::def("remove_from_instance", &lua_remove_from_instance),
		luabind::def("remove_from_instance_by_char_id", &lua_remove_from_instance_by_char_id),
		luabind::def("check_instance_by_char_id", (bool(*)(uint16, uint32))&lua_check_instance_by_char_id),
		luabind::def("remove_all_from_instance", &lua_remove_all_from_instance),
		luabind::def("flag_instance_by_group_leader", &lua_flag_instance_by_group_leader),
		luabind::def("flag_instance_by_raid_leader", &lua_flag_instance_by_raid_leader),
		luabind::def("fly_mode", &lua_fly_mode),
		luabind::def("faction_value", &lua_faction_value),
		luabind::def("check_title", &lua_check_title),
		luabind::def("enable_title", &lua_enable_title),
		luabind::def("remove_title", &lua_remove_title),
		luabind::def("wear_change", &lua_wear_change),
		luabind::def("voice_tell", &lua_voice_tell),
		luabind::def("send_mail", &lua_send_mail),
		luabind::def("get_qglobals", (luabind::adl::object(*)(lua_State*,Lua_NPC,Lua_Client))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::adl::object(*)(lua_State*,Lua_Client))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::adl::object(*)(lua_State*,Lua_NPC))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::adl::object(*)(lua_State*))&lua_get_qglobals),
		luabind::def("get_entity_list", &lua_get_entity_list),
		luabind::def("zone", &lua_zone),
		luabind::def("zone_group", &lua_zone_group),
		luabind::def("zone_raid", &lua_zone_raid),
		luabind::def("get_zone_id", &lua_get_zone_id),
		luabind::def("get_zone_id_by_name", &lua_get_zone_id_by_name),
		luabind::def("get_zone_long_name", &lua_get_zone_long_name),
		luabind::def("get_zone_long_name_by_name", &lua_get_zone_long_name_by_name),
		luabind::def("get_zone_long_name_by_id", &lua_get_zone_long_name_by_id),
		luabind::def("get_zone_short_name", &lua_get_zone_short_name),
		luabind::def("get_zone_short_name_by_id", &lua_get_zone_short_name_by_id),
		luabind::def("get_zone_instance_id", &lua_get_zone_instance_id),
		luabind::def("get_zone_instance_version", &lua_get_zone_instance_version),
		luabind::def("get_zone_weather", &lua_get_zone_weather),
		luabind::def("get_zone_time", &lua_get_zone_time),
		luabind::def("add_area", &lua_add_area),
		luabind::def("remove_area", &lua_remove_area),
		luabind::def("clear_areas", &lua_clear_areas),
		luabind::def("add_spawn_point", &lua_add_spawn_point),
		luabind::def("remove_spawn_point", &lua_remove_spawn_point),
		luabind::def("attack", &lua_attack),
		luabind::def("attack_npc", &lua_attack_npc),
		luabind::def("attack_npc_type", &lua_attack_npc_type),
		luabind::def("follow", (void(*)(int))&lua_follow),
		luabind::def("follow", (void(*)(int,int))&lua_follow),
		luabind::def("stop_follow", &lua_stop_follow),
		luabind::def("get_initiator", &lua_get_initiator),
		luabind::def("get_owner", &lua_get_owner),
		luabind::def("get_quest_item", &lua_get_quest_item),
		luabind::def("get_quest_spell", &lua_get_quest_spell),
		luabind::def("get_encounter", &lua_get_encounter),
		luabind::def("map_opcodes", &lua_map_opcodes),
		luabind::def("clear_opcode", &lua_clear_opcode),
		luabind::def("enable_recipe", &lua_enable_recipe),
		luabind::def("disable_recipe", &lua_disable_recipe),
		luabind::def("clear_npctype_cache", &lua_clear_npctype_cache),
		luabind::def("reloadzonestaticdata", &lua_reloadzonestaticdata),
		luabind::def("update_zone_header", &lua_update_zone_header),
		luabind::def("clock", &lua_clock),
		luabind::def("create_npc", &lua_create_npc),
		luabind::def("log", (void(*)(int, std::string))&lua_log),
		luabind::def("debug", (void(*)(std::string))&lua_debug),
		luabind::def("debug", (void(*)(std::string, int))&lua_debug),
		luabind::def("log_combat", (void(*)(std::string))&lua_log_combat),
		luabind::def("seconds_to_time", &lua_seconds_to_time),
		luabind::def("get_hex_color_code", &lua_get_hex_color_code),
		luabind::def("get_aa_exp_modifier_by_char_id", &lua_get_aa_exp_modifier_by_char_id),
		luabind::def("get_exp_modifier_by_char_id", &lua_get_exp_modifier_by_char_id),
		luabind::def("set_aa_exp_modifier_by_char_id", &lua_set_aa_exp_modifier_by_char_id),
		luabind::def("set_exp_modifier_by_char_id", &lua_set_exp_modifier_by_char_id),
		luabind::def("add_ldon_loss", &lua_add_ldon_loss),
		luabind::def("add_ldon_points", &lua_add_ldon_points),
		luabind::def("add_ldon_win", &lua_add_ldon_win),
		luabind::def("get_gender_name", &lua_get_gender_name),
		luabind::def("get_deity_name", &lua_get_deity_name),
		luabind::def("get_inventory_slot_name", &lua_get_inventory_slot_name),
		luabind::def("rename", &lua_rename),
		luabind::def("get_data_remaining", &lua_get_data_remaining),
		luabind::def("get_item_stat", &lua_get_item_stat),
		luabind::def("get_spell_stat", (int(*)(uint32,std::string))&lua_get_spell_stat),
		luabind::def("get_spell_stat", (int(*)(uint32,std::string,uint8))&lua_get_spell_stat),
		luabind::def("is_npc_spawned", &lua_is_npc_spawned),
		luabind::def("count_spawned_npcs", &lua_count_spawned_npcs),
		luabind::def("get_spell", &lua_get_spell),
		luabind::def("get_ldon_theme_name", &lua_get_ldon_theme_name),
		luabind::def("get_faction_name", &lua_get_faction_name),
		luabind::def("get_language_name", &lua_get_language_name),
		luabind::def("get_body_type_name", &lua_get_body_type_name),
		luabind::def("get_consider_level_name", &lua_get_consider_level_name),
		luabind::def("get_environmental_damage_name", &lua_get_environmental_damage_name),

		/*
			Cross Zone
		*/
		luabind::def("cross_zone_add_ldon_loss_by_char_id", &lua_cross_zone_add_ldon_loss_by_char_id),
		luabind::def("cross_zone_add_ldon_loss_by_group_id", &lua_cross_zone_add_ldon_loss_by_group_id),
		luabind::def("cross_zone_add_ldon_loss_by_raid_id", &lua_cross_zone_add_ldon_loss_by_raid_id),
		luabind::def("cross_zone_add_ldon_loss_by_guild_id", &lua_cross_zone_add_ldon_loss_by_guild_id),
		luabind::def("cross_zone_add_ldon_loss_by_expedition_id", &lua_cross_zone_add_ldon_loss_by_expedition_id),
		luabind::def("cross_zone_add_ldon_loss_by_client_name", &lua_cross_zone_add_ldon_loss_by_client_name),
		luabind::def("cross_zone_add_ldon_points_by_char_id", &lua_cross_zone_add_ldon_points_by_char_id),
		luabind::def("cross_zone_add_ldon_points_by_group_id", &lua_cross_zone_add_ldon_points_by_group_id),
		luabind::def("cross_zone_add_ldon_points_by_raid_id", &lua_cross_zone_add_ldon_points_by_raid_id),
		luabind::def("cross_zone_add_ldon_points_by_guild_id", &lua_cross_zone_add_ldon_points_by_guild_id),
		luabind::def("cross_zone_add_ldon_points_by_expedition_id", &lua_cross_zone_add_ldon_points_by_expedition_id),
		luabind::def("cross_zone_add_ldon_points_by_client_name", &lua_cross_zone_add_ldon_points_by_client_name),
		luabind::def("cross_zone_add_ldon_win_by_char_id", &lua_cross_zone_add_ldon_win_by_char_id),
		luabind::def("cross_zone_add_ldon_win_by_group_id", &lua_cross_zone_add_ldon_win_by_group_id),
		luabind::def("cross_zone_add_ldon_win_by_raid_id", &lua_cross_zone_add_ldon_win_by_raid_id),
		luabind::def("cross_zone_add_ldon_win_by_guild_id", &lua_cross_zone_add_ldon_win_by_guild_id),
		luabind::def("cross_zone_add_ldon_win_by_expedition_id", &lua_cross_zone_add_ldon_win_by_expedition_id),
		luabind::def("cross_zone_add_ldon_win_by_client_name", &lua_cross_zone_add_ldon_win_by_client_name),
		luabind::def("cross_zone_assign_task_by_char_id", (void(*)(int,uint32))&lua_cross_zone_assign_task_by_char_id),
		luabind::def("cross_zone_assign_task_by_char_id", (void(*)(int,uint32,bool))&lua_cross_zone_assign_task_by_char_id),
		luabind::def("cross_zone_assign_task_by_group_id", (void(*)(int,uint32))&lua_cross_zone_assign_task_by_group_id),
		luabind::def("cross_zone_assign_task_by_group_id", (void(*)(int,uint32,bool))&lua_cross_zone_assign_task_by_group_id),
		luabind::def("cross_zone_assign_task_by_raid_id", (void(*)(int,uint32))&lua_cross_zone_assign_task_by_raid_id),
		luabind::def("cross_zone_assign_task_by_raid_id", (void(*)(int,uint32,bool))&lua_cross_zone_assign_task_by_raid_id),
		luabind::def("cross_zone_assign_task_by_guild_id", (void(*)(int,uint32))&lua_cross_zone_assign_task_by_guild_id),
		luabind::def("cross_zone_assign_task_by_guild_id", (void(*)(int,uint32,bool))&lua_cross_zone_assign_task_by_guild_id),
		luabind::def("cross_zone_assign_task_by_expedition_id", (void(*)(uint32,uint32))&lua_cross_zone_assign_task_by_expedition_id),
		luabind::def("cross_zone_assign_task_by_expedition_id", (void(*)(uint32,uint32,bool))&lua_cross_zone_assign_task_by_expedition_id),
		luabind::def("cross_zone_assign_task_by_client_name", (void(*)(const char*,uint32))&lua_cross_zone_assign_task_by_client_name),
		luabind::def("cross_zone_assign_task_by_client_name", (void(*)(const char*,uint32,bool))&lua_cross_zone_assign_task_by_client_name),
		luabind::def("cross_zone_cast_spell_by_char_id", &lua_cross_zone_cast_spell_by_char_id),
		luabind::def("cross_zone_cast_spell_by_group_id", &lua_cross_zone_cast_spell_by_group_id),
		luabind::def("cross_zone_cast_spell_by_raid_id", &lua_cross_zone_cast_spell_by_raid_id),
		luabind::def("cross_zone_cast_spell_by_guild_id", &lua_cross_zone_cast_spell_by_guild_id),
		luabind::def("cross_zone_cast_spell_by_expedition_id", &lua_cross_zone_cast_spell_by_expedition_id),
		luabind::def("cross_zone_cast_spell_by_client_name", &lua_cross_zone_cast_spell_by_client_name),
		luabind::def("cross_zone_dialogue_window_by_char_id", &lua_cross_zone_dialogue_window_by_char_id),
		luabind::def("cross_zone_dialogue_window_by_group_id", &lua_cross_zone_dialogue_window_by_group_id),
		luabind::def("cross_zone_dialogue_window_by_raid_id", &lua_cross_zone_dialogue_window_by_raid_id),
		luabind::def("cross_zone_dialogue_window_by_guild_id", &lua_cross_zone_dialogue_window_by_guild_id),
		luabind::def("cross_zone_dialogue_window_by_expedition_id", &lua_cross_zone_dialogue_window_by_expedition_id),
		luabind::def("cross_zone_dialogue_window_by_client_name", &lua_cross_zone_dialogue_window_by_client_name),
		luabind::def("cross_zone_disable_task_by_char_id", &lua_cross_zone_disable_task_by_char_id),
		luabind::def("cross_zone_disable_task_by_group_id", &lua_cross_zone_disable_task_by_group_id),
		luabind::def("cross_zone_disable_task_by_raid_id", &lua_cross_zone_disable_task_by_raid_id),
		luabind::def("cross_zone_disable_task_by_guild_id", &lua_cross_zone_disable_task_by_guild_id),
		luabind::def("cross_zone_disable_task_by_expedition_id", &lua_cross_zone_disable_task_by_expedition_id),
		luabind::def("cross_zone_disable_task_by_client_name", &lua_cross_zone_disable_task_by_client_name),
		luabind::def("cross_zone_enable_task_by_char_id", &lua_cross_zone_enable_task_by_char_id),
		luabind::def("cross_zone_enable_task_by_group_id", &lua_cross_zone_enable_task_by_group_id),
		luabind::def("cross_zone_enable_task_by_raid_id", &lua_cross_zone_enable_task_by_raid_id),
		luabind::def("cross_zone_enable_task_by_guild_id", &lua_cross_zone_enable_task_by_guild_id),
		luabind::def("cross_zone_enable_task_by_expedition_id", &lua_cross_zone_enable_task_by_expedition_id),
		luabind::def("cross_zone_enable_task_by_client_name", &lua_cross_zone_enable_task_by_client_name),
		luabind::def("cross_zone_fail_task_by_char_id", &lua_cross_zone_fail_task_by_char_id),
		luabind::def("cross_zone_fail_task_by_group_id", &lua_cross_zone_fail_task_by_group_id),
		luabind::def("cross_zone_fail_task_by_raid_id", &lua_cross_zone_fail_task_by_raid_id),
		luabind::def("cross_zone_fail_task_by_guild_id", &lua_cross_zone_fail_task_by_guild_id),
		luabind::def("cross_zone_fail_task_by_expedition_id", &lua_cross_zone_fail_task_by_expedition_id),
		luabind::def("cross_zone_fail_task_by_client_name", &lua_cross_zone_fail_task_by_client_name),
		luabind::def("cross_zone_marquee_by_char_id", &lua_cross_zone_marquee_by_char_id),
		luabind::def("cross_zone_marquee_by_group_id", &lua_cross_zone_marquee_by_group_id),
		luabind::def("cross_zone_marquee_by_raid_id", &lua_cross_zone_marquee_by_raid_id),
		luabind::def("cross_zone_marquee_by_guild_id", &lua_cross_zone_marquee_by_guild_id),
		luabind::def("cross_zone_marquee_by_expedition_id", &lua_cross_zone_marquee_by_expedition_id),
		luabind::def("cross_zone_marquee_by_client_name", &lua_cross_zone_marquee_by_client_name),
		luabind::def("cross_zone_message_player_by_char_id", &lua_cross_zone_message_player_by_char_id),
		luabind::def("cross_zone_message_player_by_group_id", &lua_cross_zone_message_player_by_group_id),
		luabind::def("cross_zone_message_player_by_raid_id", &lua_cross_zone_message_player_by_raid_id),
		luabind::def("cross_zone_message_player_by_guild_id", &lua_cross_zone_message_player_by_guild_id),
		luabind::def("cross_zone_message_player_by_expedition_id", &lua_cross_zone_message_player_by_expedition_id),
		luabind::def("cross_zone_message_player_by_name", &lua_cross_zone_message_player_by_name),
		luabind::def("cross_zone_move_player_by_char_id", &lua_cross_zone_move_player_by_char_id),
		luabind::def("cross_zone_move_player_by_group_id", &lua_cross_zone_move_player_by_group_id),
		luabind::def("cross_zone_move_player_by_raid_id", &lua_cross_zone_move_player_by_raid_id),
		luabind::def("cross_zone_move_player_by_guild_id", &lua_cross_zone_move_player_by_guild_id),
		luabind::def("cross_zone_move_player_by_expedition_id", &lua_cross_zone_move_player_by_expedition_id),
		luabind::def("cross_zone_move_player_by_client_name", &lua_cross_zone_move_player_by_client_name),
		luabind::def("cross_zone_move_instance_by_char_id", &lua_cross_zone_move_instance_by_char_id),
		luabind::def("cross_zone_move_instance_by_group_id", &lua_cross_zone_move_instance_by_group_id),
		luabind::def("cross_zone_move_instance_by_raid_id", &lua_cross_zone_move_instance_by_raid_id),
		luabind::def("cross_zone_move_instance_by_guild_id", &lua_cross_zone_move_instance_by_guild_id),
		luabind::def("cross_zone_move_instance_by_expedition_id", &lua_cross_zone_move_instance_by_expedition_id),
		luabind::def("cross_zone_move_instance_by_client_name", &lua_cross_zone_move_instance_by_client_name),
		luabind::def("cross_zone_remove_ldon_loss_by_char_id", &lua_cross_zone_remove_ldon_loss_by_char_id),
		luabind::def("cross_zone_remove_ldon_loss_by_group_id", &lua_cross_zone_remove_ldon_loss_by_group_id),
		luabind::def("cross_zone_remove_ldon_loss_by_raid_id", &lua_cross_zone_remove_ldon_loss_by_raid_id),
		luabind::def("cross_zone_remove_ldon_loss_by_guild_id", &lua_cross_zone_remove_ldon_loss_by_guild_id),
		luabind::def("cross_zone_remove_ldon_loss_by_expedition_id", &lua_cross_zone_remove_ldon_loss_by_expedition_id),
		luabind::def("cross_zone_remove_ldon_loss_by_client_name", &lua_cross_zone_remove_ldon_loss_by_client_name),
		luabind::def("cross_zone_remove_ldon_win_by_char_id", &lua_cross_zone_remove_ldon_win_by_char_id),
		luabind::def("cross_zone_remove_ldon_win_by_group_id", &lua_cross_zone_remove_ldon_win_by_group_id),
		luabind::def("cross_zone_remove_ldon_win_by_raid_id", &lua_cross_zone_remove_ldon_win_by_raid_id),
		luabind::def("cross_zone_remove_ldon_win_by_guild_id", &lua_cross_zone_remove_ldon_win_by_guild_id),
		luabind::def("cross_zone_remove_ldon_win_by_expedition_id", &lua_cross_zone_remove_ldon_win_by_expedition_id),
		luabind::def("cross_zone_remove_ldon_win_by_client_name", &lua_cross_zone_remove_ldon_win_by_client_name),
		luabind::def("cross_zone_remove_spell_by_char_id", &lua_cross_zone_remove_spell_by_char_id),
		luabind::def("cross_zone_remove_spell_by_group_id", &lua_cross_zone_remove_spell_by_group_id),
		luabind::def("cross_zone_remove_spell_by_raid_id", &lua_cross_zone_remove_spell_by_raid_id),
		luabind::def("cross_zone_remove_spell_by_guild_id", &lua_cross_zone_remove_spell_by_guild_id),
		luabind::def("cross_zone_remove_spell_by_expedition_id", &lua_cross_zone_remove_spell_by_expedition_id),
		luabind::def("cross_zone_remove_spell_by_client_name", &lua_cross_zone_remove_spell_by_client_name),
		luabind::def("cross_zone_remove_task_by_char_id", &lua_cross_zone_remove_task_by_char_id),
		luabind::def("cross_zone_remove_task_by_group_id", &lua_cross_zone_remove_task_by_group_id),
		luabind::def("cross_zone_remove_task_by_raid_id", &lua_cross_zone_remove_task_by_raid_id),
		luabind::def("cross_zone_remove_task_by_guild_id", &lua_cross_zone_remove_task_by_guild_id),
		luabind::def("cross_zone_remove_task_by_expedition_id", &lua_cross_zone_remove_task_by_expedition_id),
		luabind::def("cross_zone_remove_task_by_client_name", &lua_cross_zone_remove_task_by_client_name),
		luabind::def("cross_zone_reset_activity_by_char_id", &lua_cross_zone_reset_activity_by_char_id),
		luabind::def("cross_zone_reset_activity_by_group_id", &lua_cross_zone_reset_activity_by_group_id),
		luabind::def("cross_zone_reset_activity_by_raid_id", &lua_cross_zone_reset_activity_by_raid_id),
		luabind::def("cross_zone_reset_activity_by_guild_id", &lua_cross_zone_reset_activity_by_guild_id),
		luabind::def("cross_zone_reset_activity_by_expedition_id", &lua_cross_zone_reset_activity_by_expedition_id),
		luabind::def("cross_zone_reset_activity_by_client_name", &lua_cross_zone_reset_activity_by_client_name),
		luabind::def("cross_zone_set_entity_variable_by_client_name", &lua_cross_zone_set_entity_variable_by_client_name),
		luabind::def("cross_zone_set_entity_variable_by_group_id", &lua_cross_zone_set_entity_variable_by_group_id),
		luabind::def("cross_zone_set_entity_variable_by_raid_id", &lua_cross_zone_set_entity_variable_by_raid_id),
		luabind::def("cross_zone_set_entity_variable_by_guild_id", &lua_cross_zone_set_entity_variable_by_guild_id),
		luabind::def("cross_zone_set_entity_variable_by_expedition_id", &lua_cross_zone_set_entity_variable_by_expedition_id),
		luabind::def("cross_zone_set_entity_variable_by_client_name", &lua_cross_zone_set_entity_variable_by_client_name),
		luabind::def("cross_zone_signal_client_by_char_id", &lua_cross_zone_signal_client_by_char_id),
		luabind::def("cross_zone_signal_client_by_group_id", &lua_cross_zone_signal_client_by_group_id),
		luabind::def("cross_zone_signal_client_by_raid_id", &lua_cross_zone_signal_client_by_raid_id),
		luabind::def("cross_zone_signal_client_by_guild_id", &lua_cross_zone_signal_client_by_guild_id),
		luabind::def("cross_zone_signal_client_by_expedition_id", &lua_cross_zone_signal_client_by_expedition_id),
		luabind::def("cross_zone_signal_client_by_name", &lua_cross_zone_signal_client_by_name),
		luabind::def("cross_zone_signal_npc_by_npctype_id", &lua_cross_zone_signal_npc_by_npctype_id),
		luabind::def("cross_zone_update_activity_by_char_id", (void(*)(int,uint32,int))&lua_cross_zone_update_activity_by_char_id),
		luabind::def("cross_zone_update_activity_by_char_id", (void(*)(int,uint32,int,int))&lua_cross_zone_update_activity_by_char_id),
		luabind::def("cross_zone_update_activity_by_group_id", (void(*)(int,uint32,int))&lua_cross_zone_update_activity_by_group_id),
		luabind::def("cross_zone_update_activity_by_group_id", (void(*)(int,uint32,int,int))&lua_cross_zone_update_activity_by_group_id),
		luabind::def("cross_zone_update_activity_by_raid_id", (void(*)(int,uint32,int))&lua_cross_zone_update_activity_by_raid_id),
		luabind::def("cross_zone_update_activity_by_raid_id", (void(*)(int,uint32,int,int))&lua_cross_zone_update_activity_by_raid_id),
		luabind::def("cross_zone_update_activity_by_guild_id", (void(*)(int,uint32,int))&lua_cross_zone_update_activity_by_guild_id),
		luabind::def("cross_zone_update_activity_by_guild_id", (void(*)(int,uint32,int,int))&lua_cross_zone_update_activity_by_guild_id),
		luabind::def("cross_zone_update_activity_by_expedition_id", (void(*)(uint32,uint32,int))&lua_cross_zone_update_activity_by_expedition_id),
		luabind::def("cross_zone_update_activity_by_expedition_id", (void(*)(uint32,uint32,int,int))&lua_cross_zone_update_activity_by_expedition_id),
		luabind::def("cross_zone_update_activity_by_client_name", (void(*)(const char*,uint32,int))&lua_cross_zone_update_activity_by_client_name),
		luabind::def("cross_zone_update_activity_by_client_name", (void(*)(const char*,uint32,int,int))&lua_cross_zone_update_activity_by_client_name),

		/*
			World Wide
		*/
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32))&lua_world_wide_add_ldon_loss),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32,uint8))&lua_world_wide_add_ldon_loss),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32,uint8,uint8))&lua_world_wide_add_ldon_loss),
		luabind::def("world_wide_add_ldon_points", (void(*)(uint32,int))&lua_world_wide_add_ldon_points),
		luabind::def("world_wide_add_ldon_points", (void(*)(uint32,int,uint8))&lua_world_wide_add_ldon_points),
		luabind::def("world_wide_add_ldon_points", (void(*)(uint32,int,uint8,uint8))&lua_world_wide_add_ldon_points),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32))&lua_world_wide_add_ldon_win),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32,uint8))&lua_world_wide_add_ldon_win),
		luabind::def("world_wide_add_ldon_loss", (void(*)(uint32,uint8,uint8))&lua_world_wide_add_ldon_win),
		luabind::def("world_wide_assign_task", (void(*)(uint32))&lua_world_wide_assign_task),
		luabind::def("world_wide_assign_task", (void(*)(uint32,bool))&lua_world_wide_assign_task),
		luabind::def("world_wide_assign_task", (void(*)(uint32,bool,uint8))&lua_world_wide_assign_task),
		luabind::def("world_wide_assign_task", (void(*)(uint32,bool,uint8,uint8))&lua_world_wide_assign_task),
		luabind::def("world_wide_cast_spell", (void(*)(uint32))&lua_world_wide_cast_spell),
		luabind::def("world_wide_cast_spell", (void(*)(uint32,uint8))&lua_world_wide_cast_spell),
		luabind::def("world_wide_cast_spell", (void(*)(uint32,uint8,uint8))&lua_world_wide_cast_spell),
		luabind::def("world_wide_dialogue_window", (void(*)(const char*))&lua_world_wide_dialogue_window),
		luabind::def("world_wide_dialogue_window", (void(*)(const char*,uint8))&lua_world_wide_dialogue_window),
		luabind::def("world_wide_dialogue_window", (void(*)(const char*,uint8,uint8))&lua_world_wide_dialogue_window),
		luabind::def("world_wide_disable_task", (void(*)(uint32))&lua_world_wide_disable_task),
		luabind::def("world_wide_disable_task", (void(*)(uint32,uint8))&lua_world_wide_disable_task),
		luabind::def("world_wide_disable_task", (void(*)(uint32,uint8,uint8))&lua_world_wide_disable_task),
		luabind::def("world_wide_enable_task", (void(*)(uint32))&lua_world_wide_enable_task),
		luabind::def("world_wide_enable_task", (void(*)(uint32,uint8))&lua_world_wide_enable_task),
		luabind::def("world_wide_enable_task", (void(*)(uint32,uint8,uint8))&lua_world_wide_enable_task),
		luabind::def("world_wide_fail_task", (void(*)(uint32))&lua_world_wide_fail_task),
		luabind::def("world_wide_fail_task", (void(*)(uint32,uint8))&lua_world_wide_fail_task),
		luabind::def("world_wide_fail_task", (void(*)(uint32,uint8,uint8))&lua_world_wide_fail_task),
		luabind::def("world_wide_marquee", (void(*)(uint32,uint32,uint32,uint32,uint32,const char*))&lua_world_wide_marquee),
		luabind::def("world_wide_marquee", (void(*)(uint32,uint32,uint32,uint32,uint32,const char*,uint8))&lua_world_wide_marquee),
		luabind::def("world_wide_marquee", (void(*)(uint32,uint32,uint32,uint32,uint32,const char*,uint8,uint8))&lua_world_wide_marquee),
		luabind::def("world_wide_message", (void(*)(uint32,const char*))&lua_world_wide_message),
		luabind::def("world_wide_message", (void(*)(uint32,const char*,uint8))&lua_world_wide_message),
		luabind::def("world_wide_message", (void(*)(uint32,const char*,uint8,uint8))&lua_world_wide_message),
		luabind::def("world_wide_move", (void(*)(const char*))&lua_world_wide_move),
		luabind::def("world_wide_move", (void(*)(const char*,uint8))&lua_world_wide_move),
		luabind::def("world_wide_move", (void(*)(const char*,uint8,uint8))&lua_world_wide_move),
		luabind::def("world_wide_move_instance", (void(*)(uint16))&lua_world_wide_move_instance),
		luabind::def("world_wide_move_instance", (void(*)(uint16,uint8))&lua_world_wide_move_instance),
		luabind::def("world_wide_move_instance", (void(*)(uint16,uint8,uint8))&lua_world_wide_move_instance),
		luabind::def("world_wide_remove_spell", (void(*)(uint32))&lua_world_wide_remove_spell),
		luabind::def("world_wide_remove_spell", (void(*)(uint32,uint8))&lua_world_wide_remove_spell),
		luabind::def("world_wide_remove_spell", (void(*)(uint32,uint8,uint8))&lua_world_wide_remove_spell),
		luabind::def("world_wide_remove_task", (void(*)(uint32))&lua_world_wide_remove_task),
		luabind::def("world_wide_remove_task", (void(*)(uint32,uint8))&lua_world_wide_remove_task),
		luabind::def("world_wide_remove_task", (void(*)(uint32,uint8,uint8))&lua_world_wide_remove_task),
		luabind::def("world_wide_reset_activity", (void(*)(uint32,int))&lua_world_wide_reset_activity),
		luabind::def("world_wide_reset_activity", (void(*)(uint32,int,uint8))&lua_world_wide_reset_activity),
		luabind::def("world_wide_reset_activity", (void(*)(uint32,int,uint8,uint8))&lua_world_wide_reset_activity),
		luabind::def("world_wide_set_entity_variable_client", (void(*)(const char*,const char*))&lua_world_wide_set_entity_variable_client),
		luabind::def("world_wide_set_entity_variable_client", (void(*)(const char*,const char*,uint8))&lua_world_wide_set_entity_variable_client),
		luabind::def("world_wide_set_entity_variable_client", (void(*)(const char*,const char*,uint8,uint8))&lua_world_wide_set_entity_variable_client),
		luabind::def("world_wide_set_entity_variable_npc", &lua_world_wide_set_entity_variable_npc),
		luabind::def("world_wide_signal_client", (void(*)(uint32))&lua_world_wide_signal_client),
		luabind::def("world_wide_signal_client", (void(*)(uint32,uint8))&lua_world_wide_signal_client),
		luabind::def("world_wide_signal_client", (void(*)(uint32,uint8,uint8))&lua_world_wide_signal_client),
		luabind::def("world_wide_signal_npc", &lua_world_wide_signal_npc),
		luabind::def("world_wide_update_activity", (void(*)(uint32,int))&lua_world_wide_update_activity),
		luabind::def("world_wide_update_activity", (void(*)(uint32,int,int))&lua_world_wide_update_activity),
		luabind::def("world_wide_update_activity", (void(*)(uint32,int,int,uint8))&lua_world_wide_update_activity),
		luabind::def("world_wide_update_activity", (void(*)(uint32,int,int,uint8,uint8))&lua_world_wide_update_activity),

		/**
		 * Expansions
		 */
		luabind::def("is_classic_enabled", &lua_is_classic_enabled),
		luabind::def("is_the_ruins_of_kunark_enabled", &lua_is_the_ruins_of_kunark_enabled),
		luabind::def("is_the_scars_of_velious_enabled", &lua_is_the_scars_of_velious_enabled),
		luabind::def("is_the_shadows_of_luclin_enabled", &lua_is_the_shadows_of_luclin_enabled),
		luabind::def("is_the_planes_of_power_enabled", &lua_is_the_planes_of_power_enabled),
		luabind::def("is_the_legacy_of_ykesha_enabled", &lua_is_the_legacy_of_ykesha_enabled),
		luabind::def("is_lost_dungeons_of_norrath_enabled", &lua_is_lost_dungeons_of_norrath_enabled),
		luabind::def("is_gates_of_discord_enabled", &lua_is_gates_of_discord_enabled),
		luabind::def("is_omens_of_war_enabled", &lua_is_omens_of_war_enabled),
		luabind::def("is_dragons_of_norrath_enabled", &lua_is_dragons_of_norrath_enabled),
		luabind::def("is_depths_of_darkhollow_enabled", &lua_is_depths_of_darkhollow_enabled),
		luabind::def("is_prophecy_of_ro_enabled", &lua_is_prophecy_of_ro_enabled),
		luabind::def("is_the_serpents_spine_enabled", &lua_is_the_serpents_spine_enabled),
		luabind::def("is_the_buried_sea_enabled", &lua_is_the_buried_sea_enabled),
		luabind::def("is_secrets_of_faydwer_enabled", &lua_is_secrets_of_faydwer_enabled),
		luabind::def("is_seeds_of_destruction_enabled", &lua_is_seeds_of_destruction_enabled),
		luabind::def("is_underfoot_enabled", &lua_is_underfoot_enabled),
		luabind::def("is_house_of_thule_enabled", &lua_is_house_of_thule_enabled),
		luabind::def("is_veil_of_alaris_enabled", &lua_is_veil_of_alaris_enabled),
		luabind::def("is_rain_of_fear_enabled", &lua_is_rain_of_fear_enabled),
		luabind::def("is_call_of_the_forsaken_enabled", &lua_is_call_of_the_forsaken_enabled),
		luabind::def("is_the_darkend_sea_enabled", &lua_is_the_darkend_sea_enabled),
		luabind::def("is_the_broken_mirror_enabled", &lua_is_the_broken_mirror_enabled),
		luabind::def("is_empires_of_kunark_enabled", &lua_is_empires_of_kunark_enabled),
		luabind::def("is_ring_of_scale_enabled", &lua_is_ring_of_scale_enabled),
		luabind::def("is_the_burning_lands_enabled", &lua_is_the_burning_lands_enabled),
		luabind::def("is_torment_of_velious_enabled", &lua_is_torment_of_velious_enabled),
		luabind::def("is_current_expansion_classic", &lua_is_current_expansion_classic),
		luabind::def("is_current_expansion_the_ruins_of_kunark", &lua_is_current_expansion_the_ruins_of_kunark),
		luabind::def("is_current_expansion_the_scars_of_velious", &lua_is_current_expansion_the_scars_of_velious),
		luabind::def("is_current_expansion_the_shadows_of_luclin", &lua_is_current_expansion_the_shadows_of_luclin),
		luabind::def("is_current_expansion_the_planes_of_power", &lua_is_current_expansion_the_planes_of_power),
		luabind::def("is_current_expansion_the_legacy_of_ykesha", &lua_is_current_expansion_the_legacy_of_ykesha),
		luabind::def("is_current_expansion_lost_dungeons_of_norrath", &lua_is_current_expansion_lost_dungeons_of_norrath),
		luabind::def("is_current_expansion_gates_of_discord", &lua_is_current_expansion_gates_of_discord),
		luabind::def("is_current_expansion_omens_of_war", &lua_is_current_expansion_omens_of_war),
		luabind::def("is_current_expansion_dragons_of_norrath", &lua_is_current_expansion_dragons_of_norrath),
		luabind::def("is_current_expansion_depths_of_darkhollow", &lua_is_current_expansion_depths_of_darkhollow),
		luabind::def("is_current_expansion_prophecy_of_ro", &lua_is_current_expansion_prophecy_of_ro),
		luabind::def("is_current_expansion_the_serpents_spine", &lua_is_current_expansion_the_serpents_spine),
		luabind::def("is_current_expansion_the_buried_sea", &lua_is_current_expansion_the_buried_sea),
		luabind::def("is_current_expansion_secrets_of_faydwer", &lua_is_current_expansion_secrets_of_faydwer),
		luabind::def("is_current_expansion_seeds_of_destruction", &lua_is_current_expansion_seeds_of_destruction),
		luabind::def("is_current_expansion_underfoot", &lua_is_current_expansion_underfoot),
		luabind::def("is_current_expansion_house_of_thule", &lua_is_current_expansion_house_of_thule),
		luabind::def("is_current_expansion_veil_of_alaris", &lua_is_current_expansion_veil_of_alaris),
		luabind::def("is_current_expansion_rain_of_fear", &lua_is_current_expansion_rain_of_fear),
		luabind::def("is_current_expansion_call_of_the_forsaken", &lua_is_current_expansion_call_of_the_forsaken),
		luabind::def("is_current_expansion_the_darkend_sea", &lua_is_current_expansion_the_darkend_sea),
		luabind::def("is_current_expansion_the_broken_mirror", &lua_is_current_expansion_the_broken_mirror),
		luabind::def("is_current_expansion_empires_of_kunark", &lua_is_current_expansion_empires_of_kunark),
		luabind::def("is_current_expansion_ring_of_scale", &lua_is_current_expansion_ring_of_scale),
		luabind::def("is_current_expansion_the_burning_lands", &lua_is_current_expansion_the_burning_lands),
		luabind::def("is_current_expansion_torment_of_velious", &lua_is_current_expansion_torment_of_velious),

		/**
		 * Content flags
		 */
		luabind::def("is_content_flag_enabled", (bool(*)(std::string))&lua_is_content_flag_enabled),
		luabind::def("set_content_flag", (void(*)(std::string, bool))&lua_set_content_flag),

		luabind::def("get_expedition", &lua_get_expedition),
		luabind::def("get_expedition_by_char_id", &lua_get_expedition_by_char_id),
		luabind::def("get_expedition_by_dz_id", &lua_get_expedition_by_dz_id),
		luabind::def("get_expedition_by_zone_instance", &lua_get_expedition_by_zone_instance),
		luabind::def("get_expedition_lockout_by_char_id", &lua_get_expedition_lockout_by_char_id),
		luabind::def("get_expedition_lockouts_by_char_id", (luabind::object(*)(lua_State*, uint32))&lua_get_expedition_lockouts_by_char_id),
		luabind::def("get_expedition_lockouts_by_char_id", (luabind::object(*)(lua_State*, uint32, std::string))&lua_get_expedition_lockouts_by_char_id),
		luabind::def("add_expedition_lockout_all_clients", (void(*)(std::string, std::string, uint32))&lua_add_expedition_lockout_all_clients),
		luabind::def("add_expedition_lockout_all_clients", (void(*)(std::string, std::string, uint32, std::string))&lua_add_expedition_lockout_all_clients),
		luabind::def("add_expedition_lockout_by_char_id", (void(*)(uint32, std::string, std::string, uint32))&lua_add_expedition_lockout_by_char_id),
		luabind::def("add_expedition_lockout_by_char_id", (void(*)(uint32, std::string, std::string, uint32, std::string))&lua_add_expedition_lockout_by_char_id),
		luabind::def("remove_expedition_lockout_by_char_id", &lua_remove_expedition_lockout_by_char_id),
		luabind::def("remove_all_expedition_lockouts_by_char_id", (void(*)(uint32))&lua_remove_all_expedition_lockouts_by_char_id),
		luabind::def("remove_all_expedition_lockouts_by_char_id", (void(*)(uint32, std::string))&lua_remove_all_expedition_lockouts_by_char_id)
	];
}

luabind::scope lua_register_random() {
	return luabind::namespace_("Random")
		[
			luabind::def("Int", &random_int),
			luabind::def("Real", &random_real),
			luabind::def("Roll", &random_roll_int),
			luabind::def("RollReal", &random_roll_real),
			luabind::def("Roll0", &random_roll0)
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
			luabind::value("combat", static_cast<int>(EVENT_COMBAT)),
			luabind::value("slay", static_cast<int>(EVENT_SLAY)),
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
			luabind::value("killed_merit", static_cast<int>(EVENT_KILLED_MERIT)),
			luabind::value("cast_on", static_cast<int>(EVENT_CAST_ON)),
			luabind::value("task_accepted", static_cast<int>(EVENT_TASK_ACCEPTED)),
			luabind::value("task_stage_complete", static_cast<int>(EVENT_TASK_STAGE_COMPLETE)),
			luabind::value("environmental_damage", static_cast<int>(EVENT_ENVIRONMENTAL_DAMAGE)),
			luabind::value("task_update", static_cast<int>(EVENT_TASK_UPDATE)),
			luabind::value("task_complete", static_cast<int>(EVENT_TASK_COMPLETE)),
			luabind::value("task_fail", static_cast<int>(EVENT_TASK_FAIL)),
			luabind::value("aggro_say", static_cast<int>(EVENT_AGGRO_SAY)),
			luabind::value("player_pickup", static_cast<int>(EVENT_PLAYER_PICKUP)),
			luabind::value("popup_response", static_cast<int>(EVENT_POPUP_RESPONSE)),
			luabind::value("proximity_say", static_cast<int>(EVENT_PROXIMITY_SAY)),
			luabind::value("cast", static_cast<int>(EVENT_CAST)),
			luabind::value("cast_begin", static_cast<int>(EVENT_CAST_BEGIN)),
			luabind::value("scale_calc", static_cast<int>(EVENT_SCALE_CALC)),
			luabind::value("item_enter_zone", static_cast<int>(EVENT_ITEM_ENTER_ZONE)),
			luabind::value("target_change", static_cast<int>(EVENT_TARGET_CHANGE)),
			luabind::value("hate_list", static_cast<int>(EVENT_HATE_LIST)),
			luabind::value("spell_effect", static_cast<int>(EVENT_SPELL_EFFECT_CLIENT)),
			luabind::value("spell_buff_tic", static_cast<int>(EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT)),
			luabind::value("spell_fade", static_cast<int>(EVENT_SPELL_FADE)),
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
			luabind::value("encounter_unload", static_cast<int>(EVENT_ENCOUNTER_UNLOAD)),
			luabind::value("command", static_cast<int>(EVENT_COMMAND)),
			luabind::value("drop_item", static_cast<int>(EVENT_DROP_ITEM)),
			luabind::value("destroy_item", static_cast<int>(EVENT_DESTROY_ITEM)),
			luabind::value("feign_death", static_cast<int>(EVENT_FEIGN_DEATH)),
			luabind::value("weapon_proc", static_cast<int>(EVENT_WEAPON_PROC)),
			luabind::value("equip_item", static_cast<int>(EVENT_EQUIP_ITEM)),
			luabind::value("unequip_item", static_cast<int>(EVENT_UNEQUIP_ITEM)),
			luabind::value("augment_item", static_cast<int>(EVENT_AUGMENT_ITEM)),
			luabind::value("unaugment_item", static_cast<int>(EVENT_UNAUGMENT_ITEM)),
			luabind::value("augment_insert", static_cast<int>(EVENT_AUGMENT_INSERT)),
			luabind::value("augment_remove", static_cast<int>(EVENT_AUGMENT_REMOVE)),
			luabind::value("enter_area", static_cast<int>(EVENT_ENTER_AREA)),
			luabind::value("leave_area", static_cast<int>(EVENT_LEAVE_AREA)),
			luabind::value("death_complete", static_cast<int>(EVENT_DEATH_COMPLETE)),
			luabind::value("unhandled_opcode", static_cast<int>(EVENT_UNHANDLED_OPCODE)),
			luabind::value("tick", static_cast<int>(EVENT_TICK)),
			luabind::value("spawn_zone", static_cast<int>(EVENT_SPAWN_ZONE)),
			luabind::value("death_zone", static_cast<int>(EVENT_DEATH_ZONE)),
			luabind::value("use_skill", static_cast<int>(EVENT_USE_SKILL)),
			luabind::value("warp", static_cast<int>(EVENT_WARP)),
			luabind::value("test_buff", static_cast<int>(EVENT_TEST_BUFF)),
			luabind::value("consider", static_cast<int>(EVENT_CONSIDER)),
			luabind::value("consider_corpse", static_cast<int>(EVENT_CONSIDER_CORPSE)),
			luabind::value("loot_zone", static_cast<int>(EVENT_LOOT_ZONE)),
			luabind::value("equip_item_client", static_cast<int>(EVENT_EQUIP_ITEM_CLIENT)),
			luabind::value("unequip_item_client", static_cast<int>(EVENT_UNEQUIP_ITEM_CLIENT))
		];
}

luabind::scope lua_register_faction() {
	return luabind::class_<Factions>("Faction")
		.enum_("constants")
		[
			luabind::value("Ally", static_cast<int>(FACTION_ALLY)),
			luabind::value("Warmly", static_cast<int>(FACTION_WARMLY)),
			luabind::value("Kindly", static_cast<int>(FACTION_KINDLY)),
			luabind::value("Amiable", static_cast<int>(FACTION_AMIABLY)),
			luabind::value("Indifferent", static_cast<int>(FACTION_INDIFFERENTLY)),
			luabind::value("Apprehensive", static_cast<int>(FACTION_APPREHENSIVELY)),
			luabind::value("Dubious", static_cast<int>(FACTION_DUBIOUSLY)),
			luabind::value("Threatenly", static_cast<int>(FACTION_THREATENINGLY)),
			luabind::value("Scowls", static_cast<int>(FACTION_SCOWLS))
		];
}

luabind::scope lua_register_slot() {
	return luabind::class_<Slots>("Slot")
		.enum_("constants")
		[
			luabind::value("Charm", static_cast<int>(EQ::invslot::slotCharm)),
			luabind::value("Ear1", static_cast<int>(EQ::invslot::slotEar1)),
			luabind::value("Head", static_cast<int>(EQ::invslot::slotHead)),
			luabind::value("Face", static_cast<int>(EQ::invslot::slotFace)),
			luabind::value("Ear2", static_cast<int>(EQ::invslot::slotEar2)),
			luabind::value("Neck", static_cast<int>(EQ::invslot::slotNeck)),
			luabind::value("Shoulders", static_cast<int>(EQ::invslot::slotShoulders)),
			luabind::value("Arms", static_cast<int>(EQ::invslot::slotArms)),
			luabind::value("Back", static_cast<int>(EQ::invslot::slotBack)),
			luabind::value("Wrist1", static_cast<int>(EQ::invslot::slotWrist1)),
			luabind::value("Wrist2", static_cast<int>(EQ::invslot::slotWrist2)),
			luabind::value("Range", static_cast<int>(EQ::invslot::slotRange)),
			luabind::value("Hands", static_cast<int>(EQ::invslot::slotHands)),
			luabind::value("Primary", static_cast<int>(EQ::invslot::slotPrimary)),
			luabind::value("Secondary", static_cast<int>(EQ::invslot::slotSecondary)),
			luabind::value("Finger1", static_cast<int>(EQ::invslot::slotFinger1)),
			luabind::value("Finger2", static_cast<int>(EQ::invslot::slotFinger2)),
			luabind::value("Chest", static_cast<int>(EQ::invslot::slotChest)),
			luabind::value("Legs", static_cast<int>(EQ::invslot::slotLegs)),
			luabind::value("Feet", static_cast<int>(EQ::invslot::slotFeet)),
			luabind::value("Waist", static_cast<int>(EQ::invslot::slotWaist)),
			luabind::value("PowerSource", static_cast<int>(EQ::invslot::slotPowerSource)),
			luabind::value("Ammo", static_cast<int>(EQ::invslot::slotAmmo)),
			luabind::value("General1", static_cast<int>(EQ::invslot::slotGeneral1)),
			luabind::value("General2", static_cast<int>(EQ::invslot::slotGeneral2)),
			luabind::value("General3", static_cast<int>(EQ::invslot::slotGeneral3)),
			luabind::value("General4", static_cast<int>(EQ::invslot::slotGeneral4)),
			luabind::value("General5", static_cast<int>(EQ::invslot::slotGeneral5)),
			luabind::value("General6", static_cast<int>(EQ::invslot::slotGeneral6)),
			luabind::value("General7", static_cast<int>(EQ::invslot::slotGeneral7)),
			luabind::value("General8", static_cast<int>(EQ::invslot::slotGeneral8)),
			luabind::value("General9", static_cast<int>(EQ::invslot::slotGeneral9)),
			luabind::value("General10", static_cast<int>(EQ::invslot::slotGeneral10)),
			luabind::value("Cursor", static_cast<int>(EQ::invslot::slotCursor)),
			luabind::value("PossessionsBegin", static_cast<int>(EQ::invslot::POSSESSIONS_BEGIN)),
			luabind::value("PossessionsEnd", static_cast<int>(EQ::invslot::POSSESSIONS_END)),
			luabind::value("EquipmentBegin", static_cast<int>(EQ::invslot::EQUIPMENT_BEGIN)),
			luabind::value("EquipmentEnd", static_cast<int>(EQ::invslot::EQUIPMENT_END)),
			luabind::value("GeneralBegin", static_cast<int>(EQ::invslot::GENERAL_BEGIN)),
			luabind::value("GeneralEnd", static_cast<int>(EQ::invslot::GENERAL_END)),
			luabind::value("PossessionsBagsBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN)),
			luabind::value("PossessionsBagsEnd", static_cast<int>(EQ::invbag::CURSOR_BAG_END)),
			luabind::value("GeneralBagsBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN)),
			luabind::value("GeneralBagsEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_END)),
			luabind::value("General1BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN)),
			luabind::value("General1BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 9),
			luabind::value("General2BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 10),
			luabind::value("General2BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 19),
			luabind::value("General3BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 20),
			luabind::value("General3BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 29),
			luabind::value("General4BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 30),
			luabind::value("General4BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 39),
			luabind::value("General5BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 40),
			luabind::value("General5BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 49),
			luabind::value("General6BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 50),
			luabind::value("General6BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 59),
			luabind::value("General7BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 60),
			luabind::value("General7BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 69),
			luabind::value("General8BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 70),
			luabind::value("General8BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 79),
			luabind::value("General9BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 80),
			luabind::value("General9BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 89),
			luabind::value("General10BagBegin", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 90),
			luabind::value("General10BagEnd", static_cast<int>(EQ::invbag::GENERAL_BAGS_BEGIN) + 99),
			luabind::value("CursorBagBegin", static_cast<int>(EQ::invbag::CURSOR_BAG_BEGIN)),
			luabind::value("CursorBagEnd", static_cast<int>(EQ::invbag::CURSOR_BAG_END)),
			luabind::value("Tradeskill", static_cast<int>(EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE)),
			luabind::value("Augment", static_cast<int>(EQ::invslot::SLOT_AUGMENT_GENERIC_RETURN)),
			luabind::value("BankBegin", static_cast<int>(EQ::invslot::BANK_BEGIN)),
			luabind::value("BankEnd", static_cast<int>(EQ::invslot::BANK_END)),
			luabind::value("BankBagsBegin", static_cast<int>(EQ::invbag::BANK_BAGS_BEGIN)),
			luabind::value("BankBagsEnd", static_cast<int>(EQ::invbag::BANK_BAGS_END)),
			luabind::value("SharedBankBegin", static_cast<int>(EQ::invslot::SHARED_BANK_BEGIN)),
			luabind::value("SharedBankEnd", static_cast<int>(EQ::invslot::SHARED_BANK_END)),
			luabind::value("SharedBankBagsBegin", static_cast<int>(EQ::invbag::SHARED_BANK_BAGS_BEGIN)),
			luabind::value("SharedBankBagsEnd", static_cast<int>(EQ::invbag::SHARED_BANK_BAGS_END)),
			luabind::value("BagSlotBegin", static_cast<int>(EQ::invbag::SLOT_BEGIN)),
			luabind::value("BagSlotEnd", static_cast<int>(EQ::invbag::SLOT_END)),
			luabind::value("AugSocketBegin", static_cast<int>(EQ::invaug::SOCKET_BEGIN)),
			luabind::value("AugSocketEnd", static_cast<int>(EQ::invaug::SOCKET_END)),
			luabind::value("Invalid", static_cast<int>(EQ::invslot::SLOT_INVALID)),

			luabind::value("Shoulder", static_cast<int>(EQ::invslot::slotShoulders)), // deprecated
			luabind::value("Bracer1", static_cast<int>(EQ::invslot::slotWrist1)), // deprecated
			luabind::value("Bracer2", static_cast<int>(EQ::invslot::slotWrist2)), // deprecated
			luabind::value("Ring1", static_cast<int>(EQ::invslot::slotFinger1)), // deprecated
			luabind::value("Ring2", static_cast<int>(EQ::invslot::slotFinger2)), // deprecated
			luabind::value("PersonalBegin", static_cast<int>(EQ::invslot::GENERAL_BEGIN)), // deprecated
			luabind::value("PersonalEnd", static_cast<int>(EQ::invslot::GENERAL_END)), // deprecated
			luabind::value("CursorEnd", 0xFFFE) // deprecated (not in use..and never valid vis-a-vis client behavior)
		];
}

luabind::scope lua_register_material() {
	return luabind::class_<Materials>("Material")
		.enum_("constants")
		[
			luabind::value("Head", static_cast<int>(EQ::textures::armorHead)),
			luabind::value("Chest", static_cast<int>(EQ::textures::armorChest)),
			luabind::value("Arms", static_cast<int>(EQ::textures::armorArms)),
			luabind::value("Wrist", static_cast<int>(EQ::textures::armorWrist)),
			luabind::value("Hands", static_cast<int>(EQ::textures::armorHands)),
			luabind::value("Legs", static_cast<int>(EQ::textures::armorLegs)),
			luabind::value("Feet", static_cast<int>(EQ::textures::armorFeet)),
			luabind::value("Primary", static_cast<int>(EQ::textures::weaponPrimary)),
			luabind::value("Secondary", static_cast<int>(EQ::textures::weaponSecondary)),
			luabind::value("Count", static_cast<int>(EQ::textures::materialCount)),
			luabind::value("Invalid", static_cast<int>(EQ::textures::materialInvalid)),

			luabind::value("Bracer", static_cast<int>(EQ::textures::armorWrist)), // deprecated
			luabind::value("Max", static_cast<int>(EQ::textures::materialCount)) // deprecated
		];
}

luabind::scope lua_register_client_version() {
	return luabind::class_<ClientVersions>("ClientVersion")
		.enum_("constants")
		[
			luabind::value("Unknown", static_cast<int>(EQ::versions::ClientVersion::Unknown)),
			luabind::value("Titanium", static_cast<int>(EQ::versions::ClientVersion::Titanium)),
			luabind::value("SoF", static_cast<int>(EQ::versions::ClientVersion::SoF)),
			luabind::value("SoD", static_cast<int>(EQ::versions::ClientVersion::SoD)),
			luabind::value("Underfoot", static_cast<int>(EQ::versions::ClientVersion::UF)), // deprecated
			luabind::value("UF", static_cast<int>(EQ::versions::ClientVersion::UF)),
			luabind::value("RoF", static_cast<int>(EQ::versions::ClientVersion::RoF)),
			luabind::value("RoF2", static_cast<int>(EQ::versions::ClientVersion::RoF2))
		];
}

luabind::scope lua_register_appearance() {
	return luabind::class_<Appearances>("Appearance")
		.enum_("constants")
		[
			luabind::value("Standing", static_cast<int>(eaStanding)),
			luabind::value("Sitting", static_cast<int>(eaSitting)),
			luabind::value("Crouching", static_cast<int>(eaCrouching)),
			luabind::value("Dead", static_cast<int>(eaDead)),
			luabind::value("Looting", static_cast<int>(eaLooting))
		];
}

luabind::scope lua_register_classes() {
	return luabind::class_<Classes>("Class")
		.enum_("constants")
		[
			luabind::value("WARRIOR", WARRIOR),
			luabind::value("CLERIC", CLERIC),
			luabind::value("PALADIN", PALADIN),
			luabind::value("RANGER", RANGER),
			luabind::value("SHADOWKNIGHT", SHADOWKNIGHT),
			luabind::value("DRUID", DRUID),
			luabind::value("MONK", MONK),
			luabind::value("BARD", BARD),
			luabind::value("ROGUE", ROGUE),
			luabind::value("SHAMAN", SHAMAN),
			luabind::value("NECROMANCER", NECROMANCER),
			luabind::value("WIZARD", WIZARD),
			luabind::value("MAGICIAN", MAGICIAN),
			luabind::value("ENCHANTER", ENCHANTER),
			luabind::value("BEASTLORD", BEASTLORD),
			luabind::value("BERSERKER", BERSERKER),
			luabind::value("WARRIORGM", WARRIORGM),
			luabind::value("CLERICGM", CLERICGM),
			luabind::value("PALADINGM", PALADINGM),
			luabind::value("RANGERGM", RANGERGM),
			luabind::value("SHADOWKNIGHTGM", SHADOWKNIGHTGM),
			luabind::value("DRUIDGM", DRUIDGM),
			luabind::value("MONKGM", MONKGM),
			luabind::value("BARDGM", BARDGM),
			luabind::value("ROGUEGM", ROGUEGM),
			luabind::value("SHAMANGM", SHAMANGM),
			luabind::value("NECROMANCERGM", NECROMANCERGM),
			luabind::value("WIZARDGM", WIZARDGM),
			luabind::value("MAGICIANGM", MAGICIANGM),
			luabind::value("ENCHANTERGM", ENCHANTERGM),
			luabind::value("BEASTLORDGM", BEASTLORDGM),
			luabind::value("BERSERKERGM", BERSERKERGM),
			luabind::value("BANKER", BANKER),
			luabind::value("MERCHANT", MERCHANT),
			luabind::value("DISCORD_MERCHANT", DISCORD_MERCHANT),
			luabind::value("ADVENTURERECRUITER", ADVENTURERECRUITER),
			luabind::value("ADVENTUREMERCHANT", ADVENTUREMERCHANT),
			luabind::value("LDON_TREASURE", LDON_TREASURE),
			luabind::value("CORPSE_CLASS", CORPSE_CLASS),
			luabind::value("TRIBUTE_MASTER", TRIBUTE_MASTER),
			luabind::value("GUILD_TRIBUTE_MASTER", GUILD_TRIBUTE_MASTER),
			luabind::value("NORRATHS_KEEPERS_MERCHANT", NORRATHS_KEEPERS_MERCHANT),
			luabind::value("DARK_REIGN_MERCHANT", DARK_REIGN_MERCHANT),
			luabind::value("FELLOWSHIP_MASTER", FELLOWSHIP_MASTER),
			luabind::value("ALT_CURRENCY_MERCHANT", ALT_CURRENCY_MERCHANT),
			luabind::value("MERCERNARY_MASTER", MERCERNARY_MASTER)
		];
}

luabind::scope lua_register_skills() {
	return luabind::class_<Skills>("Skill")
		.enum_("constants")
		[
			luabind::value("1HBlunt", EQ::skills::Skill1HBlunt),
			luabind::value("Blunt1H", EQ::skills::Skill1HBlunt),
			luabind::value("1HSlashing", EQ::skills::Skill1HSlashing),
			luabind::value("Slashing1H", EQ::skills::Skill1HSlashing),
			luabind::value("2HBlunt", EQ::skills::Skill2HBlunt),
			luabind::value("Blunt2H", EQ::skills::Skill2HBlunt),
			luabind::value("2HSlashing", EQ::skills::Skill2HSlashing),
			luabind::value("Slashing2H", EQ::skills::Skill2HSlashing),
			luabind::value("Abjuration", EQ::skills::SkillAbjuration),
			luabind::value("Alteration", EQ::skills::SkillAlteration),
			luabind::value("ApplyPoison", EQ::skills::SkillApplyPoison),
			luabind::value("Archery", EQ::skills::SkillArchery),
			luabind::value("Backstab", EQ::skills::SkillBackstab),
			luabind::value("BindWound", EQ::skills::SkillBindWound),
			luabind::value("Bash", EQ::skills::SkillBash),
			luabind::value("Block", EQ::skills::SkillBlock),
			luabind::value("BrassInstruments", EQ::skills::SkillBrassInstruments),
			luabind::value("Channeling", EQ::skills::SkillChanneling),
			luabind::value("Conjuration", EQ::skills::SkillConjuration),
			luabind::value("Defense", EQ::skills::SkillDefense),
			luabind::value("Disarm", EQ::skills::SkillDisarm),
			luabind::value("DisarmTraps", EQ::skills::SkillDisarmTraps),
			luabind::value("Divination", EQ::skills::SkillDivination),
			luabind::value("Dodge", EQ::skills::SkillDodge),
			luabind::value("DoubleAttack", EQ::skills::SkillDoubleAttack),
			luabind::value("DragonPunch", EQ::skills::SkillDragonPunch),
			luabind::value("TailRake", EQ::skills::SkillTailRake),
			luabind::value("DualWield", EQ::skills::SkillDualWield),
			luabind::value("EagleStrike", EQ::skills::SkillEagleStrike),
			luabind::value("Evocation", EQ::skills::SkillEvocation),
			luabind::value("FeignDeath", EQ::skills::SkillFeignDeath),
			luabind::value("FlyingKick", EQ::skills::SkillFlyingKick),
			luabind::value("Forage", EQ::skills::SkillForage),
			luabind::value("HandtoHand", EQ::skills::SkillHandtoHand),
			luabind::value("Hide", EQ::skills::SkillHide),
			luabind::value("Kick", EQ::skills::SkillKick),
			luabind::value("Meditate", EQ::skills::SkillMeditate),
			luabind::value("Mend", EQ::skills::SkillMend),
			luabind::value("Offense", EQ::skills::SkillOffense),
			luabind::value("Parry", EQ::skills::SkillParry),
			luabind::value("PickLock", EQ::skills::SkillPickLock),
			luabind::value("1HPiercing", EQ::skills::Skill1HPiercing),
			luabind::value("Piercing1H", EQ::skills::Skill1HPiercing),
			luabind::value("Riposte", EQ::skills::SkillRiposte),
			luabind::value("RoundKick", EQ::skills::SkillRoundKick),
			luabind::value("SafeFall", EQ::skills::SkillSafeFall),
			luabind::value("SenseHeading", EQ::skills::SkillSenseHeading),
			luabind::value("Singing", EQ::skills::SkillSinging),
			luabind::value("Sneak", EQ::skills::SkillSneak),
			luabind::value("SpecializeAbjure", EQ::skills::SkillSpecializeAbjure),
			luabind::value("SpecializeAlteration", EQ::skills::SkillSpecializeAlteration),
			luabind::value("SpecializeConjuration", EQ::skills::SkillSpecializeConjuration),
			luabind::value("SpecializeDivination", EQ::skills::SkillSpecializeDivination),
			luabind::value("SpecializeEvocation", EQ::skills::SkillSpecializeEvocation),
			luabind::value("PickPockets", EQ::skills::SkillPickPockets),
			luabind::value("StringedInstruments", EQ::skills::SkillStringedInstruments),
			luabind::value("Swimming", EQ::skills::SkillSwimming),
			luabind::value("Throwing", EQ::skills::SkillThrowing),
			luabind::value("TigerClaw", EQ::skills::SkillTigerClaw),
			luabind::value("Tracking", EQ::skills::SkillTracking),
			luabind::value("WindInstruments", EQ::skills::SkillWindInstruments),
			luabind::value("Fishing", EQ::skills::SkillFishing),
			luabind::value("MakePoison", EQ::skills::SkillMakePoison),
			luabind::value("Tinkering", EQ::skills::SkillTinkering),
			luabind::value("Research", EQ::skills::SkillResearch),
			luabind::value("Alchemy", EQ::skills::SkillAlchemy),
			luabind::value("Baking", EQ::skills::SkillBaking),
			luabind::value("Tailoring", EQ::skills::SkillTailoring),
			luabind::value("SenseTraps", EQ::skills::SkillSenseTraps),
			luabind::value("Blacksmithing", EQ::skills::SkillBlacksmithing),
			luabind::value("Fletching", EQ::skills::SkillFletching),
			luabind::value("Brewing", EQ::skills::SkillBrewing),
			luabind::value("AlcoholTolerance", EQ::skills::SkillAlcoholTolerance),
			luabind::value("Begging", EQ::skills::SkillBegging),
			luabind::value("JewelryMaking", EQ::skills::SkillJewelryMaking),
			luabind::value("Pottery", EQ::skills::SkillPottery),
			luabind::value("PercussionInstruments", EQ::skills::SkillPercussionInstruments),
			luabind::value("Intimidation", EQ::skills::SkillIntimidation),
			luabind::value("Berserking", EQ::skills::SkillBerserking),
			luabind::value("Taunt", EQ::skills::SkillTaunt),
			luabind::value("Frenzy", EQ::skills::SkillFrenzy),
			luabind::value("RemoveTraps", EQ::skills::SkillRemoveTraps),
			luabind::value("TripleAttack", EQ::skills::SkillTripleAttack),
			luabind::value("2HPiercing", EQ::skills::Skill2HPiercing),
			luabind::value("Piercing2H", EQ::skills::Skill2HPiercing),
			luabind::value("HIGHEST_SKILL", EQ::skills::HIGHEST_SKILL)
		];
}

luabind::scope lua_register_bodytypes() {
	return luabind::class_<BodyTypes>("BT")
		.enum_("constants")
		[
			luabind::value("Humanoid", 1),
			luabind::value("Lycanthrope", 2),
			luabind::value("Undead", 3),
			luabind::value("Giant", 4),
			luabind::value("Construct", 5),
			luabind::value("Extraplanar", 6),
			luabind::value("Magical", 7),
			luabind::value("SummonedUndead", 8),
			luabind::value("RaidGiant", 9),
			luabind::value("NoTarget", 11),
			luabind::value("Vampire", 12),
			luabind::value("Atenha_Ra", 13),
			luabind::value("Greater_Akheva", 14),
			luabind::value("Khati_Sha", 15),
			luabind::value("Seru", 16),
			luabind::value("Draz_Nurakk", 18),
			luabind::value("Zek", 19),
			luabind::value("Luggald", 20),
			luabind::value("Animal", 21),
			luabind::value("Insect", 22),
			luabind::value("Monster", 23),
			luabind::value("Summoned", 24),
			luabind::value("Plant", 25),
			luabind::value("Dragon", 26),
			luabind::value("Summoned2", 27),
			luabind::value("Summoned3", 28),
			luabind::value("VeliousDragon", 30),
			luabind::value("Dragon3", 32),
			luabind::value("Boxes", 33),
			luabind::value("Muramite", 34),
			luabind::value("NoTarget2", 60),
			luabind::value("SwarmPet", 63),
			luabind::value("InvisMan", 66),
			luabind::value("Special", 67)
		];
}

luabind::scope lua_register_filters() {
	return luabind::class_<Filters>("Filter")
		.enum_("constants")
		[
			luabind::value("None", FilterNone),
			luabind::value("GuildChat", FilterGuildChat),
			luabind::value("Socials", FilterSocials),
			luabind::value("GroupChat", FilterGroupChat),
			luabind::value("Shouts", FilterShouts),
			luabind::value("Auctions", FilterAuctions),
			luabind::value("OOC", FilterOOC),
			luabind::value("BadWords", FilterBadWords),
			luabind::value("PCSpells", FilterPCSpells),
			luabind::value("NPCSpells", FilterNPCSpells),
			luabind::value("BardSongs", FilterBardSongs),
			luabind::value("SpellCrits", FilterSpellCrits),
			luabind::value("MeleeCrits", FilterMeleeCrits),
			luabind::value("SpellDamage", FilterSpellDamage),
			luabind::value("MyMisses", FilterMyMisses),
			luabind::value("OthersMiss", FilterOthersMiss),
			luabind::value("OthersHit", FilterOthersHit),
			luabind::value("MissedMe", FilterMissedMe),
			luabind::value("DamageShields", FilterDamageShields),
			luabind::value("DOT", FilterDOT),
			luabind::value("PetHits", FilterPetHits),
			luabind::value("PetMisses", FilterPetMisses),
			luabind::value("FocusEffects", FilterFocusEffects),
			luabind::value("PetSpells", FilterPetSpells),
			luabind::value("HealOverTime", FilterHealOverTime),
			luabind::value("Unknown25", FilterUnknown25),
			luabind::value("Unknown26", FilterUnknown26),
			luabind::value("Unknown27", FilterUnknown27),
			luabind::value("Unknown28", FilterUnknown28)
		];
}

luabind::scope lua_register_message_types() {
	return luabind::class_<MessageTypes>("MT")
		.enum_("constants")
		[
			luabind::value("White", Chat::White),
			luabind::value("DimGray", Chat::DimGray),
			luabind::value("Default", Chat::Default),
			luabind::value("Green", Chat::Green),
			luabind::value("BrightBlue", Chat::BrightBlue),
			luabind::value("LightBlue", Chat::LightBlue),
			luabind::value("Magenta", Chat::Magenta),
			luabind::value("Gray", Chat::Gray),
			luabind::value("LightGray", Chat::LightGray),
			luabind::value("NPCQuestSay", Chat::NPCQuestSay),
			luabind::value("DarkGray", Chat::DarkGray),
			luabind::value("Red", Chat::Red),
			luabind::value("Lime", Chat::Lime),
			luabind::value("Yellow", Chat::Yellow),
			luabind::value("Blue", Chat::Blue),
			luabind::value("LightNavy", Chat::LightNavy),
			luabind::value("Cyan", Chat::Cyan),
			luabind::value("Black", Chat::Black),
			luabind::value("Say", Chat::Say),
			luabind::value("Tell", Chat::Tell),
			luabind::value("Group", Chat::Group),
			luabind::value("Guild", Chat::Guild),
			luabind::value("OOC", Chat::OOC),
			luabind::value("Auction", Chat::Auction),
			luabind::value("Shout", Chat::Shout),
			luabind::value("Emote", Chat::Emote),
			luabind::value("Spells", Chat::Spells),
			luabind::value("YouHitOther", Chat::YouHitOther),
			luabind::value("OtherHitsYou", Chat::OtherHitYou),
			luabind::value("YouMissOther", Chat::YouMissOther),
			luabind::value("OtherMissesYou", Chat::OtherMissYou),
			luabind::value("Broadcasts", Chat::Broadcasts),
			luabind::value("Skills", Chat::Skills),
			luabind::value("Disciplines", Chat::Disciplines),
			luabind::value("Unused1", Chat::Unused1),
			luabind::value("DefaultText", Chat::DefaultText),
			luabind::value("Unused2", Chat::Unused2),
			luabind::value("MerchantOffer", Chat::MerchantOffer),
			luabind::value("MerchantBuySell", Chat::MerchantExchange),
			luabind::value("YourDeath", Chat::YourDeath),
			luabind::value("OtherDeath", Chat::OtherDeath),
			luabind::value("OtherHits", Chat::OtherHitOther),
			luabind::value("OtherMisses", Chat::OtherMissOther),
			luabind::value("Who", Chat::Who),
			luabind::value("YellForHelp", Chat::YellForHelp),
			luabind::value("NonMelee", Chat::NonMelee),
			luabind::value("WornOff", Chat::SpellWornOff),
			luabind::value("MoneySplit", Chat::MoneySplit),
			luabind::value("LootMessages", Chat::Loot),
			luabind::value("DiceRoll", Chat::DiceRoll),
			luabind::value("OtherSpells", Chat::OtherSpells),
			luabind::value("SpellFailure", Chat::SpellFailure),
			luabind::value("Chat", Chat::ChatChannel),
			luabind::value("Channel1", Chat::Chat1),
			luabind::value("Channel2", Chat::Chat2),
			luabind::value("Channel3", Chat::Chat3),
			luabind::value("Channel4", Chat::Chat4),
			luabind::value("Channel5", Chat::Chat5),
			luabind::value("Channel6", Chat::Chat6),
			luabind::value("Channel7", Chat::Chat7),
			luabind::value("Channel8", Chat::Chat8),
			luabind::value("Channel9", Chat::Chat9),
			luabind::value("Channel10", Chat::Chat10),
			luabind::value("CritMelee", Chat::MeleeCrit),
			luabind::value("SpellCrits", Chat::SpellCrit),
			luabind::value("TooFarAway", Chat::TooFarAway),
			luabind::value("NPCRampage", Chat::NPCRampage),
			luabind::value("NPCFlurry", Chat::NPCFlurry),
			luabind::value("NPCEnrage", Chat::NPCEnrage),
			luabind::value("SayEcho", Chat::EchoSay),
			luabind::value("TellEcho", Chat::EchoTell),
			luabind::value("GroupEcho", Chat::EchoGroup),
			luabind::value("GuildEcho", Chat::EchoGuild),
			luabind::value("OOCEcho", Chat::EchoOOC),
			luabind::value("AuctionEcho", Chat::EchoAuction),
			luabind::value("ShoutECho", Chat::EchoShout),
			luabind::value("EmoteEcho", Chat::EchoEmote),
			luabind::value("Chat1Echo", Chat::EchoChat1),
			luabind::value("Chat2Echo", Chat::EchoChat2),
			luabind::value("Chat3Echo", Chat::EchoChat3),
			luabind::value("Chat4Echo", Chat::EchoChat4),
			luabind::value("Chat5Echo", Chat::EchoChat5),
			luabind::value("Chat6Echo", Chat::EchoChat6),
			luabind::value("Chat7Echo", Chat::EchoChat7),
			luabind::value("Chat8Echo", Chat::EchoChat8),
			luabind::value("Chat9Echo", Chat::EchoChat9),
			luabind::value("Chat10Echo", Chat::EchoChat10),
			luabind::value("DoTDamage", Chat::DotDamage),
			luabind::value("ItemLink", Chat::ItemLink),
			luabind::value("RaidSay", Chat::RaidSay),
			luabind::value("MyPet", Chat::MyPet),
			luabind::value("DS", Chat::DamageShield),
			luabind::value("Leadership", Chat::LeaderShip),
			luabind::value("PetFlurry", Chat::PetFlurry),
			luabind::value("PetCrit", Chat::PetCritical),
			luabind::value("FocusEffect", Chat::FocusEffect),
			luabind::value("Experience", Chat::Experience),
			luabind::value("System", Chat::System),
			luabind::value("PetSpell", Chat::PetSpell),
			luabind::value("PetResponse", Chat::PetResponse),
			luabind::value("ItemSpeech", Chat::ItemSpeech),
			luabind::value("StrikeThrough", Chat::StrikeThrough),
			luabind::value("Stun", Chat::Stun)
		];
}

luabind::scope lua_register_rules_const() {
	return luabind::class_<Rule>("Rule")
		.enum_("constants")
	[
#define RULE_INT(cat, rule, default_value, notes) \
		luabind::value(#rule, RuleManager::Int__##rule),
#include "../common/ruletypes.h"
		luabind::value("_IntRuleCount", RuleManager::_IntRuleCount),
#undef RULE_INT
#define RULE_REAL(cat, rule, default_value, notes) \
		luabind::value(#rule, RuleManager::Real__##rule),
#include "../common/ruletypes.h"
		luabind::value("_RealRuleCount", RuleManager::_RealRuleCount),
#undef RULE_REAL
#define RULE_BOOL(cat, rule, default_value, notes) \
		luabind::value(#rule, RuleManager::Bool__##rule),
#include "../common/ruletypes.h"
		luabind::value("_BoolRuleCount", RuleManager::_BoolRuleCount)
	];
}

luabind::scope lua_register_rulei() {
	return luabind::namespace_("RuleI")
		[
			luabind::def("Get", &get_rulei)
		];
}

luabind::scope lua_register_ruler() {
	return luabind::namespace_("RuleR")
		[
			luabind::def("Get", &get_ruler)
		];
}

luabind::scope lua_register_ruleb() {
	return luabind::namespace_("RuleB")
		[
			luabind::def("Get", &get_ruleb)
		];
}

luabind::scope lua_register_journal_speakmode() {
	return luabind::class_<Journal_SpeakMode>("SpeakMode")
		.enum_("constants")
		[
			luabind::value("Raw", static_cast<int>(Journal::SpeakMode::Raw)),
			luabind::value("Say", static_cast<int>(Journal::SpeakMode::Say)),
			luabind::value("Shout", static_cast<int>(Journal::SpeakMode::Shout)),
			luabind::value("EmoteAlt", static_cast<int>(Journal::SpeakMode::EmoteAlt)),
			luabind::value("Emote", static_cast<int>(Journal::SpeakMode::Emote)),
			luabind::value("Group", static_cast<int>(Journal::SpeakMode::Group))
		];
}

luabind::scope lua_register_journal_mode() {
	return luabind::class_<Journal_Mode>("JournalMode")
		.enum_("constants")
		[
			luabind::value("None", static_cast<int>(Journal::Mode::None)),
			luabind::value("Log1", static_cast<int>(Journal::Mode::Log1)),
			luabind::value("Log2", static_cast<int>(Journal::Mode::Log2))
		];
}

#endif
