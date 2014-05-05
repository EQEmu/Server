#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include <sstream>
#include <list>
#include <map>

#include "lua_parser.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_entity_list.h"
#include "QuestParserCollection.h"
#include "questmgr.h"
#include "QGlobals.h"
#include "../common/timer.h"

struct Events { };
struct Factions { };
struct Slots { };
struct Materials { };
struct ClientVersions { };
struct Appearances { };

struct lua_registered_event {
	std::string encounter_name;
	luabind::object lua_reference;
	QuestEventID event_id;
};

extern std::map<std::string, std::list<lua_registered_event>> lua_encounter_events_registered;
extern void MapOpcodes();
extern void ClearMappedOpcode(EmuOpcode op);

void load_encounter(std::string name) {
	parse->EventEncounter(EVENT_ENCOUNTER_LOAD, name, 0);
}

void unload_encounter(std::string name) {
	parse->EventEncounter(EVENT_ENCOUNTER_UNLOAD, name, 0);
}

void register_event(std::string package_name, std::string name, int evt, luabind::object func) {
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
		std::list<lua_registered_event> elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if(iter->event_id == evt && iter->encounter_name.compare(name) == 0) {
				//already registered this event for this encounter
				return;
			}
			++iter;
		}

		elist.push_back(e);
		lua_encounter_events_registered[package_name] = elist;
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
			}
		}
		lua_encounter_events_registered[package_name] = elist;
	}
}

void register_npc_event(std::string name, int evt, int npc_id, luabind::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "npc_" << npc_id;

		register_event(package_name.str(), name, evt, func);
	}
}

void unregister_npc_event(std::string name, int evt, int npc_id) {
	std::stringstream package_name;
	package_name << "npc_" << npc_id;

	unregister_event(package_name.str(), name, evt);
}

void register_player_event(std::string name, int evt, luabind::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		register_event("player", name, evt, func);
	}
}

void unregister_player_event(std::string name, int evt) {
	unregister_event("player", name, evt);
}

void register_item_event(std::string name, int evt, int item_id, luabind::object func) {
	std::string package_name = "item_";
	package_name += std::to_string(static_cast<long long>(item_id));
	
	if(luabind::type(func) == LUA_TFUNCTION) {
		register_event(package_name, name, evt, func);
	}
}

void unregister_item_event(std::string name, int evt, int item_id) {
	std::string package_name = "item_";
	package_name += std::to_string(static_cast<long long>(item_id));

	unregister_event(package_name, name, evt);
}

void register_spell_event(std::string name, int evt, int spell_id, luabind::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "spell_" << spell_id;

		register_event(package_name.str(), name, evt, func);
	}
}

void unregister_spell_event(std::string name, int evt, int spell_id) {
	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	unregister_event(package_name.str(), name, evt);
}

Lua_Mob lua_spawn2(int npc_type, int grid, int unused, double x, double y, double z, double heading) {
	return Lua_Mob(quest_manager.spawn2(npc_type, grid, unused,
		static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(heading)));
}

