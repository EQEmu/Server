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
	ManifestEntry{
		.version = 9040,
		.description = "2023_11_16_bot_starting_items.sql",
		.check = "SHOW TABLES LIKE 'bot_starting_items'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `bot_starting_items`  (
`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
`races` int(11) UNSIGNED NOT NULL DEFAULT 0,
`classes` int(11) UNSIGNED NOT NULL DEFAULT 0,
`item_id` int(11) UNSIGNED NOT NULL DEFAULT 0,
`item_charges` tinyint(3) UNSIGNED NOT NULL DEFAULT 1,
`min_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
`slot_id` mediumint(9) NOT NULL DEFAULT -1,
`min_expansion` tinyint(4) NOT NULL DEFAULT -1,
`max_expansion` tinyint(4) NOT NULL DEFAULT -1,
`content_flags` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
`content_flags_disabled` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
PRIMARY KEY (`id`)
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci;
)",
	},
	ManifestEntry{
		.version = 9041,
		.description = "2023_12_04_bot_timers.sql",
		.check = "SHOW COLUMNS FROM `bot_timers` LIKE 'recast_time'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `bot_timers`
	ADD COLUMN `recast_time` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `timer_value`,
	ADD COLUMN `is_spell` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0 AFTER `recast_time`,
	ADD COLUMN `is_disc` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0 AFTER `is_spell`,
	ADD COLUMN `spell_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `is_disc`,
	ADD COLUMN `is_item` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0 AFTER `spell_id`,
	ADD COLUMN `item_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `is_item`;
ALTER TABLE `bot_timers`
	DROP FOREIGN KEY `FK_bot_timers_1`;
ALTER TABLE `bot_timers`
	DROP PRIMARY KEY;
ALTER TABLE `bot_timers`
	ADD PRIMARY KEY (`bot_id`, `timer_id`, `spell_id`, `item_id`);
)"
	},
	ManifestEntry{
		.version = 9042,
		.description = "2024_01_27_delete_bot_foreign_keys.sql",
		.check = "SHOW CREATE TABLE `bot_stances`",
		.condition = "contains",
		.match = "FOREIGN",
		.sql = R"(
ALTER TABLE `bot_buffs` DROP FOREIGN KEY `FK_bot_buffs_1`;
ALTER TABLE `bot_heal_rotations` DROP FOREIGN KEY `FK_bot_heal_rotations`;
ALTER TABLE `bot_heal_rotation_members` DROP FOREIGN KEY `FK_bot_heal_rotation_members_1`;
ALTER TABLE `bot_heal_rotation_members` DROP FOREIGN KEY `FK_bot_heal_rotation_members_2`;
ALTER TABLE `bot_heal_rotation_targets` DROP FOREIGN KEY `FK_bot_heal_rotation_targets`;
ALTER TABLE `bot_inventories` DROP FOREIGN KEY `FK_bot_inventories_1`;
ALTER TABLE `bot_pets` DROP FOREIGN KEY `FK_bot_pets_1`;
ALTER TABLE `bot_pet_buffs` DROP FOREIGN KEY `FK_bot_pet_buffs_1`;
ALTER TABLE `bot_pet_inventories` DROP FOREIGN KEY `FK_bot_pet_inventories_1`;
ALTER TABLE `bot_stances` DROP FOREIGN KEY `FK_bot_stances_1`;
)"
	},
	ManifestEntry{
		.version = 9043,
		.description = "2024_02_18_bot_starting_items_augments.sql",
		.check = "SHOW COLUMNS FROM `bot_starting_items` LIKE 'augment_one'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `bot_starting_items`
ADD COLUMN `augment_one` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `item_charges`,
ADD COLUMN `augment_two` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_one`,
ADD COLUMN `augment_three` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_two`,
ADD COLUMN `augment_four` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_three`,
ADD COLUMN `augment_five` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_four`,
ADD COLUMN `augment_six` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `augment_five`;
)"
	}
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
