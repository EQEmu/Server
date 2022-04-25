#ifndef _EQE_LUA_PARSER_EVENTS_H
#define _EQE_LUA_PARSER_EVENTS_H
#ifdef LUA_EQEMU

typedef void(*NPCArgumentHandler)(QuestInterface*, lua_State*, NPC*, Mob*, std::string, uint32, std::vector<EQ::Any>*);
typedef void(*PlayerArgumentHandler)(QuestInterface*, lua_State*, Client*, std::string, uint32, std::vector<EQ::Any>*);
typedef void(*ItemArgumentHandler)(QuestInterface*, lua_State*, Client*, EQ::ItemInstance*, Mob*, std::string, uint32, std::vector<EQ::Any>*);
typedef void(*SpellArgumentHandler)(QuestInterface*, lua_State*, NPC*, Client*, uint32, std::string, uint32, std::vector<EQ::Any>*);
typedef void(*EncounterArgumentHandler)(QuestInterface*, lua_State*, Encounter* encounter, std::string, uint32, std::vector<EQ::Any>*);

//NPC
void handle_npc_event_say(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_event_trade(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_event_hp(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_single_mob(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_single_client(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_single_npc(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_task_accepted(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_popup(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_waypoint(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_hate(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_signal(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_timer(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_death(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_cast(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_area(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_null(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);
void handle_npc_loot_zone(QuestInterface *parse, lua_State* L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<EQ::Any> *extra_pointers);

//Player
void handle_player_say(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_environmental_damage(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any> *extra_pointers);
void handle_player_death(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_timer(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_discover_item(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_fish_forage_success(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_click_object(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_click_door(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_signal(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_popup_response(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_pick_up(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_cast(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_task_fail(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_zone(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_duel_win(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_duel_loss(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_loot(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_task_stage_complete(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_task_update(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_command(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_combine(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_feign(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_area(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_respawn(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_packet(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_null(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_use_skill(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_test_buff(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any>* extra_pointers);
void handle_player_combine_validate(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any>* extra_pointers);
void handle_player_bot_command(QuestInterface *parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_player_warp(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any>* extra_pointers);
void handle_player_quest_combine(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any>* extra_pointers);
void handle_player_consider(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any>* extra_pointers);
void handle_player_consider_corpse(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any>* extra_pointers);
void handle_player_equip_item(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any>* extra_pointers);
void handle_player_skill_up(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any>* extra_pointers);
void handle_player_language_skill_up(QuestInterface* parse, lua_State* L, Client* client, std::string data, uint32 extra_data,
	std::vector<EQ::Any>* extra_pointers);

//Item
void handle_item_click(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_item_timer(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_item_proc(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_item_loot(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_item_equip(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_item_augment(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_item_augment_insert(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_item_augment_remove(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_item_null(QuestInterface *parse, lua_State* L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);

//Spell
void handle_spell_event(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_translocate_finish(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_spell_null(QuestInterface *parse, lua_State* L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);


//Encounter
void handle_encounter_timer(QuestInterface *parse, lua_State* L, Encounter* encounter, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);
void handle_encounter_load(QuestInterface *parse, lua_State* L, Encounter* encounter, std::string data, uint32 extra_data,
	std::vector<EQ::Any> *extra_pointers);
void handle_encounter_unload(QuestInterface *parse, lua_State* L, Encounter* encounter, std::string data, uint32 extra_data,
	std::vector<EQ::Any> *extra_pointers);
void handle_encounter_null(QuestInterface *parse, lua_State* L, Encounter* encounter, std::string data, uint32 extra_data,
		std::vector<EQ::Any> *extra_pointers);

#endif
#endif
