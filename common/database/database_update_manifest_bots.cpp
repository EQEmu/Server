#include "database_update.h"

std::vector<ManifestEntry> bot_manifest_entries = {
	ManifestEntry{
		.version = 9035,
		.description = "2022_12_04_bot_archery.sql",
		.check = "SHOW COLUMNS FROM `bot_data` LIKE 'archery_setting'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `bot_data`
ADD COLUMN `archery_setting` TINYINT(2) UNSIGNED NOT NULL DEFAULT '0' AFTER `enforce_spell_settings`;
)",
	},
	ManifestEntry{
		.version = 9036,
		.description = "2023_01_19_drop_bot_views.sql",
		.check = "SHOW TABLES LIKE 'vw_groups'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
DROP VIEW vw_bot_groups;
DROP VIEW vw_bot_character_mobs;
DROP VIEW vw_groups;
DROP VIEW vw_guild_members;
DROP TABLE bot_guild_members;

)",
	},
	ManifestEntry{
		.version = 9037,
		.description = "2023_01_22_add_name_index.sql",
		.check = "show index from bot_data WHERE key_name = 'name`",
		.condition = "",
		.match = "empty",
		.sql = R"(
create index `name` on bot_data(`name`);
)",
	},
	ManifestEntry{
		.version = 9038,
		.description = "2023_02_16_add_caster_range.sql",
		.check = "SHOW COLUMNS FROM `bot_data` LIKE 'caster_range'",
		.condition = "",
		.match = "empty",
		.sql = R"(
ALTER TABLE `bot_data`
ADD COLUMN `caster_range` INT(11) UNSIGNED NOT NULL DEFAULT '300' AFTER `archery_setting`;
)",
	},
	ManifestEntry{
		.version = 9039,
		.description = "2023_03_31_remove_bot_groups.sql",
		.check = "SHOW TABLES LIKE 'bot_groups'",
		.condition = "",
		.match = "not_empty",
		.sql = R"(
SET FOREIGN_KEY_CHECKS = 0;
DROP TABLE IF EXISTS `bot_groups`;
DROP TABLE IF EXISTS `bot_group_members`;
SET FOREIGN_KEY_CHECKS = 1;
)",
	},
// -- template; copy/paste this when you need to create a new entry
//	ManifestEntry{
//		.version = 9228,
//		.description = "some_new_migration.sql",
//		.check = "SHOW COLUMNS FROM `table_name` LIKE 'column_name'",
//		.condition = "empty",
//		.match = "",
//		.sql = R"(
//
//)"
};

// see struct definitions for what each field does
// struct ManifestEntry {
// 	int         version{};     // database version of the migration
// 	std::string description{}; // description of the migration ex: "add_new_table" or "add_index_to_table"
// 	std::string check{};       // query that checks against the condition
// 	std::string condition{};   // condition or "match_type" - Possible values [contains|match|missing|empty|not_empty]
// 	std::string match{};       // match field that is not always used, but works in conjunction with "condition" values [missing|match|contains]
// 	std::string sql{};         // the SQL DDL that gets ran when the condition is true
// };
