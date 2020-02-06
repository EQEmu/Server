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
			{"character_inspect_messages",     "id"},
			{"character_item_recast",          "id"},
			{"character_languages",            "id"},
			{"character_leadership_abilities", "id"},
			{"character_material",             "id"},
			{"character_memmed_spells",        "id"},
			{"character_pet_buffs",            "char_id"},
			{"character_pet_info",             "char_id"},
			{"character_pet_inventory",        "char_id"},
			{"character_potionbelt",           "id"},
			{"character_skills",               "id"},
			{"character_spells",               "id"},
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
			{"titles",                         "char_id"},
			{"trader",                         "char_id"},
			{"zone_flags",                     "charID"}
		};
	}

	/**
	 * Gets all player and meta-data tables
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
			"character_inspect_messages",
			"character_item_recast",
			"character_languages",
			"character_leadership_abilities",
			"character_material",
			"character_memmed_spells",
			"character_pet_buffs",
			"character_pet_info",
			"character_pet_inventory",
			"character_potionbelt",
			"character_skills",
			"character_spells",
			"character_tasks",
			"character_tribute",
			"completed_tasks",
			"data_buckets",
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
			"player_titlesets",
			"quest_globals",
			"sharedbank",
			"timers",
			"titles",
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
			"aa_actions",
			"aa_effects",
			"aa_rank_effects",
			"aa_rank_prereqs",
			"aa_ranks",
			"aa_required_level_cost",
			"adventure_template",
			"adventure_template_entry",
			"adventure_template_entry_flavor",
			"altadv_vars",
			"alternate_currency",
			"auras",
			"base_data",
			"blocked_spells",
			"books",
			"char_create_combinations",
			"char_create_point_allocations",
			"class_skill",
			"damageshieldtypes",
			"doors",
			"faction_base_data",
			"faction_list",
			"faction_list_mod",
			"fear_hints",
			"fishing",
			"forage",
			"global_loot",
			"goallists",
			"graveyard",
			"grid",
			"grid_entries",
			"ground_spawns",
			"horses",
			"instance_list",
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
			"npc_types_metadata",
			"npc_types_tint",
			"object",
			"pets",
			"pets_equipmentset",
			"pets_equipmentset_entries",
			"proximities",
			"races",
			"skill_caps",
			"spawn2",
			"spawn_condition_values",
			"spawn_conditions",
			"spawn_events",
			"spawnentry",
			"spawngroup",
			"spells_new",
			"start_zones",
			"starting_items",
			"task_activities",
			"tasks",
			"tasksets",
			"titles",
			"tradeskill_recipe",
			"tradeskill_recipe_entries",
			"traps",
			"tribute_levels",
			"tributes",
			"veteran_reward_templates",
			"zone",
			"zone_points",
			"zone_server",
			"zoneserver_auth",
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
			"banned_ips",
			"bugs",
			"bug_reports",
			"command_settings",
			"db_str",
			"discovered_items",
			"eqtime",
			"eventlog",
			"gm_ips",
			"hackers",
			"ip_exemptions",
			"launcher",
			"launcher_zones",
			"level_exp_mods",
			"logsys_categories",
			"name_filter",
			"perl_event_export_settings",
			"petitions",
			"profanity_list",
			"reports",
			"rule_sets",
			"rule_values",
			"saylink",
			"variables",
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
			"chatchannels",
			"group_id",
			"group_leaders",
			"item_tick",
			"lfguild",
			"merchantlist_temp",
			"object_contents",
			"raid_details",
			"raid_leaders",
			"raid_members",
			"respawn_times",
			"spell_buckets",
			"spell_globals",
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

}

#endif //EQEMU_DATABASE_SCHEMA_H
