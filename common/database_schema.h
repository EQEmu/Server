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

namespace DatabaseSchema {

	/**
	 * Gets player tables
	 *
	 * @return
	 */
	static std::vector<std::string> GetPlayerTables()
	{
		std::vector<std::string> tables = {
			"adventure_stats",
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
			"character_pet_inventory",
			"character_potionbelt",
			"character_skills",
			"character_spells",
			"character_tribute",
			"completed_tasks",
			"faction_values",
			"friends",
			"guild_members",
			"instance_list_player",
			"inventory",
			"inventory_snapshots",
			"keyring",
			"mail",
			"player_titlesets",
			"quest_globals",
			"timers",
			"titles",
			"zone_flags"
		};

		return tables;
	}

	/**
	 * Gets content tables
	 *
	 * @return
	 */
	static std::vector<std::string> GetContentTables()
	{
		std::vector<std::string> tables = {
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
			"data_buckets",
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

		return tables;
	}

	/**
	 * Gets server tables
	 *
	 * @return
	 */
	static std::vector<std::string> GetServerTables()
	{
		std::vector<std::string> tables = {
			"bug_reports",
			"db_str",
			"eqtime",
			"level_exp_mods",
			"logsys_categories",
			"name_filter",
			"perl_event_export_settings",
			"profanity_list",
			"saylink"
		};

		return tables;
	}

}

#endif //EQEMU_DATABASE_SCHEMA_H
