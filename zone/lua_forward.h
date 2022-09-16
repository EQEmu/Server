#pragma once
#ifdef LUA_EQEMU
#include <sol/forward.hpp>
#include <sol/sol.hpp>

void lua_register_general(sol::state_view &sv);
void lua_register_random(sol::state_view &sv);
void lua_register_events(sol::state_view &sv);
void lua_register_faction(sol::state_view &sv);
void lua_register_slot(sol::state_view &sv);
void lua_register_material(sol::state_view &sv);
void lua_register_client_version(sol::state_view &sv);
void lua_register_appearance(sol::state_view &sv);
void lua_register_classes(sol::state_view &sv);
void lua_register_skills(sol::state_view &sv);
void lua_register_bodytypes(sol::state_view &sv);
void lua_register_filters(sol::state_view &sv);
void lua_register_message_types(sol::state_view &sv);
void lua_register_entity(sol::state_view &sv);
void lua_register_encounter(sol::state_view &sv);
void lua_register_mob(sol::state_view &sv);
void lua_register_special_abilities(sol::state_view &sv);
void lua_register_npc(sol::state_view &sv);
void lua_register_client(sol::state_view &sv);
#ifdef BOTS
void lua_register_bot(sol::state_view &sv);
#endif
void lua_register_inventory(sol::state_view &sv);
void lua_register_inventory_where(sol::state_view &sv);
void lua_register_iteminst(sol::state_view &sv);
void lua_register_item(sol::state_view &sv);
void lua_register_spell(sol::state_view &sv);
void lua_register_spawn(sol::state_view &sv);
void lua_register_hate_entry(sol::state_view &sv);
void lua_register_hate_list(sol::state_view &sv);
void lua_register_entity_list(sol::state_view &sv);
void lua_register_mob_list(sol::state_view &sv);
void lua_register_client_list(sol::state_view &sv);
#ifdef BOTS
void lua_register_bot_list(sol::state_view &sv);
#endif
void lua_register_npc_list(sol::state_view &sv);
void lua_register_corpse_list(sol::state_view &sv);
void lua_register_object_list(sol::state_view &sv);
void lua_register_door_list(sol::state_view &sv);
void lua_register_spawn_list(sol::state_view &sv);
void lua_register_corpse_loot_list(sol::state_view &sv);
void lua_register_npc_loot_list(sol::state_view &sv);
void lua_register_group(sol::state_view &sv);
void lua_register_raid(sol::state_view &sv);
void lua_register_corpse(sol::state_view &sv);
void lua_register_door(sol::state_view &sv);
void lua_register_object(sol::state_view &sv);
void lua_register_packet(sol::state_view &sv);
void lua_register_packet_opcodes(sol::state_view &sv);
void lua_register_stat_bonuses(sol::state_view &sv);
void lua_register_rules_const(sol::state_view &sv);
void lua_register_rulei(sol::state_view &sv);
void lua_register_ruler(sol::state_view &sv);
void lua_register_ruleb(sol::state_view &sv);
void lua_register_journal_speakmode(sol::state_view &sv);
void lua_register_journal_mode(sol::state_view &sv);
void lua_register_expedition(sol::state_view &sv);
void lua_register_expedition_lock_messages(sol::state_view &sv);

#endif