Lua_Mob lua_unique_spawn(int npc_type, int grid, int unused, double x, double y, double z, double heading = 0.0) {
	return Lua_Mob(quest_manager.unique_spawn(npc_type, grid, unused,
		static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(heading)));
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

void lua_stop_timer(const char *timer) {
	quest_manager.stoptimer(timer);
}

void lua_stop_timer(const char *timer, Lua_ItemInst inst) {
	quest_manager.stoptimer(timer, inst);
}

void lua_stop_timer(const char *timer, Lua_Mob mob) {
	quest_manager.stoptimer(timer, mob);
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

bool lua_is_disc_tome(int item_id) {
	return quest_manager.isdisctome(item_id);
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
	quest_manager.settime(hour, min);
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
	quest_manager.moveto(x, y, z, 0, false);
}

void lua_move_to(float x, float y, float z, float h) {
	quest_manager.moveto(x, y, z, h, false);
}

void lua_move_to(float x, float y, float z, float h, bool save_guard_spot) {
	quest_manager.moveto(x, y, z, h, save_guard_spot);
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

void lua_summon_burried_player_corpse(uint32 char_id, float x, float y, float z, float h) {
	quest_manager.summonburriedplayercorpse(char_id, x, y, z, h);
}

void lua_summon_all_player_corpses(uint32 char_id, float x, float y, float z, float h) {
	quest_manager.summonallplayercorpses(char_id, x, y, z, h);
}

int lua_get_player_burried_corpse_count(uint32 char_id) {
	return quest_manager.getplayerburriedcorpsecount(char_id);
}

bool lua_bury_player_corpse(uint32 char_id) {
	return quest_manager.buryplayercorpse(char_id);
}

void lua_task_selector(luabind::object table) {
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
			} catch(luabind::cast_failed) {
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

void lua_enable_task(luabind::object table) {
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
			} catch(luabind::cast_failed) {
			}
		} else {
			count = i - 1;
			break;
		}

		tasks[i - 1] = cur_value;
	}

	quest_manager.enabletask(count, tasks);
}

void lua_disable_task(luabind::object table) {
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
			} catch(luabind::cast_failed) {
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

int lua_get_level(int type) {
	return quest_manager.getlevel(type);
}

void lua_create_ground_object(uint32 item_id, float x, float y, float z, float h) {
	quest_manager.CreateGroundObject(item_id, x, y, z, h);
}

void lua_create_ground_object(uint32 item_id, float x, float y, float z, float h, uint32 decay_time) {
	quest_manager.CreateGroundObject(item_id, x, y, z, h, decay_time);
}

void lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h) {
	quest_manager.CreateGroundObjectFromModel(model, x, y, z, h);
}

void lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h, int type) {
	quest_manager.CreateGroundObjectFromModel(model, x, y, z, h, type);
}

void lua_create_ground_object_from_model(const char *model, float x, float y, float z, float h, int type, uint32 decay_time) {
	quest_manager.CreateGroundObjectFromModel(model, x, y, z, h, type, decay_time);
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
	quest_manager.varlink(text, item_id);

	return std::string(text);
}

std::string lua_say_link(const char *phrase, bool silent, const char *link_name) {
	char text[256] = { 0 };
	strncpy(text, phrase, 255);
	quest_manager.saylink(text, silent, link_name);

	return std::string(text);
}

std::string lua_say_link(const char *phrase, bool silent) {
	char text[256] = { 0 };
	strncpy(text, phrase, 255);
	quest_manager.saylink(text, silent, text);

	return std::string(text);
}

std::string lua_say_link(const char *phrase) {
	char text[256] = { 0 };
	strncpy(text, phrase, 255);
	quest_manager.saylink(text, false, text);

	return std::string(text);
}

const char *lua_get_guild_name_by_id(uint32 guild_id) {
	return quest_manager.getguildnamebyid(guild_id);
}

uint32 lua_create_instance(const char *zone, uint32 version, uint32 duration) {
	return quest_manager.CreateInstance(zone, version, duration);
}

void lua_destroy_instance(uint32 instance_id) {
	quest_manager.DestroyInstance(instance_id);
}

int lua_get_instance_id(const char *zone, uint32 version) {
	return quest_manager.GetInstanceID(zone, version);
}

