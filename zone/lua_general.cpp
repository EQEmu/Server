#ifdef LUA_EQEMU

#include <sol/sol.hpp>

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

struct lua_registered_event {
	std::string encounter_name;
	sol::function lua_reference;
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
	std::vector<std::any> info_ptrs;
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
	std::vector<std::any> info_ptrs;
	info_ptrs.push_back(&info_str);
	parse->EventEncounter(EVENT_ENCOUNTER_UNLOAD, name, "", 0, &info_ptrs);
}

void register_event(std::string package_name, std::string name, int evt, sol::protected_function func) {
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

void register_npc_event(std::string name, int evt, int npc_id, sol::protected_function func) {
	std::stringstream package_name;
	package_name << "npc_" << npc_id;

	register_event(package_name.str(), name, evt, func);
}

void register_npc_event(int evt, int npc_id, sol::protected_function func) {
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

void register_player_event(std::string name, int evt, sol::protected_function func) {
	register_event("player", name, evt, func);
}

void register_player_event(int evt, sol::protected_function func) {
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

void register_item_event(std::string name, int evt, int item_id, sol::protected_function func) {
	std::string package_name = "item_";
	package_name += std::to_string(item_id);

	register_event(package_name, name, evt, func);
}

void register_item_event(int evt, int item_id, sol::protected_function func) {
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

void register_spell_event(std::string name, int evt, int spell_id, sol::protected_function func) {
	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	register_event(package_name.str(), name, evt, func);
}

void register_spell_event(int evt, int spell_id, sol::protected_function func) {
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

void lua_task_selector(sol::table table, bool ignore_cooldown = false) {
	std::vector<int> tasks;
	for (int i = 1; i <= MAXCHOOSERENTRIES; ++i)
	{
		auto task = table.get<sol::optional<int>>(table[i]);
		if (task)
		{
			tasks.push_back(task.value());
		}
	}

	quest_manager.taskselector(tasks, ignore_cooldown);
}

void lua_task_selector(sol::table table) {
	lua_task_selector(table, false);
}

void lua_task_set_selector(int task_set) {
	quest_manager.tasksetselector(task_set);
}

void lua_task_set_selector(int task_set, bool ignore_cooldown) {
	quest_manager.tasksetselector(task_set, ignore_cooldown);
}

void lua_enable_task(sol::table table) {
	int tasks[MAXCHOOSERENTRIES] = { 0 };
	int count = 0;

	for (int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto cur = table.get<sol::optional<int>>(i);
		if (cur) {
			tasks[i - 1] = cur.value();
		} else {
			count = i - 1;
			break;
		}
	}

	quest_manager.enabletask(count, tasks);
}

void lua_disable_task(sol::table table) {
	int tasks[MAXCHOOSERENTRIES] = { 0 };
	int count = 0;

	for (int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto cur = table.get<sol::optional<int>>(i);
		if (cur) {
		tasks[i - 1] = cur.value();
		} else {
			count = i - 1;
			break;
		}
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

sol::table lua_get_qglobals(sol::this_state s, Lua_NPC npc, Lua_Client client) {
	sol::state_view sv(s);
	auto ret = sv.create_table();

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

sol::table lua_get_qglobals(sol::this_state s, Lua_Client client) {
	sol::state_view sv(s);
	auto ret = sv.create_table();

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

sol::table lua_get_qglobals(sol::this_state s, Lua_NPC npc) {
	sol::state_view sv(s);
	auto ret = sv.create_table();

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

sol::table lua_get_qglobals(sol::this_state s) {
	sol::state_view sv(s);
	auto ret = sv.create_table();

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

sol::table lua_get_characters_in_instance(sol::this_state s, uint16 instance_id) {
	sol::state_view sv(s);
	auto ret = sv.create_table();

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
	if (!zone) {
		return EQ::constants::WeatherTypes::None;
	}

	return zone->zone_weather;
}

sol::table lua_get_zone_time(sol::this_state s) {
	TimeOfDay_Struct eqTime;
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), &eqTime);

	sol::state_view sv(s);
	auto ret = sv.create_table();
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

void lua_add_spawn_point(sol::table table) {
	if(!zone)
		return;

	uint32 spawn2_id;
	uint32 spawngroup_id;
	float x;
	float y;
	float z;
	float heading;
	uint32 respawn;
	uint32 variance;
	uint32 timeleft = table.get_or("timeleft", 0);
	uint32 grid = table.get_or("grid", 0);
	bool path_when_zone_idle = table.get_or("path_when_zone_idle", false);
	int condition_id = table.get_or("condition_id", 0);
	int condition_min_value = table.get_or("condition_id", 0);
	bool enabled = table.get_or("enabled", true);
	int animation = table.get_or("animation", 0);

	auto opt_uint32 = table.get<sol::optional<uint32>>("spawn2_id");
	if(opt_uint32) {
		spawn2_id = opt_uint32.value();
	} else {
		return;
	}

	opt_uint32 = table.get<sol::optional<uint32>>("spawngroup_id");
	if(opt_uint32) {
		spawngroup_id = opt_uint32.value();
	} else {
		return;
	}

	auto opt_float = table.get<sol::optional<float>>("x");
	if(opt_float) {
		x = opt_float.value();
	} else {
		return;
	}

	opt_float = table.get<sol::optional<float>>("y");
	if(opt_float) {
		y = opt_float.value();
	} else {
		return;
	}

	opt_float = table.get<sol::optional<float>>("z");
	if (opt_float) {
		z = opt_float.value();
	} else {
		return;
	}

	opt_float = table.get<sol::optional<float>>("heading");
	if (opt_float) {
		heading = opt_float.value();
	} else {
		return;
	}

	opt_uint32 = table.get<sol::optional<uint32>>("respawn");
	if (opt_uint32) {
		respawn = opt_uint32.value();
	} else {
		return;
	}

	opt_uint32 = table.get<sol::optional<uint32>>("variance");
	if (opt_uint32) {
		variance = opt_uint32.value();
	} else {
		return;
	}

	lua_remove_spawn_point(spawn2_id);

	auto t = new Spawn2(spawn2_id, spawngroup_id, x, y, z, heading, respawn,
		variance, timeleft, grid, path_when_zone_idle, condition_id,
		condition_min_value, enabled, static_cast<EmuAppearance>(animation));

	zone->spawn2_list.Insert(t);
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

bool lua_is_the_darkened_sea_enabled() {
	return content_service.IsTheDarkenedSeaEnabled();
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

bool lua_is_current_expansion_the_darkened_sea() {
	return content_service.IsCurrentExpansionTheDarkenedSea();
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

sol::table lua_get_expedition_lockout_by_char_id(sol::this_state s, uint32 char_id, std::string expedition_name, std::string event_name) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();

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

sol::table lua_get_expedition_lockouts_by_char_id(sol::this_state s, uint32 char_id) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();

	auto lockouts = Expedition::GetExpeditionLockoutsByCharacterID(char_id);
	for (const auto& lockout : lockouts)
	{
		auto lockout_table = lua_table.get_or(lockout.GetExpeditionName(), sv.create_table());

		auto event_table = lockout_table.get_or(lockout.GetEventName(), sv.create_table());

		event_table["remaining"] = lockout.GetSecondsRemaining();
		event_table["uuid"] = lockout.GetExpeditionUUID();
	}
	return lua_table;
}

sol::table lua_get_expedition_lockouts_by_char_id(sol::this_state s, uint32 char_id, std::string expedition_name) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();

	auto lockouts = Expedition::GetExpeditionLockoutsByCharacterID(char_id);
	for (const auto& lockout : lockouts)
	{
		if (lockout.GetExpeditionName() == expedition_name)
		{
			auto event_table = lua_table.get_or(lockout.GetEventName(), sv.create_table());

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

double lua_get_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, int16 instance_version) {
	return database.GetAAEXPModifier(character_id, zone_id, instance_version);
}

double lua_get_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id) {
	return database.GetEXPModifier(character_id, zone_id);
}

double lua_get_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, int16 instance_version) {
	return database.GetEXPModifier(character_id, zone_id, instance_version);
}

void lua_set_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, double aa_modifier) {
	database.SetAAEXPModifier(character_id, zone_id, aa_modifier);
}

void lua_set_aa_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, double aa_modifier, int16 instance_version) {
	database.SetAAEXPModifier(character_id, zone_id, aa_modifier, instance_version);
}

void lua_set_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, double exp_modifier) {
	database.SetEXPModifier(character_id, zone_id, exp_modifier);
}

void lua_set_exp_modifier_by_char_id(uint32 character_id, uint32 zone_id, double exp_modifier, int16 instance_version) {
	database.SetEXPModifier(character_id, zone_id, exp_modifier, instance_version);
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

bool lua_is_npc_spawned(sol::table table) {
	std::vector<uint32> npc_ids;

	for (int i = 1; i <= table.size(); ++i) {
		auto cur = table.get<sol::optional<uint32>>(i);
		if (cur) {
			npc_ids.push_back(cur.value());
		} else {
			break; // break early if they give us something dumb
		}
	}

	if (npc_ids.empty()) {
		return false;
	}

	return entity_list.IsNPCSpawned(npc_ids);
}

uint32 lua_count_spawned_npcs(sol::table table) {
	std::vector<uint32> npc_ids;

	for (int i = 1; i <= table.size(); ++i) {
		auto cur = table.get<sol::optional<uint32>>(i);
		if (cur) {
			npc_ids.push_back(cur.value());
		} else {
			break; // break early if they give us something dumb
		}
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

std::string lua_commify(std::string number) {
	return Strings::Commify(number);
}

bool lua_check_name_filter(std::string name) {
	return database.CheckNameFilter(name);
}

void lua_discord_send(std::string webhook_name, std::string message) {
	zone->SendDiscordMessage(webhook_name, message);
}

void lua_track_npc(uint32 entity_id) {
	quest_manager.TrackNPC(entity_id);
}

int lua_get_recipe_made_count(uint32 recipe_id) {
	return quest_manager.GetRecipeMadeCount(recipe_id);
}

std::string lua_get_recipe_name(uint32 recipe_id) {
	return quest_manager.GetRecipeName(recipe_id);
}

bool lua_has_recipe_learned(uint32 recipe_id) {
	return quest_manager.HasRecipeLearned(recipe_id);
}

bool lua_is_raining() {
	if (!zone) {
		return false;
	}

	return zone->IsRaining();
}

bool lua_is_snowing() {
	if (!zone) {
		return false;
	}

	return zone->IsSnowing();
}

#define LuaCreateNPCParse(name, c_type, default_value) do { \
	npc_type->name = table.get_or(#name, default_value); \
} while(0)

#define LuaCreateNPCParseString(name, str_length, default_value) do { \
	std::string cur = table.get_or(#name, std::string(default_value)); \
	strncpy(npc_type->name, cur.c_str(), str_length); \
} while(0)

void lua_create_npc(sol::table table, float x, float y, float z, float heading) {
	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

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

void lua_register_general(sol::state_view &sv)
{
	auto eq = sv.create_named_table("eq");
	eq["load_encounter"] = &load_encounter;
	eq["unload_encounter"] = &unload_encounter;
	eq["load_encounter_with_data"] = &load_encounter_with_data;
	eq["unload_encounter_with_data"] = &unload_encounter_with_data;
	eq["register_npc_event"] =
	    sol::overload((void (*)(std::string, int, int, sol::protected_function)) & register_npc_event,
			  (void (*)(int, int, sol::protected_function)) & register_npc_event);
	eq["unregister_npc_event"] = sol::overload((void (*)(std::string, int, int)) & unregister_npc_event,
						   (void (*)(int, int)) & unregister_npc_event);
	eq["register_player_event"] =
	    sol::overload((void (*)(std::string, int, sol::protected_function)) & register_player_event,
			  (void (*)(int, sol::protected_function)) & register_player_event);
	eq["unregister_player_event"] = sol::overload((void (*)(std::string, int)) & unregister_player_event,
						      (void (*)(int)) & unregister_player_event);
	eq["register_item_event"] =
	    sol::overload((void (*)(std::string, int, int, sol::protected_function)) & register_item_event,
			  (void (*)(int, int, sol::protected_function)) & register_item_event);
	eq["unregister_item_event"] = sol::overload((void (*)(std::string, int, int)) & unregister_item_event,
						    (void (*)(int, int)) & unregister_item_event);
	eq["register_spell_event"] =
	    sol::overload((void (*)(std::string, int, int, sol::protected_function func)) & register_spell_event,
			  (void (*)(int, int, sol::protected_function func)) & register_spell_event);
	eq["unregister_spell_event"] = sol::overload((void (*)(std::string, int, int)) & unregister_spell_event,
						     (void (*)(int, int)) & unregister_spell_event);
	eq["spawn2"] = (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_spawn2;
	eq["unique_spawn"] =
	    sol::overload((Lua_Mob(*)(int, int, int, double, double, double)) & lua_unique_spawn,
			  (Lua_Mob(*)(int, int, int, double, double, double, double)) & lua_unique_spawn);
	eq["spawn_from_spawn2"] = (Lua_Mob(*)(uint32))&lua_spawn_from_spawn2;
	eq["enable_spawn2"] = &lua_enable_spawn2;
	eq["disable_spawn2"] = &lua_disable_spawn2;
	eq["has_timer"] = (bool(*)(const char*))&lua_has_timer;
	eq["get_remaining_time"] = (uint32(*)(const char*))&lua_get_remaining_time;
	eq["get_timer_duration"] = (uint32(*)(const char*))&lua_get_timer_duration;
	eq["set_timer"] = sol::overload((void (*)(const char *, int)) & lua_set_timer,
					(void (*)(const char *, int, Lua_ItemInst)) & lua_set_timer,
					(void (*)(const char *, int, Lua_Mob)) & lua_set_timer,
					(void (*)(const char *, int, Lua_Encounter)) & lua_set_timer);
	eq["stop_timer"] = sol::overload((void (*)(const char *)) & lua_stop_timer,
					 (void (*)(const char *, Lua_ItemInst)) & lua_stop_timer,
					 (void (*)(const char *, Lua_Mob)) & lua_stop_timer,
					 (void (*)(const char *, Lua_Encounter)) & lua_stop_timer);
	eq["pause_timer"] = (void(*)(const char*))&lua_pause_timer;
	eq["resume_timer"] = (void(*)(const char*))&lua_resume_timer;
	eq["is_paused_timer"] = (bool(*)(const char*))&lua_is_paused_timer;
	eq["stop_all_timers"] =
	    sol::overload((void (*)(void)) & lua_stop_all_timers, (void (*)(Lua_ItemInst)) & lua_stop_all_timers,
			  (void (*)(Lua_Mob)) & lua_stop_all_timers, (void (*)(Lua_Encounter)) & lua_stop_all_timers);
	eq["depop"] = sol::overload((void (*)(void)) & lua_depop, (void (*)(int)) & lua_depop);
	eq["depop_with_timer"] =
	    sol::overload((void (*)(void)) & lua_depop_with_timer, (void (*)(int)) & lua_depop_with_timer);
	eq["depop_all"] = sol::overload((void (*)(void)) & lua_depop_all, (void (*)(int)) & lua_depop_all);
	eq["depop_zone"] = &lua_depop_zone;
	eq["repop_zone"] = &lua_repop_zone;
	eq["process_mobs_while_zone_empty"] = &lua_process_mobs_while_zone_empty;
	eq["is_disc_tome"] = &lua_is_disc_tome;
	eq["get_race_name"] = (std::string(*)(uint16))&lua_get_race_name;
	eq["get_spell_name"] = (std::string(*)(uint32))&lua_get_spell_name;
	eq["get_skill_name"] = (std::string(*)(int))&lua_get_skill_name;
	eq["safe_move"] = &lua_safe_move;
	eq["rain"] = &lua_rain;
	eq["snow"] = &lua_snow;
	eq["scribe_spells"] =
	    sol::overload((int (*)(int)) & lua_scribe_spells, (int (*)(int, int)) & lua_scribe_spells);
	eq["train_discs"] = sol::overload((int (*)(int)) & lua_train_discs, (int (*)(int, int)) & lua_train_discs);
	eq["set_sky"] = &lua_set_sky;
	eq["set_guild"] = &lua_set_guild;
	eq["create_guild"] = &lua_create_guild;
	eq["set_time"] = sol::overload((void (*)(int, int)) & lua_set_time, (void (*)(int, int, bool)) & lua_set_time);
	eq["signal"] = sol::overload((void (*)(int, int)) & lua_signal, (void (*)(int, int, int)) & lua_signal);
	eq["set_global"] = &lua_set_global;
	eq["target_global"] = &lua_target_global;
	eq["delete_global"] = &lua_delete_global;
	eq["start"] = &lua_start;
	eq["stop"] = &lua_stop;
	eq["pause"] = &lua_pause;
	eq["move_to"] = sol::overload((void (*)(float, float, float)) & lua_move_to,
				      (void (*)(float, float, float, float)) & lua_move_to,
				      (void (*)(float, float, float, float, bool)) & lua_move_to);
	eq["resume"] = &lua_path_resume;
	eq["set_next_hp_event"] = &lua_set_next_hp_event;
	eq["set_next_inc_hp_event"] = &lua_set_next_inc_hp_event;
	eq["respawn"] = &lua_respawn;
	eq["set_proximity"] =
	    sol::overload((void (*)(float, float, float, float)) & lua_set_proximity,
			  (void (*)(float, float, float, float, float, float)) & lua_set_proximity,
			  (void (*)(float, float, float, float, float, float, bool)) & lua_set_proximity);
	eq["clear_proximity"] = &lua_clear_proximity;
	eq["enable_proximity_say"] = &lua_enable_proximity_say;
	eq["disable_proximity_say"] = &lua_disable_proximity_say;
	eq["set_anim"] = &lua_set_anim;
	eq["spawn_condition"] = &lua_spawn_condition;
	eq["get_spawn_condition"] = &lua_get_spawn_condition;
	eq["toggle_spawn_event"] = &lua_toggle_spawn_event;
	eq["summon_buried_player_corpse"] = &lua_summon_buried_player_corpse;
	eq["summon_all_player_corpses"] = &lua_summon_all_player_corpses;
	eq["get_player_corpse_count"] = &lua_get_player_corpse_count;
	eq["get_player_corpse_count_by_zone_id"] = &lua_get_player_corpse_count_by_zone_id;
	eq["get_player_buried_corpse_count"] = &lua_get_player_buried_corpse_count;
	eq["bury_player_corpse"] = &lua_bury_player_corpse;
	eq["task_selector"] =
	    sol::overload((void (*)(sol::table)) & lua_task_selector, (void (*)(sol::table, bool)) & lua_task_selector);
	eq["task_set_selector"] =
	    sol::overload((void (*)(int)) & lua_task_set_selector, (void (*)(int, bool)) & lua_task_set_selector);
	eq["enable_task"] = &lua_enable_task;
	eq["disable_task"] = &lua_disable_task;
	eq["is_task_enabled"] = &lua_is_task_enabled;
	eq["is_task_active"] = &lua_is_task_active;
	eq["is_task_activity_active"] = &lua_is_task_activity_active;
	eq["get_task_activity_done_count"] = &lua_get_task_activity_done_count;
	eq["update_task_activity"] = &lua_update_task_activity;
	eq["reset_task_activity"] = &lua_reset_task_activity;
	eq["assign_task"] = &lua_assign_task;
	eq["fail_task"] = &lua_fail_task;
	eq["task_time_left"] = &lua_task_time_left;
	eq["is_task_completed"] = &lua_is_task_completed;
	eq["enabled_task_count"] = &lua_enabled_task_count;
	eq["first_task_in_set"] = &lua_first_task_in_set;
	eq["last_task_in_set"] = &lua_last_task_in_set;
	eq["next_task_in_set"] = &lua_next_task_in_set;
	eq["active_speak_task"] = &lua_active_speak_task;
	eq["active_speak_activity"] = &lua_active_speak_activity;
	eq["active_tasks_in_set"] = &lua_active_tasks_in_set;
	eq["completed_tasks_in_set"] = &lua_completed_tasks_in_set;
	eq["is_task_appropriate"] = &lua_is_task_appropriate;
	eq["get_task_name"] = (std::string(*)(uint32))&lua_get_task_name;
	eq["popup"] = &lua_popup;
	eq["clear_spawn_timers"] = &lua_clear_spawn_timers;
	eq["zone_emote"] = &lua_zone_emote;
	eq["world_emote"] = &lua_world_emote;
	eq["message"] = &lua_message;
	eq["whisper"] = &lua_whisper;
	eq["get_level"] = &lua_get_level;
	eq["create_ground_object"] =
	    sol::overload((void (*)(uint32, float, float, float, float)) & lua_create_ground_object,
			  (void (*)(uint32, float, float, float, float, uint32)) & lua_create_ground_object);
	eq["create_ground_object_from_model"] = sol::overload(
	    (void (*)(const char *, float, float, float, float)) & lua_create_ground_object_from_model,
	    (void (*)(const char *, float, float, float, float, int)) & lua_create_ground_object_from_model,
	    (void (*)(const char *, float, float, float, float, int, uint32)) & lua_create_ground_object_from_model);
	eq["create_door"] = &lua_create_door;
	eq["modify_npc_stat"] = &lua_modify_npc_stat;
	eq["collect_items"] = &lua_collect_items;
	eq["count_item"] = &lua_count_item;
	eq["remove_item"] =
	    sol::overload((void (*)(uint32)) & lua_remove_item, (void (*)(uint32, uint32)) & lua_remove_item);
	eq["update_spawn_timer"] = &lua_update_spawn_timer;
	eq["merchant_set_item"] = sol::overload((void (*)(uint32, uint32)) & lua_merchant_set_item,
						(void (*)(uint32, uint32, uint32)) & lua_merchant_set_item);
	eq["merchant_count_item"] = &lua_merchant_count_item;
	eq["item_link"] = &lua_item_link;
	eq["get_item_name"] = (std::string(*)(uint32))&lua_get_item_name;
	eq["say_link"] = sol::overload((std::string(*)(const char *, bool, const char *)) & lua_say_link,
				       (std::string(*)(const char *, bool)) & lua_say_link,
				       (std::string(*)(const char *)) & lua_say_link);
	eq["set_rule"] = (void(*)(std::string, std::string))&lua_set_rule;
	eq["get_rule"] = (std::string(*)(std::string))&lua_get_rule;
	eq["get_data"] = (std::string(*)(std::string))&lua_get_data;
	eq["get_data_expires"] = (std::string(*)(std::string))&lua_get_data_expires;
	eq["set_data"] = sol::overload((void (*)(std::string, std::string)) & lua_set_data,
				       (void (*)(std::string, std::string, std::string)) & lua_set_data);
	eq["delete_data"] = (bool(*)(std::string))&lua_delete_data;
	eq["get_char_name_by_id"] = &lua_get_char_name_by_id;
	eq["get_char_id_by_name"] = (uint32(*)(const char*))&lua_get_char_id_by_name;
	eq["get_class_name"] = sol::overload((std::string(*)(uint8)) & lua_get_class_name,
					     (std::string(*)(uint8, uint8)) & lua_get_class_name);
	eq["get_clean_npc_name_by_id"] = &lua_get_clean_npc_name_by_id;
	eq["get_currency_id"] = &lua_get_currency_id;
	eq["get_currency_item_id"] = &lua_get_currency_item_id;
	eq["get_guild_name_by_id"] = &lua_get_guild_name_by_id;
	eq["get_guild_id_by_char_id"] = &lua_get_guild_id_by_char_id;
	eq["get_group_id_by_char_id"] = &lua_get_group_id_by_char_id;
	eq["get_npc_name_by_id"] = &lua_get_npc_name_by_id;
	eq["get_raid_id_by_char_id"] = &lua_get_raid_id_by_char_id;
	eq["create_instance"] = &lua_create_instance;
	eq["destroy_instance"] = &lua_destroy_instance;
	eq["update_instance_timer"] = &lua_update_instance_timer;
	eq["get_instance_id"] = &lua_get_instance_id;
	eq["get_instance_id_by_char_id"] = &lua_get_instance_id_by_char_id;
	eq["get_instance_timer"] = &lua_get_instance_timer;
	eq["get_instance_timer_by_id"] = &lua_get_instance_timer_by_id;
	eq["get_characters_in_instance"] = &lua_get_characters_in_instance;
	eq["assign_to_instance"] = &lua_assign_to_instance;
	eq["assign_to_instance_by_char_id"] = &lua_assign_to_instance_by_char_id;
	eq["assign_group_to_instance"] = &lua_assign_group_to_instance;
	eq["assign_raid_to_instance"] = &lua_assign_raid_to_instance;
	eq["remove_from_instance"] = &lua_remove_from_instance;
	eq["remove_from_instance_by_char_id"] = &lua_remove_from_instance_by_char_id;
	eq["check_instance_by_char_id"] = (bool(*)(uint16, uint32))&lua_check_instance_by_char_id;
	eq["remove_all_from_instance"] = &lua_remove_all_from_instance;
	eq["flag_instance_by_group_leader"] = &lua_flag_instance_by_group_leader;
	eq["flag_instance_by_raid_leader"] = &lua_flag_instance_by_raid_leader;
	eq["fly_mode"] = &lua_fly_mode;
	eq["faction_value"] = &lua_faction_value;
	eq["check_title"] = &lua_check_title;
	eq["enable_title"] = &lua_enable_title;
	eq["remove_title"] = &lua_remove_title;
	eq["wear_change"] = &lua_wear_change;
	eq["voice_tell"] = &lua_voice_tell;
	eq["send_mail"] = &lua_send_mail;
	eq["get_qglobals"] = sol::overload((sol::table(*)(sol::this_state, Lua_NPC, Lua_Client)) & lua_get_qglobals,
					   (sol::table(*)(sol::this_state, Lua_Client)) & lua_get_qglobals,
					   (sol::table(*)(sol::this_state, Lua_NPC)) & lua_get_qglobals,
					   (sol::table(*)(sol::this_state)) & lua_get_qglobals);
	eq["get_entity_list"] = &lua_get_entity_list;
	eq["zone"] = &lua_zone;
	eq["zone_group"] = &lua_zone_group;
	eq["zone_raid"] = &lua_zone_raid;
	eq["get_zone_id"] = &lua_get_zone_id;
	eq["get_zone_id_by_name"] = &lua_get_zone_id_by_name;
	eq["get_zone_long_name"] = &lua_get_zone_long_name;
	eq["get_zone_long_name_by_name"] = &lua_get_zone_long_name_by_name;
	eq["get_zone_long_name_by_id"] = &lua_get_zone_long_name_by_id;
	eq["get_zone_short_name"] = &lua_get_zone_short_name;
	eq["get_zone_short_name_by_id"] = &lua_get_zone_short_name_by_id;
	eq["get_zone_instance_id"] = &lua_get_zone_instance_id;
	eq["get_zone_instance_version"] = &lua_get_zone_instance_version;
	eq["get_zone_weather"] = &lua_get_zone_weather;
	eq["get_zone_time"] = &lua_get_zone_time;
	eq["add_area"] = &lua_add_area;
	eq["remove_area"] = &lua_remove_area;
	eq["clear_areas"] = &lua_clear_areas;
	eq["add_spawn_point"] = &lua_add_spawn_point;
	eq["remove_spawn_point"] = &lua_remove_spawn_point;
	eq["attack"] = &lua_attack;
	eq["attack_npc"] = &lua_attack_npc;
	eq["attack_npc_type"] = &lua_attack_npc_type;
	eq["follow"] = sol::overload((void (*)(int)) & lua_follow, (void (*)(int, int)) & lua_follow);
	eq["stop_follow"] = &lua_stop_follow;
	eq["get_initiator"] = &lua_get_initiator;
	eq["get_owner"] = &lua_get_owner;
	eq["get_quest_item"] = &lua_get_quest_item;
	eq["get_quest_spell"] = &lua_get_quest_spell;
	eq["get_encounter"] = &lua_get_encounter;
	eq["map_opcodes"] = &lua_map_opcodes;
	eq["clear_opcode"] = &lua_clear_opcode;
	eq["enable_recipe"] = &lua_enable_recipe;
	eq["disable_recipe"] = &lua_disable_recipe;
	eq["clear_npctype_cache"] = &lua_clear_npctype_cache;
	eq["reloadzonestaticdata"] = &lua_reloadzonestaticdata;
	eq["update_zone_header"] = &lua_update_zone_header;
	eq["clock"] = &lua_clock;
	eq["create_npc"] = &lua_create_npc;
	eq["log"] = (void(*)(int, std::string))&lua_log;
	eq["debug"] = sol::overload((void (*)(std::string)) & lua_debug, (void (*)(std::string, int)) & lua_debug);
	eq["log_combat"] = (void(*)(std::string))&lua_log_combat;
	eq["seconds_to_time"] = &lua_seconds_to_time;
	eq["get_hex_color_code"] = &lua_get_hex_color_code;
	eq["get_aa_exp_modifier_by_char_id"] =
	    sol::overload((double (*)(uint32, uint32)) & lua_get_aa_exp_modifier_by_char_id,
			  (double (*)(uint32, uint32, int16)) & lua_get_aa_exp_modifier_by_char_id);
	eq["get_exp_modifier_by_char_id"] =
	    sol::overload((double (*)(uint32, uint32)) & lua_get_exp_modifier_by_char_id,
			  (double (*)(uint32, uint32, int16)) & lua_get_exp_modifier_by_char_id);
	eq["set_aa_exp_modifier_by_char_id"] =
	    sol::overload((void (*)(uint32, uint32, double)) & lua_set_aa_exp_modifier_by_char_id,
			  (void (*)(uint32, uint32, double, int16)) & lua_set_aa_exp_modifier_by_char_id);
	eq["set_exp_modifier_by_char_id"] =
	    sol::overload((void (*)(uint32, uint32, double)) & lua_set_exp_modifier_by_char_id,
			  (void (*)(uint32, uint32, double, int16)) & lua_set_exp_modifier_by_char_id);
	eq["add_ldon_loss"] = &lua_add_ldon_loss;
	eq["add_ldon_points"] = &lua_add_ldon_points;
	eq["add_ldon_win"] = &lua_add_ldon_win;
	eq["get_gender_name"] = &lua_get_gender_name;
	eq["get_deity_name"] = &lua_get_deity_name;
	eq["get_inventory_slot_name"] = &lua_get_inventory_slot_name;
	eq["rename"] = &lua_rename;
	eq["get_data_remaining"] = &lua_get_data_remaining;
	eq["get_item_stat"] = &lua_get_item_stat;
	eq["get_spell_stat"] = sol::overload((int (*)(uint32, std::string)) & lua_get_spell_stat,
					     (int (*)(uint32, std::string, uint8)) & lua_get_spell_stat);
	eq["is_npc_spawned"] = &lua_is_npc_spawned;
	eq["count_spawned_npcs"] = &lua_count_spawned_npcs;
	eq["get_spell"] = &lua_get_spell;
	eq["get_ldon_theme_name"] = &lua_get_ldon_theme_name;
	eq["get_faction_name"] = &lua_get_faction_name;
	eq["get_language_name"] = &lua_get_language_name;
	eq["get_body_type_name"] = &lua_get_body_type_name;
	eq["get_consider_level_name"] = &lua_get_consider_level_name;
	eq["get_environmental_damage_name"] = &lua_get_environmental_damage_name;
	eq["commify"] = &lua_commify;
	eq["check_name_filter"] = &lua_check_name_filter;
	eq["discord_send"] = &lua_discord_send;
	eq["track_npc"] = &lua_track_npc;
	eq["get_recipe_made_count"] = &lua_get_recipe_made_count;
	eq["get_recipe_name"] = &lua_get_recipe_name;
	eq["has_recipe_learned"] = &lua_has_recipe_learned;
	eq["is_raining"] = &lua_is_raining;
	eq["is_snowing"] = &lua_is_snowing;

	/*
		Cross Zone
	*/
	eq["cross_zone_add_ldon_loss_by_char_id"] = &lua_cross_zone_add_ldon_loss_by_char_id;
	eq["cross_zone_add_ldon_loss_by_group_id"] = &lua_cross_zone_add_ldon_loss_by_group_id;
	eq["cross_zone_add_ldon_loss_by_raid_id"] = &lua_cross_zone_add_ldon_loss_by_raid_id;
	eq["cross_zone_add_ldon_loss_by_guild_id"] = &lua_cross_zone_add_ldon_loss_by_guild_id;
	eq["cross_zone_add_ldon_loss_by_expedition_id"] = &lua_cross_zone_add_ldon_loss_by_expedition_id;
	eq["cross_zone_add_ldon_loss_by_client_name"] = &lua_cross_zone_add_ldon_loss_by_client_name;
	eq["cross_zone_add_ldon_points_by_char_id"] = &lua_cross_zone_add_ldon_points_by_char_id;
	eq["cross_zone_add_ldon_points_by_group_id"] = &lua_cross_zone_add_ldon_points_by_group_id;
	eq["cross_zone_add_ldon_points_by_raid_id"] = &lua_cross_zone_add_ldon_points_by_raid_id;
	eq["cross_zone_add_ldon_points_by_guild_id"] = &lua_cross_zone_add_ldon_points_by_guild_id;
	eq["cross_zone_add_ldon_points_by_expedition_id"] = &lua_cross_zone_add_ldon_points_by_expedition_id;
	eq["cross_zone_add_ldon_points_by_client_name"] = &lua_cross_zone_add_ldon_points_by_client_name;
	eq["cross_zone_add_ldon_win_by_char_id"] = &lua_cross_zone_add_ldon_win_by_char_id;
	eq["cross_zone_add_ldon_win_by_group_id"] = &lua_cross_zone_add_ldon_win_by_group_id;
	eq["cross_zone_add_ldon_win_by_raid_id"] = &lua_cross_zone_add_ldon_win_by_raid_id;
	eq["cross_zone_add_ldon_win_by_guild_id"] = &lua_cross_zone_add_ldon_win_by_guild_id;
	eq["cross_zone_add_ldon_win_by_expedition_id"] = &lua_cross_zone_add_ldon_win_by_expedition_id;
	eq["cross_zone_add_ldon_win_by_client_name"] = &lua_cross_zone_add_ldon_win_by_client_name;
	eq["cross_zone_assign_task_by_char_id"] =
	    sol::overload((void (*)(int, uint32)) & lua_cross_zone_assign_task_by_char_id,
			  (void (*)(int, uint32, bool)) & lua_cross_zone_assign_task_by_char_id);
	eq["cross_zone_assign_task_by_group_id"] =
	    sol::overload((void (*)(int, uint32)) & lua_cross_zone_assign_task_by_group_id,
			  (void (*)(int, uint32, bool)) & lua_cross_zone_assign_task_by_group_id);
	eq["cross_zone_assign_task_by_raid_id"] =
	    sol::overload((void (*)(int, uint32)) & lua_cross_zone_assign_task_by_raid_id,
			  (void (*)(int, uint32, bool)) & lua_cross_zone_assign_task_by_raid_id);
	eq["cross_zone_assign_task_by_guild_id"] =
	    sol::overload((void (*)(int, uint32)) & lua_cross_zone_assign_task_by_guild_id,
			  (void (*)(int, uint32, bool)) & lua_cross_zone_assign_task_by_guild_id);
	eq["cross_zone_assign_task_by_expedition_id"] =
	    sol::overload((void (*)(uint32, uint32)) & lua_cross_zone_assign_task_by_expedition_id,
			  (void (*)(uint32, uint32, bool)) & lua_cross_zone_assign_task_by_expedition_id);
	eq["cross_zone_assign_task_by_client_name"] =
	    sol::overload((void (*)(const char *, uint32)) & lua_cross_zone_assign_task_by_client_name,
			  (void (*)(const char *, uint32, bool)) & lua_cross_zone_assign_task_by_client_name);
	eq["cross_zone_cast_spell_by_char_id"] = &lua_cross_zone_cast_spell_by_char_id;
	eq["cross_zone_cast_spell_by_group_id"] = &lua_cross_zone_cast_spell_by_group_id;
	eq["cross_zone_cast_spell_by_raid_id"] = &lua_cross_zone_cast_spell_by_raid_id;
	eq["cross_zone_cast_spell_by_guild_id"] = &lua_cross_zone_cast_spell_by_guild_id;
	eq["cross_zone_cast_spell_by_expedition_id"] = &lua_cross_zone_cast_spell_by_expedition_id;
	eq["cross_zone_cast_spell_by_client_name"] = &lua_cross_zone_cast_spell_by_client_name;
	eq["cross_zone_dialogue_window_by_char_id"] = &lua_cross_zone_dialogue_window_by_char_id;
	eq["cross_zone_dialogue_window_by_group_id"] = &lua_cross_zone_dialogue_window_by_group_id;
	eq["cross_zone_dialogue_window_by_raid_id"] = &lua_cross_zone_dialogue_window_by_raid_id;
	eq["cross_zone_dialogue_window_by_guild_id"] = &lua_cross_zone_dialogue_window_by_guild_id;
	eq["cross_zone_dialogue_window_by_expedition_id"] = &lua_cross_zone_dialogue_window_by_expedition_id;
	eq["cross_zone_dialogue_window_by_client_name"] = &lua_cross_zone_dialogue_window_by_client_name;
	eq["cross_zone_disable_task_by_char_id"] = &lua_cross_zone_disable_task_by_char_id;
	eq["cross_zone_disable_task_by_group_id"] = &lua_cross_zone_disable_task_by_group_id;
	eq["cross_zone_disable_task_by_raid_id"] = &lua_cross_zone_disable_task_by_raid_id;
	eq["cross_zone_disable_task_by_guild_id"] = &lua_cross_zone_disable_task_by_guild_id;
	eq["cross_zone_disable_task_by_expedition_id"] = &lua_cross_zone_disable_task_by_expedition_id;
	eq["cross_zone_disable_task_by_client_name"] = &lua_cross_zone_disable_task_by_client_name;
	eq["cross_zone_enable_task_by_char_id"] = &lua_cross_zone_enable_task_by_char_id;
	eq["cross_zone_enable_task_by_group_id"] = &lua_cross_zone_enable_task_by_group_id;
	eq["cross_zone_enable_task_by_raid_id"] = &lua_cross_zone_enable_task_by_raid_id;
	eq["cross_zone_enable_task_by_guild_id"] = &lua_cross_zone_enable_task_by_guild_id;
	eq["cross_zone_enable_task_by_expedition_id"] = &lua_cross_zone_enable_task_by_expedition_id;
	eq["cross_zone_enable_task_by_client_name"] = &lua_cross_zone_enable_task_by_client_name;
	eq["cross_zone_fail_task_by_char_id"] = &lua_cross_zone_fail_task_by_char_id;
	eq["cross_zone_fail_task_by_group_id"] = &lua_cross_zone_fail_task_by_group_id;
	eq["cross_zone_fail_task_by_raid_id"] = &lua_cross_zone_fail_task_by_raid_id;
	eq["cross_zone_fail_task_by_guild_id"] = &lua_cross_zone_fail_task_by_guild_id;
	eq["cross_zone_fail_task_by_expedition_id"] = &lua_cross_zone_fail_task_by_expedition_id;
	eq["cross_zone_fail_task_by_client_name"] = &lua_cross_zone_fail_task_by_client_name;
	eq["cross_zone_marquee_by_char_id"] = &lua_cross_zone_marquee_by_char_id;
	eq["cross_zone_marquee_by_group_id"] = &lua_cross_zone_marquee_by_group_id;
	eq["cross_zone_marquee_by_raid_id"] = &lua_cross_zone_marquee_by_raid_id;
	eq["cross_zone_marquee_by_guild_id"] = &lua_cross_zone_marquee_by_guild_id;
	eq["cross_zone_marquee_by_expedition_id"] = &lua_cross_zone_marquee_by_expedition_id;
	eq["cross_zone_marquee_by_client_name"] = &lua_cross_zone_marquee_by_client_name;
	eq["cross_zone_message_player_by_char_id"] = &lua_cross_zone_message_player_by_char_id;
	eq["cross_zone_message_player_by_group_id"] = &lua_cross_zone_message_player_by_group_id;
	eq["cross_zone_message_player_by_raid_id"] = &lua_cross_zone_message_player_by_raid_id;
	eq["cross_zone_message_player_by_guild_id"] = &lua_cross_zone_message_player_by_guild_id;
	eq["cross_zone_message_player_by_expedition_id"] = &lua_cross_zone_message_player_by_expedition_id;
	eq["cross_zone_message_player_by_name"] = &lua_cross_zone_message_player_by_name;
	eq["cross_zone_move_player_by_char_id"] = &lua_cross_zone_move_player_by_char_id;
	eq["cross_zone_move_player_by_group_id"] = &lua_cross_zone_move_player_by_group_id;
	eq["cross_zone_move_player_by_raid_id"] = &lua_cross_zone_move_player_by_raid_id;
	eq["cross_zone_move_player_by_guild_id"] = &lua_cross_zone_move_player_by_guild_id;
	eq["cross_zone_move_player_by_expedition_id"] = &lua_cross_zone_move_player_by_expedition_id;
	eq["cross_zone_move_player_by_client_name"] = &lua_cross_zone_move_player_by_client_name;
	eq["cross_zone_move_instance_by_char_id"] = &lua_cross_zone_move_instance_by_char_id;
	eq["cross_zone_move_instance_by_group_id"] = &lua_cross_zone_move_instance_by_group_id;
	eq["cross_zone_move_instance_by_raid_id"] = &lua_cross_zone_move_instance_by_raid_id;
	eq["cross_zone_move_instance_by_guild_id"] = &lua_cross_zone_move_instance_by_guild_id;
	eq["cross_zone_move_instance_by_expedition_id"] = &lua_cross_zone_move_instance_by_expedition_id;
	eq["cross_zone_move_instance_by_client_name"] = &lua_cross_zone_move_instance_by_client_name;
	eq["cross_zone_remove_ldon_loss_by_char_id"] = &lua_cross_zone_remove_ldon_loss_by_char_id;
	eq["cross_zone_remove_ldon_loss_by_group_id"] = &lua_cross_zone_remove_ldon_loss_by_group_id;
	eq["cross_zone_remove_ldon_loss_by_raid_id"] = &lua_cross_zone_remove_ldon_loss_by_raid_id;
	eq["cross_zone_remove_ldon_loss_by_guild_id"] = &lua_cross_zone_remove_ldon_loss_by_guild_id;
	eq["cross_zone_remove_ldon_loss_by_expedition_id"] = &lua_cross_zone_remove_ldon_loss_by_expedition_id;
	eq["cross_zone_remove_ldon_loss_by_client_name"] = &lua_cross_zone_remove_ldon_loss_by_client_name;
	eq["cross_zone_remove_ldon_win_by_char_id"] = &lua_cross_zone_remove_ldon_win_by_char_id;
	eq["cross_zone_remove_ldon_win_by_group_id"] = &lua_cross_zone_remove_ldon_win_by_group_id;
	eq["cross_zone_remove_ldon_win_by_raid_id"] = &lua_cross_zone_remove_ldon_win_by_raid_id;
	eq["cross_zone_remove_ldon_win_by_guild_id"] = &lua_cross_zone_remove_ldon_win_by_guild_id;
	eq["cross_zone_remove_ldon_win_by_expedition_id"] = &lua_cross_zone_remove_ldon_win_by_expedition_id;
	eq["cross_zone_remove_ldon_win_by_client_name"] = &lua_cross_zone_remove_ldon_win_by_client_name;
	eq["cross_zone_remove_spell_by_char_id"] = &lua_cross_zone_remove_spell_by_char_id;
	eq["cross_zone_remove_spell_by_group_id"] = &lua_cross_zone_remove_spell_by_group_id;
	eq["cross_zone_remove_spell_by_raid_id"] = &lua_cross_zone_remove_spell_by_raid_id;
	eq["cross_zone_remove_spell_by_guild_id"] = &lua_cross_zone_remove_spell_by_guild_id;
	eq["cross_zone_remove_spell_by_expedition_id"] = &lua_cross_zone_remove_spell_by_expedition_id;
	eq["cross_zone_remove_spell_by_client_name"] = &lua_cross_zone_remove_spell_by_client_name;
	eq["cross_zone_remove_task_by_char_id"] = &lua_cross_zone_remove_task_by_char_id;
	eq["cross_zone_remove_task_by_group_id"] = &lua_cross_zone_remove_task_by_group_id;
	eq["cross_zone_remove_task_by_raid_id"] = &lua_cross_zone_remove_task_by_raid_id;
	eq["cross_zone_remove_task_by_guild_id"] = &lua_cross_zone_remove_task_by_guild_id;
	eq["cross_zone_remove_task_by_expedition_id"] = &lua_cross_zone_remove_task_by_expedition_id;
	eq["cross_zone_remove_task_by_client_name"] = &lua_cross_zone_remove_task_by_client_name;
	eq["cross_zone_reset_activity_by_char_id"] = &lua_cross_zone_reset_activity_by_char_id;
	eq["cross_zone_reset_activity_by_group_id"] = &lua_cross_zone_reset_activity_by_group_id;
	eq["cross_zone_reset_activity_by_raid_id"] = &lua_cross_zone_reset_activity_by_raid_id;
	eq["cross_zone_reset_activity_by_guild_id"] = &lua_cross_zone_reset_activity_by_guild_id;
	eq["cross_zone_reset_activity_by_expedition_id"] = &lua_cross_zone_reset_activity_by_expedition_id;
	eq["cross_zone_reset_activity_by_client_name"] = &lua_cross_zone_reset_activity_by_client_name;
	eq["cross_zone_set_entity_variable_by_client_name"] = &lua_cross_zone_set_entity_variable_by_client_name;
	eq["cross_zone_set_entity_variable_by_group_id"] = &lua_cross_zone_set_entity_variable_by_group_id;
	eq["cross_zone_set_entity_variable_by_raid_id"] = &lua_cross_zone_set_entity_variable_by_raid_id;
	eq["cross_zone_set_entity_variable_by_guild_id"] = &lua_cross_zone_set_entity_variable_by_guild_id;
	eq["cross_zone_set_entity_variable_by_expedition_id"] = &lua_cross_zone_set_entity_variable_by_expedition_id;
	eq["cross_zone_set_entity_variable_by_client_name"] = &lua_cross_zone_set_entity_variable_by_client_name;
	eq["cross_zone_signal_client_by_char_id"] = &lua_cross_zone_signal_client_by_char_id;
	eq["cross_zone_signal_client_by_group_id"] = &lua_cross_zone_signal_client_by_group_id;
	eq["cross_zone_signal_client_by_raid_id"] = &lua_cross_zone_signal_client_by_raid_id;
	eq["cross_zone_signal_client_by_guild_id"] = &lua_cross_zone_signal_client_by_guild_id;
	eq["cross_zone_signal_client_by_expedition_id"] = &lua_cross_zone_signal_client_by_expedition_id;
	eq["cross_zone_signal_client_by_name"] = &lua_cross_zone_signal_client_by_name;
	eq["cross_zone_signal_npc_by_npctype_id"] = &lua_cross_zone_signal_npc_by_npctype_id;
	eq["cross_zone_update_activity_by_char_id"] =
	    sol::overload((void (*)(int, uint32, int)) & lua_cross_zone_update_activity_by_char_id,
			  (void (*)(int, uint32, int, int)) & lua_cross_zone_update_activity_by_char_id);
	eq["cross_zone_update_activity_by_group_id"] =
	    sol::overload((void (*)(int, uint32, int)) & lua_cross_zone_update_activity_by_group_id,
			  (void (*)(int, uint32, int, int)) & lua_cross_zone_update_activity_by_group_id);
	eq["cross_zone_update_activity_by_raid_id"] =
	    sol::overload((void (*)(int, uint32, int)) & lua_cross_zone_update_activity_by_raid_id,
			  (void (*)(int, uint32, int, int)) & lua_cross_zone_update_activity_by_raid_id);
	eq["cross_zone_update_activity_by_guild_id"] =
	    sol::overload((void (*)(int, uint32, int)) & lua_cross_zone_update_activity_by_guild_id,
			  (void (*)(int, uint32, int, int)) & lua_cross_zone_update_activity_by_guild_id);
	eq["cross_zone_update_activity_by_expedition_id"] =
	    sol::overload((void (*)(uint32, uint32, int)) & lua_cross_zone_update_activity_by_expedition_id,
			  (void (*)(uint32, uint32, int, int)) & lua_cross_zone_update_activity_by_expedition_id);
	eq["cross_zone_update_activity_by_client_name"] =
	    sol::overload((void (*)(const char *, uint32, int)) & lua_cross_zone_update_activity_by_client_name,
			  (void (*)(const char *, uint32, int, int)) & lua_cross_zone_update_activity_by_client_name);

	/*
		World Wide
	*/
	eq["world_wide_add_ldon_loss"] = sol::overload((void (*)(uint32)) & lua_world_wide_add_ldon_loss,
						       (void (*)(uint32, uint8)) & lua_world_wide_add_ldon_loss,
						       (void (*)(uint32, uint8, uint8)) & lua_world_wide_add_ldon_loss);
	eq["world_wide_add_ldon_points"] =
	    sol::overload((void (*)(uint32, int)) & lua_world_wide_add_ldon_points,
			  (void (*)(uint32, int, uint8)) & lua_world_wide_add_ldon_points,
			  (void (*)(uint32, int, uint8, uint8)) & lua_world_wide_add_ldon_points);
	eq["world_wide_add_ldon_loss"] = sol::overload((void (*)(uint32)) & lua_world_wide_add_ldon_win,
						       (void (*)(uint32, uint8)) & lua_world_wide_add_ldon_win,
						       (void (*)(uint32, uint8, uint8)) & lua_world_wide_add_ldon_win);
	eq["world_wide_assign_task"] = sol::overload(
	    (void (*)(uint32)) & lua_world_wide_assign_task, (void (*)(uint32, bool)) & lua_world_wide_assign_task,
	    (void (*)(uint32, bool, uint8)) & lua_world_wide_assign_task,
	    (void (*)(uint32, bool, uint8, uint8)) & lua_world_wide_assign_task);
	eq["world_wide_cast_spell"] = sol::overload((void (*)(uint32)) & lua_world_wide_cast_spell,
						    (void (*)(uint32, uint8)) & lua_world_wide_cast_spell,
						    (void (*)(uint32, uint8, uint8)) & lua_world_wide_cast_spell);
	eq["world_wide_dialogue_window"] =
	    sol::overload((void (*)(const char *)) & lua_world_wide_dialogue_window,
			  (void (*)(const char *, uint8)) & lua_world_wide_dialogue_window,
			  (void (*)(const char *, uint8, uint8)) & lua_world_wide_dialogue_window);
	eq["world_wide_disable_task"] = sol::overload((void (*)(uint32)) & lua_world_wide_disable_task,
						      (void (*)(uint32, uint8)) & lua_world_wide_disable_task,
						      (void (*)(uint32, uint8, uint8)) & lua_world_wide_disable_task);
	eq["world_wide_enable_task"] = sol::overload((void (*)(uint32)) & lua_world_wide_enable_task,
						     (void (*)(uint32, uint8)) & lua_world_wide_enable_task,
						     (void (*)(uint32, uint8, uint8)) & lua_world_wide_enable_task);
	eq["world_wide_fail_task"] = sol::overload((void (*)(uint32)) & lua_world_wide_fail_task,
						   (void (*)(uint32, uint8)) & lua_world_wide_fail_task,
						   (void (*)(uint32, uint8, uint8)) & lua_world_wide_fail_task);
	eq["world_wide_marquee"] = sol::overload(
	    (void (*)(uint32, uint32, uint32, uint32, uint32, const char *)) & lua_world_wide_marquee,
	    (void (*)(uint32, uint32, uint32, uint32, uint32, const char *, uint8)) & lua_world_wide_marquee,
	    (void (*)(uint32, uint32, uint32, uint32, uint32, const char *, uint8, uint8)) & lua_world_wide_marquee);
	eq["world_wide_message"] =
	    sol::overload((void (*)(uint32, const char *)) & lua_world_wide_message,
			  (void (*)(uint32, const char *, uint8)) & lua_world_wide_message,
			  (void (*)(uint32, const char *, uint8, uint8)) & lua_world_wide_message);
	eq["world_wide_move"] = sol::overload((void (*)(const char *)) & lua_world_wide_move,
					      (void (*)(const char *, uint8)) & lua_world_wide_move,
					      (void (*)(const char *, uint8, uint8)) & lua_world_wide_move);
	eq["world_wide_move_instance"] = sol::overload((void (*)(uint16)) & lua_world_wide_move_instance,
						       (void (*)(uint16, uint8)) & lua_world_wide_move_instance,
						       (void (*)(uint16, uint8, uint8)) & lua_world_wide_move_instance);
	eq["world_wide_remove_spell"] = sol::overload((void (*)(uint32)) & lua_world_wide_remove_spell,
						      (void (*)(uint32, uint8)) & lua_world_wide_remove_spell,
						      (void (*)(uint32, uint8, uint8)) & lua_world_wide_remove_spell);
	eq["world_wide_remove_task"] = sol::overload((void (*)(uint32)) & lua_world_wide_remove_task,
						     (void (*)(uint32, uint8)) & lua_world_wide_remove_task,
						     (void (*)(uint32, uint8, uint8)) & lua_world_wide_remove_task);
	eq["world_wide_reset_activity"] =
	    sol::overload((void (*)(uint32, int)) & lua_world_wide_reset_activity,
			  (void (*)(uint32, int, uint8)) & lua_world_wide_reset_activity,
			  (void (*)(uint32, int, uint8, uint8)) & lua_world_wide_reset_activity);
	eq["world_wide_set_entity_variable_client"] = sol::overload(
	    (void (*)(const char *, const char *)) & lua_world_wide_set_entity_variable_client,
	    (void (*)(const char *, const char *, uint8)) & lua_world_wide_set_entity_variable_client,
	    (void (*)(const char *, const char *, uint8, uint8)) & lua_world_wide_set_entity_variable_client);
	eq["world_wide_set_entity_variable_npc"] = &lua_world_wide_set_entity_variable_npc;
	eq["world_wide_signal_client"] = sol::overload((void (*)(uint32)) & lua_world_wide_signal_client,
						       (void (*)(uint32, uint8)) & lua_world_wide_signal_client,
						       (void (*)(uint32, uint8, uint8)) & lua_world_wide_signal_client);
	eq["world_wide_signal_npc"] = &lua_world_wide_signal_npc;
	eq["world_wide_update_activity"] =
	    sol::overload((void (*)(uint32, int)) & lua_world_wide_update_activity,
			  (void (*)(uint32, int, int)) & lua_world_wide_update_activity,
			  (void (*)(uint32, int, int, uint8)) & lua_world_wide_update_activity,
			  (void (*)(uint32, int, int, uint8, uint8)) & lua_world_wide_update_activity);

	/**
	 * Expansions
	 */
	eq["is_classic_enabled"] = &lua_is_classic_enabled;
	eq["is_the_ruins_of_kunark_enabled"] = &lua_is_the_ruins_of_kunark_enabled;
	eq["is_the_scars_of_velious_enabled"] = &lua_is_the_scars_of_velious_enabled;
	eq["is_the_shadows_of_luclin_enabled"] = &lua_is_the_shadows_of_luclin_enabled;
	eq["is_the_planes_of_power_enabled"] = &lua_is_the_planes_of_power_enabled;
	eq["is_the_legacy_of_ykesha_enabled"] = &lua_is_the_legacy_of_ykesha_enabled;
	eq["is_lost_dungeons_of_norrath_enabled"] = &lua_is_lost_dungeons_of_norrath_enabled;
	eq["is_gates_of_discord_enabled"] = &lua_is_gates_of_discord_enabled;
	eq["is_omens_of_war_enabled"] = &lua_is_omens_of_war_enabled;
	eq["is_dragons_of_norrath_enabled"] = &lua_is_dragons_of_norrath_enabled;
	eq["is_depths_of_darkhollow_enabled"] = &lua_is_depths_of_darkhollow_enabled;
	eq["is_prophecy_of_ro_enabled"] = &lua_is_prophecy_of_ro_enabled;
	eq["is_the_serpents_spine_enabled"] = &lua_is_the_serpents_spine_enabled;
	eq["is_the_buried_sea_enabled"] = &lua_is_the_buried_sea_enabled;
	eq["is_secrets_of_faydwer_enabled"] = &lua_is_secrets_of_faydwer_enabled;
	eq["is_seeds_of_destruction_enabled"] = &lua_is_seeds_of_destruction_enabled;
	eq["is_underfoot_enabled"] = &lua_is_underfoot_enabled;
	eq["is_house_of_thule_enabled"] = &lua_is_house_of_thule_enabled;
	eq["is_veil_of_alaris_enabled"] = &lua_is_veil_of_alaris_enabled;
	eq["is_rain_of_fear_enabled"] = &lua_is_rain_of_fear_enabled;
	eq["is_call_of_the_forsaken_enabled"] = &lua_is_call_of_the_forsaken_enabled;
	eq["is_the_darkened_sea_enabled"] = &lua_is_the_darkened_sea_enabled;
	eq["is_the_broken_mirror_enabled"] = &lua_is_the_broken_mirror_enabled;
	eq["is_empires_of_kunark_enabled"] = &lua_is_empires_of_kunark_enabled;
	eq["is_ring_of_scale_enabled"] = &lua_is_ring_of_scale_enabled;
	eq["is_the_burning_lands_enabled"] = &lua_is_the_burning_lands_enabled;
	eq["is_torment_of_velious_enabled"] = &lua_is_torment_of_velious_enabled;
	eq["is_current_expansion_classic"] = &lua_is_current_expansion_classic;
	eq["is_current_expansion_the_ruins_of_kunark"] = &lua_is_current_expansion_the_ruins_of_kunark;
	eq["is_current_expansion_the_scars_of_velious"] = &lua_is_current_expansion_the_scars_of_velious;
	eq["is_current_expansion_the_shadows_of_luclin"] = &lua_is_current_expansion_the_shadows_of_luclin;
	eq["is_current_expansion_the_planes_of_power"] = &lua_is_current_expansion_the_planes_of_power;
	eq["is_current_expansion_the_legacy_of_ykesha"] = &lua_is_current_expansion_the_legacy_of_ykesha;
	eq["is_current_expansion_lost_dungeons_of_norrath"] = &lua_is_current_expansion_lost_dungeons_of_norrath;
	eq["is_current_expansion_gates_of_discord"] = &lua_is_current_expansion_gates_of_discord;
	eq["is_current_expansion_omens_of_war"] = &lua_is_current_expansion_omens_of_war;
	eq["is_current_expansion_dragons_of_norrath"] = &lua_is_current_expansion_dragons_of_norrath;
	eq["is_current_expansion_depths_of_darkhollow"] = &lua_is_current_expansion_depths_of_darkhollow;
	eq["is_current_expansion_prophecy_of_ro"] = &lua_is_current_expansion_prophecy_of_ro;
	eq["is_current_expansion_the_serpents_spine"] = &lua_is_current_expansion_the_serpents_spine;
	eq["is_current_expansion_the_buried_sea"] = &lua_is_current_expansion_the_buried_sea;
	eq["is_current_expansion_secrets_of_faydwer"] = &lua_is_current_expansion_secrets_of_faydwer;
	eq["is_current_expansion_seeds_of_destruction"] = &lua_is_current_expansion_seeds_of_destruction;
	eq["is_current_expansion_underfoot"] = &lua_is_current_expansion_underfoot;
	eq["is_current_expansion_house_of_thule"] = &lua_is_current_expansion_house_of_thule;
	eq["is_current_expansion_veil_of_alaris"] = &lua_is_current_expansion_veil_of_alaris;
	eq["is_current_expansion_rain_of_fear"] = &lua_is_current_expansion_rain_of_fear;
	eq["is_current_expansion_call_of_the_forsaken"] = &lua_is_current_expansion_call_of_the_forsaken;
	eq["is_current_expansion_the_darkened_sea"] = &lua_is_current_expansion_the_darkened_sea;
	eq["is_current_expansion_the_broken_mirror"] = &lua_is_current_expansion_the_broken_mirror;
	eq["is_current_expansion_empires_of_kunark"] = &lua_is_current_expansion_empires_of_kunark;
	eq["is_current_expansion_ring_of_scale"] = &lua_is_current_expansion_ring_of_scale;
	eq["is_current_expansion_the_burning_lands"] = &lua_is_current_expansion_the_burning_lands;
	eq["is_current_expansion_torment_of_velious"] = &lua_is_current_expansion_torment_of_velious;

	/**
	 * Content flags
	 */
	eq["is_content_flag_enabled"] = (bool(*)(std::string))&lua_is_content_flag_enabled;
	eq["set_content_flag"] = (void(*)(std::string, bool))&lua_set_content_flag;

	eq["get_expedition"] = &lua_get_expedition;
	eq["get_expedition_by_char_id"] = &lua_get_expedition_by_char_id;
	eq["get_expedition_by_dz_id"] = &lua_get_expedition_by_dz_id;
	eq["get_expedition_by_zone_instance"] = &lua_get_expedition_by_zone_instance;
	eq["get_expedition_lockout_by_char_id"] = &lua_get_expedition_lockout_by_char_id;
	eq["get_expedition_lockouts_by_char_id"] = sol::overload(
	    (sol::table(*)(sol::this_state, uint32)) & lua_get_expedition_lockouts_by_char_id,
	    (sol::table(*)(sol::this_state, uint32, std::string)) & lua_get_expedition_lockouts_by_char_id);
	eq["add_expedition_lockout_all_clients"] = sol::overload(
	    (void (*)(std::string, std::string, uint32)) & lua_add_expedition_lockout_all_clients,
	    (void (*)(std::string, std::string, uint32, std::string)) & lua_add_expedition_lockout_all_clients);
	eq["add_expedition_lockout_by_char_id"] = sol::overload(
	    (void (*)(uint32, std::string, std::string, uint32)) & lua_add_expedition_lockout_by_char_id,
	    (void (*)(uint32, std::string, std::string, uint32, std::string)) & lua_add_expedition_lockout_by_char_id);
	eq["remove_expedition_lockout_by_char_id"] = &lua_remove_expedition_lockout_by_char_id;
	eq["remove_all_expedition_lockouts_by_char_id"] =
	    sol::overload((void (*)(uint32)) & lua_remove_all_expedition_lockouts_by_char_id,
			  (void (*)(uint32, std::string)) & lua_remove_all_expedition_lockouts_by_char_id);
}

void lua_register_random(sol::state_view &sv)
{
	auto random = sv.create_named_table("Random");
	random["Int"] = &random_int;
	random["Real"] = &random_real;
	random["Roll"] = &random_roll_int;
	random["RollReal"] = &random_roll_real;
	random["Roll0"] = &random_roll0;
}


#endif
