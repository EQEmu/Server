/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EQEMU_DATABASE_SCHEMA_H
#define EQEMU_DATABASE_SCHEMA_H

#include <vector>
#include <map>

namespace DatabaseSchema {

	/**
	 * Character-specific tables
	 *
	 * Does not included related meta-data tables such as 'guilds', 'accounts'
	 * @return
	 */
	static std::map<std::string, std::string> GetCharacterTables()
	{
		return {
			{"adventure_stats",                "player_id"},
			{"buyer",                          "charid"},
			{"char_recipe_list",               "char_id"},
			{"character_activities",           "charid"},
			{"character_alt_currency",         "char_id"},
			{"character_alternate_abilities",  "id"},
			{"character_auras",                "id"},
			{"character_bandolier",            "id"},
			{"character_bind",                 "id"},
			{"character_buffs",                "character_id"},
			{"character_corpses",              "id"},
			{"character_currency",             "id"},
			{"character_data",                 "id"},
			{"character_disciplines",          "id"},
			{"character_enabledtasks",         "charid"},
			{"character_expedition_lockouts",  "character_id"},
			{"character_exp_modifiers",        "character_id"},
			{"character_inspect_messages",     "id"},
			{"character_instance_safereturns", "character_id"},
			{"character_item_recast",          "id"},
			{"character_languages",            "id"},
			{"character_leadership_abilities", "id"},
			{"character_material",             "id"},
			{"character_memmed_spells",        "id"},
			{"character_pet_buffs",            "char_id"},
			{"character_pet_info",             "char_id"},
			{"character_pet_inventory",        "char_id"},
			{"character_peqzone_flags",        "id"},
			{"character_potionbelt",           "id"},
			{"character_skills",               "id"},
			{"character_spells",               "id"},
			{"character_task_timers",          "character_id"},
			{"character_tasks",                "charid"},
			{"character_tribute",              "id"},
			{"completed_tasks",                "charid"},
			{"data_buckets",                   "id"},
			{"faction_values",                 "char_id"},
			{"friends",                        "charid"},
			{"guild_members",                  "char_id"},
			{"guilds",                         "id"},
			{"instance_list_player",           "id"},
			{"inventory",                      "charid"},
			{"inventory_snapshots",            "charid"},
			{"keyring",                        "char_id"},
			{"mail",                           "charid"},
			{"player_titlesets",               "char_id"},
			{"quest_globals",                  "charid"},
			{"timers",                         "char_id"},
			{"trader",                         "char_id"},
			{"zone_flags",                     "charID"}
		};
	}

	/**
	 * @description Gets all player and meta-data tables
	 * @note These tables have no content in the PEQ daily dump
	 *
	 * @return
	 */
	static std::vector<std::string> GetPlayerTables()
	{
		return {
			"account",
			"account_ip",
			"account_flags",
			"account_rewards",
			"adventure_details",
			"adventure_stats",
			"buyer",
			"char_recipe_list",
			"character_activities",
			"character_alt_currency",
			"character_alternate_abilities",
			"character_auras",
			"character_bandolier",
			"character_bind",
			"character_buffs",
			"character_corpse_items",
			"character_corpses",
			"character_currency",
			"character_data",
			"character_disciplines",
			"character_enabledtasks",
			"character_expedition_lockouts",
			"character_exp_modifiers",
			"character_inspect_messages",
			"character_instance_safereturns",
			"character_item_recast",
			"character_languages",
			"character_leadership_abilities",
			"character_material",
			"character_memmed_spells",
			"character_pet_buffs",
			"character_pet_info",
			"character_pet_inventory",
			"character_peqzone_flags",
			"character_potionbelt",
			"character_skills",
			"character_spells",
			"character_task_timers",
			"character_tasks",
			"character_tribute",
			"completed_tasks",
			"data_buckets",
			"discovered_items",
			"faction_values",
			"friends",
			"guild_bank",
			"guild_members",
			"guild_ranks",
			"guild_relations",
			"guilds",
			"instance_list_player",
			"inventory",
			"inventory_snapshots",
			"keyring",
			"mail",
			"petitions",
			"player_titlesets",
			"quest_globals",
			"sharedbank",
			"spell_buckets",
			"spell_globals",
			"timers",
			"trader",
			"trader_audit",
			"zone_flags"
		};
	}

