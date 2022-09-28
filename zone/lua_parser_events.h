#pragma once
#ifdef LUA_EQEMU

typedef void(*NPCArgumentHandler)(QuestInterface*, sol::state_view, sol::table&, NPC*, Mob*, std::string, uint32, std::vector<std::any>*);
typedef void(*PlayerArgumentHandler)(QuestInterface*, sol::state_view, sol::table&, Client*, std::string, uint32, std::vector<std::any>*);
typedef void(*ItemArgumentHandler)(QuestInterface*, sol::state_view, sol::table&, Client*, EQ::ItemInstance*, Mob*, std::string, uint32, std::vector<std::any>*);
typedef void(*SpellArgumentHandler)(QuestInterface*, sol::state_view, sol::table&, NPC*, Client*, uint32, std::string, uint32, std::vector<std::any>*);
typedef void(*EncounterArgumentHandler)(QuestInterface*, sol::state_view, sol::table&, Encounter* encounter, std::string, uint32, std::vector<std::any>*);

//NPC
void handle_npc_event_say(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_event_trade(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_event_hp(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_single_mob(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_single_client(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_single_npc(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_task_accepted(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_popup(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_waypoint(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_hate(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_signal(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_timer(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_death(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_cast(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_area(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_null(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);
void handle_npc_loot_zone(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						  std::vector<std::any> *extra_pointers);

//Player
void handle_player_say(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_environmental_damage(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers);
void handle_player_death(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_timer(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_discover_item(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_fish_forage_success(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_click_object(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_click_door(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_signal(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_popup_response(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_pick_up(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_cast(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_task_fail(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_zone(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_duel_win(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_duel_loss(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_loot(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_task_stage_complete(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_task_update(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_command(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_combine(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_feign(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_area(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_respawn(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_packet(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_null(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_use_skill(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_test_buff(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any>* extra_pointers);
void handle_player_combine_validate(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any>* extra_pointers);
void handle_player_bot_command(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_player_warp(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);
void handle_player_quest_combine(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);
void handle_player_consider(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);
void handle_player_consider_corpse(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);
void handle_player_equip_item(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);
void handle_player_skill_up(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);
void handle_player_language_skill_up(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);
void handle_player_alt_currency_merchant(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);
void handle_player_merchant(QuestInterface* parse, sol::state_view sv, sol::table& L, Client* client, std::string data, uint32 extra_data,
	std::vector<std::any>* extra_pointers);

//Item
void handle_item_click(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_item_timer(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_item_proc(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_item_loot(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_item_equip(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_item_augment(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_item_augment_insert(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_item_augment_remove(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_item_null(QuestInterface *parse, sol::state_view sv, sol::table& L, Client* client, EQ::ItemInstance* item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);

//Spell
void handle_spell_event(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_translocate_finish(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_spell_null(QuestInterface *parse, sol::state_view sv, sol::table& L, NPC* npc, Client* client, uint32 spell_id, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);


//Encounter
void handle_encounter_timer(QuestInterface *parse, sol::state_view sv, sol::table& L, Encounter* encounter, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);
void handle_encounter_load(QuestInterface *parse, sol::state_view sv, sol::table& L, Encounter* encounter, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers);
void handle_encounter_unload(QuestInterface *parse, sol::state_view sv, sol::table& L, Encounter* encounter, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers);
void handle_encounter_null(QuestInterface *parse, sol::state_view sv, sol::table& L, Encounter* encounter, std::string data, uint32 extra_data,
		std::vector<std::any> *extra_pointers);

#endif
