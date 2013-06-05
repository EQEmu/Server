#ifndef _EQE_LUA_PARSER_EVENTS_H
#define _EQE_LUA_PARSER_EVENTS_H
#ifdef LUA_EQEMU

typedef void(*NPCArgumentHandler)(QuestInterface*, lua_State*, NPC*, Mob*, std::string, uint32);
typedef void(*PlayerArgumentHandler)(QuestInterface*, lua_State*, Client*, std::string, uint32);
typedef void(*ItemArgumentHandler)(QuestInterface*, lua_State*, Client*, ItemInst*, uint32, uint32);
typedef void(*SpellArgumentHandler)(QuestInterface*, lua_State*, NPC*, Client*, uint32, uint32);

//NPC
void handle_npc_event_say(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_event_trade(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_event_hp(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_single_mob(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_single_client(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_single_npc(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_popup(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_waypoint(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_hate(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_signal(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_timer(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_death(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);
void handle_npc_null(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data);

//Player
void handle_player_say(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_death(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_timer(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_discover_item(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_fish_forage_success(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_click_object(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_click_door(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_signal(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_popup_response(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_pick_up(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_cast(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_task_fail(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_zone(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_duel_win(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_duel_loss(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_loot(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_task_stage_complete(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_task_complete(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);
void handle_player_null(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data);

//Item
void handle_item_click(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, uint32 objid, uint32 extra_data);
void handle_item_null(QuestInterface *parse, lua_State* L, Client* client, ItemInst* item, uint32 objid, uint32 extra_data);

//Spell
void handle_spell_effect(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, uint32 extra_data);
void handle_spell_null(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, uint32 extra_data);

#endif
#endif