void lua_assign_to_instance(uint32 instance_id) {
	quest_manager.AssignToInstance(instance_id);
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
	quest_manager.FlyMode(flymode);
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

void lua_cross_zone_signal_client_by_char_id(uint32 player_id, int signal) {
	quest_manager.CrossZoneSignalPlayerByCharID(player_id, signal);
}

void lua_cross_zone_signal_client_by_name(const char *player, int signal) {
	quest_manager.CrossZoneSignalPlayerByName(player, signal);
}

void lua_cross_zone_message_player_by_name(uint32 type, const char *player, const char *message) {
	quest_manager.CrossZoneMessagePlayerByName(type, player, message);
}

luabind::object lua_get_qglobals(lua_State *L, Lua_NPC npc, Lua_Client client) {
	luabind::object ret = luabind::newtable(L);

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

luabind::object lua_get_qglobals(lua_State *L, Lua_Client client, Lua_NPC npc) {
	return lua_get_qglobals(L, npc, client);
}

luabind::object lua_get_qglobals(lua_State *L, Lua_Client client) {
	return lua_get_qglobals(L, Lua_NPC(nullptr), client);
}

luabind::object lua_get_qglobals(lua_State *L, Lua_NPC npc) {
	return lua_get_qglobals(L, npc, Lua_Client(nullptr));
}

luabind::object lua_get_qglobals(lua_State *L) {
	return lua_get_qglobals(L, Lua_NPC(nullptr), Lua_Client(nullptr));
}

Lua_EntityList lua_get_entity_list() {
	return Lua_EntityList(&entity_list);
}

int lua_get_zone_id() {
	if(!zone)
		return 0;

	return zone->GetZoneID();
}

const char *lua_get_zone_long_name() {
	if(!zone)
		return "";

	return zone->GetLongName();
}

const char *lua_get_zone_short_name() {
	if(!zone)
		return "";

	return zone->GetShortName();
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

luabind::object lua_get_characters_in_instance(lua_State *L, uint16 instance_id) {
	luabind::object ret = luabind::newtable(L);

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

luabind::object lua_get_zone_time(lua_State *L) {
	TimeOfDay_Struct eqTime;
	zone->zone_time.getEQTimeOfDay(time(0), &eqTime);

	luabind::object ret = luabind::newtable(L);
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

void lua_add_spawn_point(luabind::object table) {
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
		int condition_id = 0;
		int condition_min_value = 0;
		bool enabled = true;
		int animation = 0;
		
		auto cur = table["spawn2_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				spawn2_id = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed) {
				return;
			}
		} else {
			return;
		}

		cur = table["spawngroup_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				spawngroup_id = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed) {
				return;
			}
		} else {
			return;
		}

		cur = table["x"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				x = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
				return;
			}
		} else {
			return;
		}

		cur = table["y"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				y = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
				return;
			}
		} else {
			return;
		}

		cur = table["z"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				z = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
				return;
			}
		} else {
			return;
		}

		cur = table["heading"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				heading = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
				return;
			}
		} else {
			return;
		}

		cur = table["respawn"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				respawn = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed) {
				return;
			}
		} else {
			return;
		}

		cur = table["variance"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				variance = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed) {
				return;
			}
		} else {
			return;
		}

		cur = table["timeleft"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				timeleft = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = table["grid"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				grid = luabind::object_cast<uint32>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = table["condition_id"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				condition_id = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = table["condition_min_value"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				condition_min_value = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = table["enabled"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				enabled = luabind::object_cast<bool>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = table["animation"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				animation = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		lua_remove_spawn_point(spawn2_id);

		Spawn2 *t = new Spawn2(spawn2_id, spawngroup_id, x, y, z, heading, respawn, variance, timeleft, grid, condition_id,
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

double lua_clock() {
	timeval read_time;
	gettimeofday(&read_time, nullptr);
	uint32 t = read_time.tv_sec * 1000 + read_time.tv_usec / 1000;
	return static_cast<double>(t) / 1000.0;
}

luabind::scope lua_register_general() {
	return luabind::namespace_("eq")
	[
		luabind::def("load_encounter", &load_encounter),
		luabind::def("unload_encounter", &unload_encounter),
		luabind::def("register_npc_event", &register_npc_event),
		luabind::def("unregister_npc_event", &unregister_npc_event),
		luabind::def("register_player_event", &register_player_event),
		luabind::def("unregister_player_event", &unregister_player_event),
		luabind::def("register_item_event", &register_item_event),
		luabind::def("unregister_item_event", &unregister_item_event),
		luabind::def("register_spell_event", &register_spell_event),
		luabind::def("unregister_spell_event", &unregister_spell_event),
		luabind::def("spawn2", (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_spawn2),
		luabind::def("unique_spawn", (Lua_Mob(*)(int,int,int,double,double,double))&lua_unique_spawn),
		luabind::def("unique_spawn", (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_unique_spawn),
		luabind::def("spawn_from_spawn2", (Lua_Mob(*)(uint32))&lua_spawn_from_spawn2),
		luabind::def("enable_spawn2", &lua_enable_spawn2),
		luabind::def("disable_spawn2", &lua_disable_spawn2),
		luabind::def("set_timer", (void(*)(const char*, int))&lua_set_timer),
		luabind::def("set_timer", (void(*)(const char*, int, Lua_ItemInst))&lua_set_timer),
		luabind::def("set_timer", (void(*)(const char*, int, Lua_Mob))&lua_set_timer),
		luabind::def("stop_timer", (void(*)(const char*))&lua_stop_timer),
		luabind::def("stop_timer", (void(*)(const char*, Lua_ItemInst))&lua_stop_timer),
		luabind::def("stop_timer", (void(*)(const char*, Lua_Mob))&lua_stop_timer),
		luabind::def("stop_all_timers", (void(*)(void))&lua_stop_all_timers),
		luabind::def("stop_all_timers", (void(*)(Lua_ItemInst))&lua_stop_all_timers),
		luabind::def("stop_all_timers", (void(*)(Lua_Mob))&lua_stop_all_timers),
		luabind::def("depop", (void(*)(void))&lua_depop),
		luabind::def("depop", (void(*)(int))&lua_depop),
		luabind::def("depop_with_timer", (void(*)(void))&lua_depop_with_timer),
		luabind::def("depop_with_timer", (void(*)(int))&lua_depop_with_timer),
		luabind::def("depop_all", (void(*)(void))&lua_depop_all),
		luabind::def("depop_all", (void(*)(int))&lua_depop_all),
		luabind::def("depop_zone", &lua_depop_zone),
		luabind::def("repop_zone", &lua_repop_zone),
		luabind::def("is_disc_tome", &lua_is_disc_tome),
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
		luabind::def("set_time", &lua_set_time),
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
		luabind::def("clear_proximity", &lua_clear_proximity),
		luabind::def("enable_proximity_say", &lua_enable_proximity_say),
		luabind::def("disable_proximity_say", &lua_disable_proximity_say),
		luabind::def("set_anim", &lua_set_anim),
		luabind::def("spawn_condition", &lua_spawn_condition),
		luabind::def("get_spawn_condition", &lua_get_spawn_condition),
		luabind::def("toggle_spawn_event", &lua_toggle_spawn_event),
		luabind::def("summon_burried_player_corpse", &lua_summon_burried_player_corpse),
		luabind::def("summon_all_player_corpses", &lua_summon_all_player_corpses),
		luabind::def("get_player_burried_corpse_count", &lua_get_player_burried_corpse_count),
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
		luabind::def("popup", &lua_popup),
		luabind::def("clear_spawn_timers", &lua_clear_spawn_timers),
		luabind::def("zone_emote", &lua_zone_emote),
		luabind::def("world_emote", &lua_world_emote),
		luabind::def("get_level", &lua_get_level),
		luabind::def("create_ground_object", (void(*)(uint32,float,float,float,float))&lua_create_ground_object),
		luabind::def("create_ground_object", (void(*)(uint32,float,float,float,float,uint32))&lua_create_ground_object),
		luabind::def("create_ground_object_from_model", (void(*)(const char*,float,float,float,float))&lua_create_ground_object_from_model),
		luabind::def("create_ground_object_from_model", (void(*)(const char*,float,float,float,float,int))&lua_create_ground_object_from_model),
		luabind::def("create_ground_object_from_model", (void(*)(const char*,float,float,float,float,int,uint32))&lua_create_ground_object_from_model),
		luabind::def("create_door", &lua_create_door),
		luabind::def("modify_npc_stat", &lua_modify_npc_stat),
		luabind::def("collect_items", &lua_collect_items),
		luabind::def("update_spawn_timer", &lua_update_spawn_timer),
		luabind::def("merchant_set_item", (void(*)(uint32,uint32))&lua_merchant_set_item),
		luabind::def("merchant_set_item", (void(*)(uint32,uint32,uint32))&lua_merchant_set_item),
		luabind::def("merchant_count_item", &lua_merchant_count_item),
		luabind::def("item_link", &lua_item_link),
		luabind::def("say_link", (std::string(*)(const char*,bool,const char*))&lua_say_link),
		luabind::def("say_link", (std::string(*)(const char*,bool))&lua_say_link),
		luabind::def("say_link", (std::string(*)(const char*))&lua_say_link),
		luabind::def("get_guild_name_by_id", &lua_get_guild_name_by_id),
		luabind::def("create_instance", &lua_create_instance),
		luabind::def("destroy_instance", &lua_destroy_instance),
		luabind::def("get_instance_id", &lua_get_instance_id),
		luabind::def("get_characters_in_instance", &lua_get_characters_in_instance),
		luabind::def("assign_to_instance", &lua_assign_to_instance),
		luabind::def("assign_group_to_instance", &lua_assign_group_to_instance),
		luabind::def("assign_raid_to_instance", &lua_assign_raid_to_instance),
		luabind::def("remove_from_instance", &lua_remove_from_instance),
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
		luabind::def("cross_zone_signal_client_by_char_id", &lua_cross_zone_signal_client_by_char_id),
		luabind::def("cross_zone_signal_client_by_name", &lua_cross_zone_signal_client_by_name),
		luabind::def("cross_zone_message_player_by_name", &lua_cross_zone_message_player_by_name),
		luabind::def("get_qglobals", (luabind::object(*)(lua_State*,Lua_NPC,Lua_Client))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::object(*)(lua_State*,Lua_Client,Lua_NPC))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::object(*)(lua_State*,Lua_Client))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::object(*)(lua_State*,Lua_NPC))&lua_get_qglobals),
		luabind::def("get_qglobals", (luabind::object(*)(lua_State*))&lua_get_qglobals),
		luabind::def("get_entity_list", &lua_get_entity_list),
		luabind::def("get_zone_id", &lua_get_zone_id),
		luabind::def("get_zone_long_name", &lua_get_zone_long_name),
		luabind::def("get_zone_short_name", &lua_get_zone_short_name),
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
		luabind::def("map_opcodes", &lua_map_opcodes),
		luabind::def("clear_opcode", &lua_clear_opcode),
		luabind::def("enable_recipe", &lua_enable_recipe),
		luabind::def("disable_recipe", &lua_disable_recipe),
		luabind::def("clear_npctype_cache", &lua_clear_npctype_cache),
		luabind::def("clock", &lua_clock)
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
			luabind::value("cast_begin", static_cast<int>(EVENT_CAST_BEGIN)),
			luabind::value("scale_calc", static_cast<int>(EVENT_SCALE_CALC)),
			luabind::value("item_enter_zone", static_cast<int>(EVENT_ITEM_ENTER_ZONE)),
			luabind::value("target_change", static_cast<int>(EVENT_TARGET_CHANGE)),
			luabind::value("hate_list", static_cast<int>(EVENT_HATE_LIST)),
			luabind::value("spell_effect", static_cast<int>(EVENT_SPELL_EFFECT_CLIENT)),
			luabind::value("spell_buff_tic", static_cast<int>(EVENT_SPELL_BUFF_TIC_CLIENT)),
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
			luabind::value("unhandled_opcode", static_cast<int>(EVENT_UNHANDLED_OPCODE))
		];
}

luabind::scope lua_register_faction() {
	return luabind::class_<Factions>("Faction")
		.enum_("constants")
		[
			luabind::value("Ally", static_cast<int>(FACTION_ALLY)),
			luabind::value("Warmly", static_cast<int>(FACTION_WARMLY)),
			luabind::value("Kindly", static_cast<int>(FACTION_KINDLY)),
			luabind::value("Amiable", static_cast<int>(FACTION_AMIABLE)),
			luabind::value("Indifferent", static_cast<int>(FACTION_INDIFFERENT)),
			luabind::value("Apprehensive", static_cast<int>(FACTION_APPREHENSIVE)),
			luabind::value("Dubious", static_cast<int>(FACTION_DUBIOUS)),
			luabind::value("Threatenly", static_cast<int>(FACTION_THREATENLY)),
			luabind::value("Scowls", static_cast<int>(FACTION_SCOWLS))
		];
}

luabind::scope lua_register_slot() {
	return luabind::class_<Slots>("Slot")
		.enum_("constants")
		[
			luabind::value("Charm", static_cast<int>(SLOT_CHARM)),
			luabind::value("Ear1", static_cast<int>(SLOT_EAR01)),
			luabind::value("Head", static_cast<int>(SLOT_HEAD)),
			luabind::value("Face", static_cast<int>(SLOT_FACE)),
			luabind::value("Ear2", static_cast<int>(SLOT_EAR02)),
			luabind::value("Neck", static_cast<int>(SLOT_NECK)),
			luabind::value("Shoulder", static_cast<int>(SLOT_SHOULDER)),
			luabind::value("Arms", static_cast<int>(SLOT_ARMS)),
			luabind::value("Back", static_cast<int>(SLOT_BACK)),
			luabind::value("Bracer1", static_cast<int>(SLOT_BRACER01)),
			luabind::value("Bracer2", static_cast<int>(SLOT_BRACER02)),
			luabind::value("Range", static_cast<int>(SLOT_RANGE)),
			luabind::value("Hands", static_cast<int>(SLOT_HANDS)),
			luabind::value("Primary", static_cast<int>(SLOT_PRIMARY)),
			luabind::value("Secondary", static_cast<int>(SLOT_SECONDARY)),
			luabind::value("Ring1", static_cast<int>(SLOT_RING01)),
			luabind::value("Ring2", static_cast<int>(SLOT_RING02)),
			luabind::value("Chest", static_cast<int>(SLOT_CHEST)),
			luabind::value("Legs", static_cast<int>(SLOT_LEGS)),
			luabind::value("Feet", static_cast<int>(SLOT_FEET)),
			luabind::value("Waist", static_cast<int>(SLOT_WAIST)),
			luabind::value("Ammo", static_cast<int>(SLOT_AMMO)),
			luabind::value("PersonalBegin", static_cast<int>(SLOT_PERSONAL_BEGIN)),
			luabind::value("PersonalEnd", static_cast<int>(SLOT_PERSONAL_END)),
			luabind::value("Cursor", static_cast<int>(SLOT_CURSOR)),
			luabind::value("CursorEnd", 0xFFFE),
			luabind::value("Tradeskill", static_cast<int>(SLOT_TRADESKILL)),
			luabind::value("Augment", static_cast<int>(SLOT_AUGMENT)),
			luabind::value("PowerSource", static_cast<int>(SLOT_POWER_SOURCE)),
			luabind::value("Invalid", 0xFFFF)
		];
}

luabind::scope lua_register_material() {
	return luabind::class_<Materials>("Material")
		.enum_("constants")
		[
			luabind::value("Head", static_cast<int>(MaterialHead)),
			luabind::value("Chest", static_cast<int>(MaterialChest)),
			luabind::value("Arms", static_cast<int>(MaterialArms)),
			luabind::value("Bracer", static_cast<int>(MaterialWrist)), // deprecated
			luabind::value("Wrist", static_cast<int>(MaterialWrist)),
			luabind::value("Hands", static_cast<int>(MaterialHands)),
			luabind::value("Legs", static_cast<int>(MaterialLegs)),
			luabind::value("Feet", static_cast<int>(MaterialFeet)),
			luabind::value("Primary", static_cast<int>(MaterialPrimary)),
			luabind::value("Secondary", static_cast<int>(MaterialSecondary)),
			luabind::value("Max", static_cast<int>(_MaterialCount)), // deprecated
			luabind::value("Count", static_cast<int>(_MaterialCount)),
			luabind::value("Invalid", static_cast<int>(_MaterialInvalid))
		];
}

luabind::scope lua_register_client_version() {
	return luabind::class_<ClientVersions>("ClientVersion")
		.enum_("constants")
		[
			luabind::value("Unknown", static_cast<int>(EQClientUnknown)),
			luabind::value("62", static_cast<int>(EQClient62)),
			luabind::value("Titanium", static_cast<int>(EQClientTitanium)),
			luabind::value("SoF", static_cast<int>(EQClientSoF)),
			luabind::value("SoD", static_cast<int>(EQClientSoD)),
			luabind::value("Underfoot", static_cast<int>(EQClientUnderfoot)),
			luabind::value("RoF", static_cast<int>(EQClientRoF))
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

#endif