	/**
	 * Gets content tables
	 *
	 * @return
	 */
	static std::vector<std::string> GetContentTables()
	{
		return {
			"aa_ability",
			"aa_rank_effects",
			"aa_rank_prereqs",
			"aa_ranks",
			"adventure_template",
			"adventure_template_entry",
			"adventure_template_entry_flavor",
			"alternate_currency",
			"auras",
			"base_data",
			"blocked_spells",
			"books",
			"char_create_combinations",
			"char_create_point_allocations",
			"damageshieldtypes",
			"doors",
			"faction_base_data",
			"faction_list",
			"faction_list_mod",
			"fishing",
			"forage",
			"global_loot",
			"goallists",
			"graveyard",
			"grid",
			"grid_entries",
			"ground_spawns",
			"horses",
			"items",
			"ldon_trap_entries",
			"ldon_trap_templates",
			"lootdrop",
			"lootdrop_entries",
			"loottable",
			"loottable_entries",
			"merchantlist",
			"npc_emotes",
			"npc_faction",
			"npc_faction_entries",
			"npc_scale_global_base",
			"npc_spells",
			"npc_spells_effects",
			"npc_spells_effects_entries",
			"npc_spells_entries",
			"npc_types",
			"npc_types_tint",
			"object",
			"pets",
			"pets_beastlord_data",
			"pets_equipmentset",
			"pets_equipmentset_entries",
			"proximities",
			"skill_caps",
			"spawn2",
			"spawn_conditions",
			"spawnentry",
			"spawngroup",
			"spells_new",
			"start_zones",
			"starting_items",
			"task_activities",
			"tasks",
			"tasksets",
			"tradeskill_recipe",
			"tradeskill_recipe_entries",
			"traps",
			"tribute_levels",
			"tributes",
			"veteran_reward_templates",
			"zone",
			"zone_points",
		};
	}

	/**
	 * Gets server tables
	 *
	 * @return
	 */
	static std::vector<std::string> GetServerTables()
	{
		return {
			"chatchannels",
			"command_settings",
			"content_flags",
			"db_str",
			"eqtime",
			"launcher",
			"launcher_zones",
			"spawn_condition_values",
			"spawn_events",
			"level_exp_mods",
			"logsys_categories",
			"name_filter",
			"perl_event_export_settings",
			"profanity_list",
			"rule_sets",
			"titles",
			"rule_values",
			"variables",
		};
	}

	/**
	 * Gets QueryServer tables
	 *
	 * @return
	 */
	static std::vector<std::string> GetQueryServerTables()
	{
		return {
			"qs_merchant_transaction_record",
			"qs_merchant_transaction_record_entries",
			"qs_player_aa_rate_hourly",
			"qs_player_delete_record",
			"qs_player_delete_record_entries",
			"qs_player_events",
			"qs_player_handin_record",
			"qs_player_handin_record_entries",
			"qs_player_move_record",
			"qs_player_move_record_entries",
			"qs_player_npc_kill_record",
			"qs_player_npc_kill_record_entries",
			"qs_player_speech",
			"qs_player_trade_record",
			"qs_player_trade_record_entries",
		};
	}

	/**
	 * Gets state tables
	 * Tables that keep track of server state
	 *
	 * @return
	 */
	static std::vector<std::string> GetStateTables()
	{
		return {
			"adventure_members",
			"banned_ips",
			"bug_reports",
			"bugs",
			"completed_shared_task_activity_state",
			"completed_shared_task_members",
			"completed_shared_tasks",
			"dynamic_zone_members",
			"dynamic_zones",
			"eventlog",
			"expedition_lockouts",
			"expeditions",
			"gm_ips",
			"group_id",
			"group_leaders",
			"hackers",
			"instance_list",
			"ip_exemptions",
			"item_tick",
			"lfguild",
			"merchantlist_temp",
			"object_contents",
			"raid_details",
			"raid_leaders",
			"raid_members",
			"reports",
			"respawn_times",
			"saylink",
			"server_scheduled_events",
			"shared_task_activity_state",
			"shared_task_dynamic_zones",
			"shared_task_members",
			"shared_tasks",
		};
	}

	/**
	 * Gets login tables
	 *
	 * @return
	 */
	static std::vector<std::string> GetLoginTables()
	{
		return {
			"login_accounts",
			"login_api_tokens",
			"login_server_admins",
			"login_server_list_types",
			"login_world_servers",
		};
	}

	/**
	 * Gets login tables
	 *
	 * @return
	 */
	static std::vector<std::string> GetVersionTables()
	{
		return {
			"db_version",
			"inventory_versions",
		};
	}
	
	/**
	 * @description Gets all player bot tables
	 * @note These tables have no content in the PEQ daily dump
	 *
	 * @return
	 */
	static std::vector<std::string> GetBotTables()
	{
		return {
			"bot_buffs",
			"bot_command_settings",
			"bot_create_combinations",
			"bot_data",
			"bot_group_members",
			"bot_groups",
			"bot_guild_members",
			"bot_heal_rotation_members",
			"bot_heal_rotation_targets",
			"bot_heal_rotations",
			"bot_inspect_messages",
			"bot_inventories",
			"bot_owner_options",
			"bot_pet_buffs",
			"bot_pet_inventories",
			"bot_pets",
			"bot_spell_casting_chances",
			"bot_spells_entries",
			"bot_stances",
			"bot_timers",
			"vw_bot_character_mobs",
			"vw_bot_groups"
		};
	}

}

#endif //EQEMU_DATABASE_SCHEMA_H
