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
	},
	ManifestEntry{
		.version = 9044,
		.description = "2024_04_23_bot_extra_haste.sql",
		.check = "SHOW COLUMNS FROM `bot_data` LIKE 'extra_haste'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `bot_data`
ADD COLUMN `extra_haste` mediumint(8) NOT NULL DEFAULT 0 AFTER `wis`;
)"
	},
	ManifestEntry{
		.version = 9045,
		.description = "2024_08_05_bot_spells_entries_unsigned_spell_id.sql",
		.check = "SHOW COLUMNS FROM `bot_spells_entries` LIKE 'spell_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `bot_spells_entries`
CHANGE COLUMN `spellid` `spell_id` smallint(5) UNSIGNED NOT NULL DEFAULT 0 AFTER `npc_spells_id`;
)"
	},
	ManifestEntry{
		.version = 9046,
		.description = "2024_05_18_bot_settings.sql",
		.check = "SHOW TABLES LIKE 'bot_settings'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `bot_settings` (
	`char_id` INT(10) UNSIGNED NOT NULL,
	`bot_id` INT(10) UNSIGNED NOT NULL,
	`stance` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`setting_id` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
	`setting_type` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`value` INT(10) UNSIGNED NOT NULL,
	`category_name` VARCHAR(64) NULL DEFAULT '' COLLATE 'utf8mb4_general_ci',
	`setting_name` VARCHAR(64) NULL DEFAULT '' COLLATE 'utf8mb4_general_ci',
	PRIMARY KEY (`char_id`, `bot_id`, `stance`, `setting_id`, `setting_type`) USING BTREE
)
COLLATE='utf8mb4_general_ci'
ENGINE=InnoDB
;

INSERT INTO bot_settings SELECT 0, bd.`bot_id`, (SELECT bs.`stance_id` FROM bot_stances bs WHERE bs.`bot_id` = bd.`bot_id`) AS stance_id, 0, 0, bd.`expansion_bitmask`, 'BaseSetting', 'ExpansionBitmask' FROM bot_data bd
JOIN rule_values rv
WHERE rv.rule_name LIKE 'Bots:BotExpansionSettings'
AND bd.expansion_bitmask != rv.rule_value;

INSERT INTO bot_settings SELECT 0, `bot_id`, (SELECT bs.`stance_id` FROM bot_stances bs WHERE bs.`bot_id` = bd.`bot_id`) AS stance_id, 1, 0, `show_helm`, 'BaseSetting', 'ShowHelm' FROM bot_data WHERE `show_helm` != 1;
INSERT INTO bot_settings SELECT 0, `bot_id`, (SELECT bs.`stance_id` FROM bot_stances bs WHERE bs.`bot_id` = bd.`bot_id`) AS stance_id, 2, 0, sqrt(`follow_distance`), 'BaseSetting', 'FollowDistance' FROM bot_data WHERE `follow_distance` != 184;

INSERT INTO bot_settings 
SELECT 0, `bot_id`, (SELECT bs.`stance_id` FROM bot_stances bs WHERE bs.`bot_id` = bd.`bot_id`) AS stance_id, 3, 0, `stop_melee_level`, 'BaseSetting', 'StopMeleeLevel'
FROM (
    SELECT `bot_id`, 
           (CASE 
               WHEN (`class` IN (2, 6, 10, 11, 12, 13, 14)) THEN 13
               ELSE 255
           END) AS `sml`,
           `stop_melee_level`
    FROM bot_data
) AS `subquery`
WHERE `sml` != `stop_melee_level`;

INSERT INTO bot_settings SELECT 0, `bot_id`, (SELECT bs.`stance_id` FROM bot_stances bs WHERE bs.`bot_id` = bd.`bot_id`) AS stance_id, 4, 0, `enforce_spell_settings`, 'BaseSetting', 'EnforceSpellSettings' FROM bot_data WHERE `enforce_spell_settings` != 0;
INSERT INTO bot_settings SELECT 0, `bot_id`, (SELECT bs.`stance_id` FROM bot_stances bs WHERE bs.`bot_id` = bd.`bot_id`) AS stance_id, 5, 0, `archery_setting`, 'BaseSetting', 'RangedSetting' FROM bot_data WHERE `archery_setting` != 0;

INSERT INTO bot_settings
SELECT 0, `bot_id`, (SELECT bs.`stance_id` FROM bot_stances bs WHERE bs.`bot_id` = bd.`bot_id`) AS stance_id, 8, 0, `caster_range`, 'BaseSetting', 'DistanceRanged'
FROM (
    SELECT `bot_id`, 
           (CASE 
               WHEN (`class` IN (1, 7, 19, 16)) THEN 0
                WHEN `class` = 8 THEN 0
               ELSE 90
           END) AS `DistanceRanged`,
           `caster_range`
    FROM bot_data
) AS `subquery`
WHERE `DistanceRanged` != `caster_range`;

ALTER TABLE `bot_data`
	DROP COLUMN `show_helm`;
ALTER TABLE `bot_data`
	DROP COLUMN `follow_distance`;
ALTER TABLE `bot_data`
	DROP COLUMN `stop_melee_level`;
ALTER TABLE `bot_data`
	DROP COLUMN `expansion_bitmask`;
ALTER TABLE `bot_data`
	DROP COLUMN `enforce_spell_settings`;
ALTER TABLE `bot_data`
	DROP COLUMN `archery_setting`;
ALTER TABLE `bot_data`
	DROP COLUMN `caster_range`;

UPDATE `bot_command_settings` SET `aliases`= 'bh' WHERE `bot_command`='behindmob';
UPDATE `bot_command_settings` SET `aliases`= 'bs|settings' WHERE `bot_command`='botsettings';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|followdistance') ELSE 'followd||followdistance' END WHERE `bot_command`='botfollowdistance' AND `aliases` NOT LIKE '%followdistance%';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|ranged|toggleranged|btr') ELSE 'ranged|toggleranged|btr' END WHERE `bot_command`='bottoggleranged' AND `aliases` NOT LIKE '%ranged|toggleranged|btr%';
UPDATE `bot_command_settings` SET `aliases`= 'distranged|dr' WHERE `bot_command`='distanceranged';
UPDATE `bot_command_settings` SET `aliases`= 'copy' WHERE `bot_command`='copysettings';
UPDATE `bot_command_settings` SET `aliases`= 'default' WHERE `bot_command`='defaultsettings';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|enforce') ELSE 'enforce' END WHERE `bot_command`='enforcespellsettings' AND `aliases` NOT LIKE '%enforce%';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|ib') ELSE 'ib' END WHERE `bot_command`='illusionblock' AND `aliases` NOT LIKE '%ib%';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|ig') ELSE 'invgive|ig' END WHERE `bot_command`='inventorygive' AND `aliases` NOT LIKE '%ig%';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|il') ELSE 'invlist|il' END WHERE `bot_command`='inventorylist' AND `aliases` NOT LIKE '%il%';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|ir') ELSE 'invremove|ir' END WHERE `bot_command`='inventoryremove' AND `aliases` NOT LIKE '%ir%';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|iw') ELSE 'invwindow|iw' END WHERE `bot_command`='inventorywindow' AND `aliases` NOT LIKE '%iw%';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|iu') ELSE 'iu' END WHERE `bot_command`='itemuse' AND `aliases` NOT LIKE '%iu%';
UPDATE `bot_command_settings` SET `aliases`= 'mmr' WHERE `bot_command`='maxmeleerange';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|pp') ELSE 'pp' END WHERE `bot_command`='pickpocket' AND `aliases` NOT LIKE '%pp%';
UPDATE `bot_command_settings` SET `aliases`= 'sithp' WHERE `bot_command`='sithppercent';
UPDATE `bot_command_settings` SET `aliases`= 'sitcombat' WHERE `bot_command`='sitincombat';
UPDATE `bot_command_settings` SET `aliases`= 'sitmana' WHERE `bot_command`='sitmanapercent';
UPDATE `bot_command_settings` SET `aliases`= 'aggrochecks' WHERE `bot_command`='spellaggrochecks';
UPDATE `bot_command_settings` SET `aliases`= 'delays' WHERE `bot_command`='spelldelays';
UPDATE `bot_command_settings` SET `aliases`= 'engagedpriority' WHERE `bot_command`='spellengagedpriority';
UPDATE `bot_command_settings` SET `aliases`= 'holds' WHERE `bot_command`='spellholds';
UPDATE `bot_command_settings` SET `aliases`= 'idlepriority' WHERE `bot_command`='spellidlepriority';
UPDATE `bot_command_settings` SET `aliases`= 'maxhp' WHERE `bot_command`='spellmaxhppct';
UPDATE `bot_command_settings` SET `aliases`= 'maxmana' WHERE `bot_command`='spellmaxmanapct';
UPDATE `bot_command_settings` SET `aliases`= 'maxthresholds' WHERE `bot_command`='spellmaxthresholds';
UPDATE `bot_command_settings` SET `aliases`= 'minhp' WHERE `bot_command`='spellminhppct';
UPDATE `bot_command_settings` SET `aliases`= 'minmana' WHERE `bot_command`='spellminmanapct';
UPDATE `bot_command_settings` SET `aliases`= 'minthresholds' WHERE `bot_command`='spellminthresholds';
UPDATE `bot_command_settings` SET `aliases`= 'pursuepriority' WHERE `bot_command`='spellpursuepriority';
UPDATE `bot_command_settings` SET `aliases`= 'targetcount' WHERE `bot_command`='spelltargetcount';
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|vc') ELSE 'vc' END WHERE `bot_command`='viewcombos' AND `aliases` NOT LIKE '%vc%';
)"
	},
	ManifestEntry{
		.version = 9047,
		.description = "2024_05_18_bot_update_spell_types.sql",
		.check = "SELECT * FROM `bot_spells_entries` WHERE `type` > 21",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
UPDATE `bot_spells_entries` SET `type` = 0 WHERE `type` = 1;
UPDATE `bot_spells_entries` SET `type` = 1 WHERE `type` = 2;
UPDATE `bot_spells_entries` SET `type` = 2 WHERE `type` = 4;
UPDATE `bot_spells_entries` SET `type` = 3 WHERE `type` = 8;
UPDATE `bot_spells_entries` SET `type` = 4 WHERE `type` = 16;
UPDATE `bot_spells_entries` SET `type` = 5 WHERE `type` = 32;
UPDATE `bot_spells_entries` SET `type` = 6 WHERE `type` = 64;
UPDATE `bot_spells_entries` SET `type` = 7 WHERE `type` = 128;
UPDATE `bot_spells_entries` SET `type` = 8 WHERE `type` = 256;
UPDATE `bot_spells_entries` SET `type` = 9 WHERE `type` = 512;
UPDATE `bot_spells_entries` SET `type` = 10 WHERE `type` = 1024;
UPDATE `bot_spells_entries` SET `type` = 11 WHERE `type` = 2048;
UPDATE `bot_spells_entries` SET `type` = 12 WHERE `type` = 4096;
UPDATE `bot_spells_entries` SET `type` = 13 WHERE `type` = 8192;
UPDATE `bot_spells_entries` SET `type` = 14 WHERE `type` = 16384;
UPDATE `bot_spells_entries` SET `type` = 15 WHERE `type` = 32768;
UPDATE `bot_spells_entries` SET `type` = 16 WHERE `type` = 65536;
UPDATE `bot_spells_entries` SET `type` = 17 WHERE `type` = 131072;
UPDATE `bot_spells_entries` SET `type` = 18 WHERE `type` = 262144;
UPDATE `bot_spells_entries` SET `type` = 19 WHERE `type` = 524288;
UPDATE `bot_spells_entries` SET `type` = 20 WHERE `type` = 1048576;
UPDATE `bot_spells_entries` SET `type` = 21 WHERE `type` = 2097152;
)"
	},
	ManifestEntry{
		.version = 9048,
		.description = "2024_05_18_bot_fear_spell_type.sql",
		.check = "SELECT * FROM `bot_spells_entries` where `type` = 22",
		.condition = "empty",
		.match = "",
		.sql = R"(
UPDATE bot_spells_entries b, spells_new s
SET b.`type` = 22
WHERE b.spell_id = s.id
AND (
	s.`effectid1` = 23 OR
	s.`effectid2` = 23 OR
	s.`effectid3` = 23 OR
	s.`effectid4` = 23 OR
	s.`effectid5` = 23 OR
	s.`effectid6` = 23 OR
	s.`effectid7` = 23 OR
	s.`effectid8` = 23 OR
	s.`effectid9` = 23 OR
	s.`effectid10` = 23 OR
	s.`effectid11` = 23 OR
	s.`effectid12` = 23
	);
)"
	},
	ManifestEntry{
		.version = 9049,
		.description = "2024_05_18_correct_bot_spell_entries_types.sql",
		.check = "SELECT * FROM `bot_spells_entries` where `npc_spells_id` = 3002 AND `spell_id` = 14312",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Class fixes
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3002 WHERE b.`spell_id` = 14312;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3002 WHERE b.`spell_id` = 14313;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3002 WHERE b.`spell_id` = 14314;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3005 WHERE b.`spell_id` = 15186;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3005 WHERE b.`spell_id` = 15187;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3005 WHERE b.`spell_id` = 15188;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spell_id` = 14446;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spell_id` = 14447;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spell_id` = 14467;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spell_id` = 14468;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spell_id` = 14469;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3003 WHERE b.`spell_id` = 14955;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3003 WHERE b.`spell_id` = 14956;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spell_id` = 14387;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spell_id` = 14388;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spell_id` = 14389;

-- Minlevel fixes
UPDATE bot_spells_entries SET `minlevel` = 34 WHERE `spell_id` = 1445 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 2 WHERE `spell_id` = 229 AND `npc_spells_id` = 3011;
UPDATE bot_spells_entries SET `minlevel` = 13 WHERE `spell_id` = 333 AND `npc_spells_id` = 3013;
UPDATE bot_spells_entries SET `minlevel` = 29 WHERE `spell_id` = 106 AND `npc_spells_id` = 3013;
UPDATE bot_spells_entries SET `minlevel` = 38 WHERE `spell_id` = 754 AND `npc_spells_id` = 3010;
UPDATE bot_spells_entries SET `minlevel` = 58 WHERE `spell_id` = 2589 AND `npc_spells_id` = 3003;
UPDATE bot_spells_entries SET `minlevel` = 67 WHERE `spell_id` = 5305 AND `npc_spells_id` = 3004;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 14267 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 14268 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 14269 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 23 WHERE `spell_id` = 738 AND `npc_spells_id` = 3008;
UPDATE bot_spells_entries SET `minlevel` = 51 WHERE `spell_id` = 1751 AND `npc_spells_id` = 3008;
UPDATE bot_spells_entries SET `minlevel` = 7 WHERE `spell_id` = 734 AND `npc_spells_id` = 3008;
UPDATE bot_spells_entries SET `minlevel` = 5 WHERE `spell_id` = 717 AND `npc_spells_id` = 3008;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 15186 AND `npc_spells_id` = 3005;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 15187 AND `npc_spells_id` = 3005;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 15188 AND `npc_spells_id` = 3005;
UPDATE bot_spells_entries SET `minlevel` = 80 WHERE `spell_id` = 14446 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 80 WHERE `spell_id` = 14447 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 14467 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 14468 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spell_id` = 14469 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spell_id` = 14955 AND `npc_spells_id` = 3003;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spell_id` = 14956 AND `npc_spells_id` = 3003;
UPDATE bot_spells_entries SET `minlevel` = 78 WHERE `spell_id` = 14387 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spell_id` = 14388 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spell_id` = 14389 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spell_id` = 14312 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spell_id` = 14313 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spell_id` = 14314 AND `npc_spells_id` = 3002;

-- Maxlevel fixes
UPDATE bot_spells_entries SET `maxlevel` = 83 WHERE `spell_id` = 14267 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 83 WHERE `spell_id` = 14268 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 83 WHERE `spell_id` = 14269 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spell_id` = 14446 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spell_id` = 14447 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spell_id` = 14467 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spell_id` = 14468 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spell_id` = 14469 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 81 WHERE `spell_id` = 14312 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 81 WHERE `spell_id` = 14313 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 81 WHERE `spell_id` = 14314 AND `npc_spells_id` = 3002;

-- Type fixes
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 201;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spell_id` = 752;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spell_id` = 2117;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 2542;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 2544;
UPDATE bot_spells_entries SET `type` = 6 WHERE `spell_id` = 2115;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 1403;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 1405;
UPDATE bot_spells_entries SET `type` = 9 WHERE `spell_id` = 289;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 294;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 302;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 521;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 185;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 450;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 186;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 4074;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 195;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 1712;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 1703;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spell_id` = 3229;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 3345;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 5509;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 6826;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 270;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 281;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 505;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 526;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 110;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 506;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 162;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 111;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 507;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 527;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 163;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 112;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 1588;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 1573;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 1592;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 1577;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 1578;
UPDATE bot_spells_entries SET `type` = 1 WHERE `spell_id` = 1576;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 3386;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 3387;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 4900;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 3395;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 5394;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 5392;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 6827;
UPDATE bot_spells_entries SET `type` = 1 WHERE `spell_id` = 5416;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 1437;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 1436;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 5348;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 8008;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 2571;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 370;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spell_id` = 1741;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spell_id` = 1296;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 270;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 2634;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 2942;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 3462;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spell_id` = 6828;
UPDATE bot_spells_entries SET `type` = 4 WHERE `spell_id` = 14312;
UPDATE bot_spells_entries SET `type` = 4 WHERE `spell_id` = 14313;
UPDATE bot_spells_entries SET `type` = 4 WHERE `spell_id` = 14314;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 18392;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 18393;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 18394;
UPDATE bot_spells_entries SET `type` = 10 WHERE `spell_id` = 15186;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 15187;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 15188;
UPDATE bot_spells_entries SET `type` = 1 WHERE `spell_id` = 14446;
UPDATE bot_spells_entries SET `type` = 1 WHERE `spell_id` = 14447;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 14467;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 14468;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 14469;
UPDATE bot_spells_entries SET `type` = 0 WHERE `spell_id` = 14267;
UPDATE bot_spells_entries SET `type` = 0 WHERE `spell_id` = 14268;
UPDATE bot_spells_entries SET `type` = 0 WHERE `spell_id` = 14269;
UPDATE bot_spells_entries SET `type` = 10 WHERE `spell_id` = 14955;
UPDATE bot_spells_entries SET `type` = 10 WHERE `spell_id` = 14956;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 14387;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 14388;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spell_id` = 14389;
UPDATE bot_spells_entries SET `type` = 4 WHERE `spell_id` = 10436;

-- UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 3440; -- Ro's Illumination [#3440] from DoT [#8] to Debuff [#14] [Should be 0]
-- UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 303; -- Whirl till you hurl [#303] from Nuke [#0] to Debuff [#14] [Should be 0]
-- UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 619; -- Dyn's Dizzying Draught [#619] from Nuke [#0] to Debuff [#14] [Should be 0]

-- UPDATE bot_spells_entries SET `type` = 14 WHERE `spell_id` = 74; -- Mana Sieve [#74] from Nuke [#0] to Debuff [#14]
-- UPDATE bot_spells_entries SET `type` = 6 WHERE `spell_id` = 1686; -- Theft of Thought [#1686] from Nuke [#0] to Lifetap [#6]

-- UPDATE bot_spells_entries SET `type` = 1 WHERE `spell_id` = 3694; -- Stoicism [#3694] from In-Combat Buff [#10] to Regular Heal [#1]
-- UPDATE bot_spells_entries SET `type` = 1 WHERE `spell_id` = 4899; -- Breath of Trushar [#4899] from In-Combat Buff [#10] to Regular Heal [#1]

-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spell_id` = 738; -- Selo's Consonant Chain [#738] from Slow [#13] to Snare [#7]
-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spell_id` = 1751; -- Largo's Assonant Binding [#1751] from Slow [#13] to Snare [#7]
-- UPDATE bot_spells_entries SET `type` = 8 WHERE `spell_id` = 1748; -- Angstlich's Assonance [#1748] from Slow [#13] to DoT [#8]
-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spell_id` = 738; -- Selo's Consonant Chain [#738] from Slow [#13] to Snare [#7]
-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spell_id` = 1751; -- Largo's Assonant Binding [#1751] from Slow [#13] to Snare [#7]
-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spell_id` = 738; -- Selo's Consonant Chain [#738] from Slow [#13] to Snare [#7]

)"
	},
	ManifestEntry{
		.version = 9050,
		.description = "2024_11_26_add_commanded_spelltypes.sql",
		.check = "SELECT * FROM `bot_spells_entries` where `type` = 100",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `bot_spells_entries` (`npc_spells_id`, `spell_id`, `type`, `minlevel`, `maxlevel`) 
VALUES
(3006, 9957, 100, 20, 254),
(3006, 9956, 100, 20, 254),
(3006, 552, 100, 25, 254),
(3006, 550, 100, 25, 254),
(3006, 553, 100, 25, 254),
(3006, 2432, 100, 26, 254),
(3006, 2020, 100, 26, 254),
(3006, 551, 100, 27, 254),
(3006, 3792, 100, 28, 254),
(3006, 2419, 100, 29, 254),
(3006, 554, 100, 30, 254),
(3006, 557, 100, 31, 254),
(3006, 1434, 100, 32, 254),
(3006, 555, 100, 32, 254),
(3006, 25898, 100, 32, 254),
(3006, 25904, 100, 32, 254),
(3006, 556, 100, 32, 254),
(3006, 25698, 100, 33, 254),
(3006, 1517, 100, 33, 254),
(3006, 2424, 100, 33, 254),
(3006, 25689, 100, 33, 254),
(3006, 25899, 100, 34, 254),
(3006, 25690, 100, 35, 254),
(3006, 25903, 100, 35, 254),
(3006, 25900, 100, 35, 254),
(3006, 558, 100, 36, 254),
(3006, 2429, 100, 37, 254),
(3006, 1438, 100, 38, 254),
(3006, 3184, 100, 38, 254),
(3006, 25697, 100, 38, 254),
(3006, 25902, 100, 39, 254),
(3006, 25695, 100, 39, 254),
(3006, 25901, 100, 40, 254),
(3006, 25694, 100, 40, 254),
(3006, 1398, 100, 40, 254),
(3006, 25905, 100, 41, 254),
(3006, 25696, 100, 42, 254),
(3006, 1440, 100, 42, 254),
(3006, 25906, 100, 43, 254),
(3006, 25693, 100, 44, 254),
(3006, 25699, 100, 45, 254),
(3006, 24773, 100, 46, 254),
(3006, 8965, 100, 52, 254),
(3006, 24771, 100, 52, 254),
(3006, 8235, 100, 52, 254),
(3006, 24775, 100, 52, 254),
(3006, 4966, 100, 54, 254),
(3006, 6184, 100, 55, 254),
(3006, 5731, 100, 55, 254),
(3006, 24776, 100, 56, 254),
(3006, 25700, 100, 56, 254),
(3006, 25691, 100, 57, 254),
(3006, 24772, 100, 57, 254),
(3006, 25692, 100, 57, 254),
(3006, 11981, 100, 59, 254),
(3006, 9953, 100, 60, 254),
(3006, 9954, 100, 60, 254),
(3006, 11980, 100, 64, 254),
(3006, 6179, 100, 64, 254),
(3006, 24774, 100, 67, 254),
(3006, 9950, 100, 70, 254),
(3006, 9951, 100, 70, 254),
(3006, 15886, 100, 75, 254),
(3006, 15887, 100, 75, 254),
(3006, 21989, 100, 80, 254),
(3006, 20539, 100, 80, 254),
(3006, 21984, 100, 80, 254),
(3006, 20538, 100, 80, 254),
(3006, 17883, 100, 85, 254),
(3006, 17884, 100, 85, 254),
(3006, 28997, 100, 90, 254),
(3006, 28998, 100, 90, 254),
(3006, 29000, 100, 92, 254),
(3006, 29001, 100, 92, 254),
(3006, 34832, 100, 95, 254),
(3006, 40217, 100, 95, 254),
(3006, 34833, 100, 95, 254),
(3006, 40216, 100, 95, 254),
(3012, 10881, 100, 20, 254),
(3012, 10880, 100, 20, 254),
(3012, 562, 100, 25, 254),
(3012, 563, 100, 27, 254),
(3012, 3793, 100, 27, 254),
(3012, 561, 100, 28, 254),
(3012, 2420, 100, 29, 254),
(3012, 2944, 100, 29, 254),
(3012, 564, 100, 32, 254),
(3012, 565, 100, 33, 254),
(3012, 1418, 100, 33, 254),
(3012, 2425, 100, 33, 254),
(3012, 1516, 100, 34, 254),
(3012, 1338, 100, 35, 254),
(3012, 3833, 100, 35, 254),
(3012, 566, 100, 35, 254),
(3012, 1336, 100, 36, 254),
(3012, 2943, 100, 36, 254),
(3012, 1423, 100, 36, 254),
(3012, 567, 100, 36, 254),
(3012, 568, 100, 37, 254),
(3012, 1337, 100, 37, 254),
(3012, 3180, 100, 38, 254),
(3012, 1339, 100, 38, 254),
(3012, 2421, 100, 39, 254),
(3012, 2430, 100, 39, 254),
(3012, 1372, 100, 40, 254),
(3012, 2426, 100, 41, 254),
(3012, 1371, 100, 41, 254),
(3012, 1399, 100, 42, 254),
(3012, 1374, 100, 42, 254),
(3012, 1373, 100, 43, 254),
(3012, 1425, 100, 43, 254),
(3012, 1375, 100, 44, 254),
(3012, 3181, 100, 45, 254),
(3012, 2022, 100, 45, 254),
(3012, 666, 100, 46, 254),
(3012, 3849, 100, 46, 254),
(3012, 674, 100, 46, 254),
(3012, 2023, 100, 46, 254),
(3012, 2024, 100, 47, 254),
(3012, 2025, 100, 48, 254),
(3012, 2431, 100, 49, 254),
(3012, 8966, 100, 51, 254),
(3012, 8236, 100, 51, 254),
(3012, 4965, 100, 54, 254),
(3012, 8969, 100, 55, 254),
(3012, 8239, 100, 55, 254),
(3012, 6183, 100, 55, 254),
(3012, 5732, 100, 55, 254),
(3012, 4964, 100, 57, 254),
(3012, 6182, 100, 58, 254),
(3012, 5735, 100, 60, 254),
(3012, 10877, 100, 60, 254),
(3012, 10878, 100, 60, 254),
(3012, 6178, 100, 64, 254),
(3012, 6177, 100, 67, 254),
(3012, 11984, 100, 69, 254),
(3012, 10874, 100, 70, 254),
(3012, 10875, 100, 70, 254),
(3012, 11983, 100, 74, 254),
(3012, 15889, 100, 75, 254),
(3012, 15890, 100, 75, 254),
(3012, 21988, 100, 80, 254),
(3012, 20542, 100, 80, 254),
(3012, 21985, 100, 80, 254),
(3012, 20541, 100, 80, 254),
(3012, 17886, 100, 85, 254),
(3012, 17887, 100, 85, 254),
(3012, 29840, 100, 90, 254),
(3012, 29841, 100, 90, 254),
(3012, 29843, 100, 92, 254),
(3012, 29844, 100, 92, 254),
(3012, 40443, 100, 95, 254),
(3012, 35715, 100, 95, 254),
(3012, 40442, 100, 95, 254),
(3012, 35714, 100, 95, 254),
(3002, 208, 101, 1, 4),
(3002, 501, 101, 5, 14),
(3002, 47, 101, 15, 35),
(3002, 45, 101, 36, 64),
(3002, 1541, 101, 55, 254),
(3002, 3197, 101, 65, 254),
(3002, 5274, 101, 70, 70),
(3002, 9798, 101, 71, 75),
(3002, 9799, 101, 71, 75),
(3002, 9797, 101, 71, 75),
(3002, 14288, 101, 76, 80),
(3002, 14289, 101, 76, 80),
(3002, 14290, 101, 76, 80),
(3002, 18309, 101, 81, 85),
(3002, 18310, 101, 81, 85),
(3002, 18311, 101, 81, 85),
(3002, 25103, 101, 86, 90),
(3002, 25101, 101, 86, 90),
(3002, 25102, 101, 86, 90),
(3002, 28102, 101, 91, 95),
(3002, 28100, 101, 91, 95),
(3002, 28101, 101, 91, 95),
(3002, 34096, 101, 96, 254),
(3002, 34094, 101, 96, 254),
(3002, 34095, 101, 96, 254),
(3003, 208, 101, 10, 24),
(3003, 501, 101, 25, 42),
(3003, 47, 101, 43, 48),
(3003, 45, 101, 49, 254),
(3003, 25294, 101, 86, 90),
(3003, 25295, 101, 86, 90),
(3003, 25296, 101, 86, 90),
(3003, 28340, 101, 91, 95),
(3003, 28338, 101, 91, 95),
(3003, 28339, 101, 91, 95),
(3003, 34346, 101, 96, 254),
(3003, 34344, 101, 96, 254),
(3003, 34345, 101, 96, 254),
(3004, 240, 101, 4, 30),
(3004, 250, 101, 22, 254),
(3004, 513, 101, 31, 254),
(3004, 3601, 101, 39, 254),
(3004, 5316, 101, 68, 70),
(3004, 10112, 101, 71, 75),
(3004, 10110, 101, 71, 75),
(3004, 10111, 101, 71, 75),
(3004, 15037, 101, 76, 80),
(3004, 15035, 101, 76, 80),
(3004, 15036, 101, 76, 80),
(3004, 19168, 101, 81, 85),
(3004, 19169, 101, 81, 85),
(3004, 19167, 101, 81, 85),
(3004, 25417, 101, 86, 87),
(3004, 25418, 101, 86, 90),
(3004, 25419, 101, 86, 90),
(3004, 25466, 101, 88, 90),
(3004, 25467, 101, 88, 90),
(3004, 25465, 101, 88, 90),
(3004, 28479, 101, 91, 92),
(3004, 28480, 101, 91, 95),
(3004, 28481, 101, 91, 95),
(3004, 28542, 101, 93, 95),
(3004, 28543, 101, 93, 95),
(3004, 28544, 101, 93, 95),
(3004, 34500, 101, 96, 97),
(3004, 34502, 101, 96, 254),
(3004, 34501, 101, 96, 254),
(3004, 34565, 101, 98, 254),
(3004, 34563, 101, 98, 254),
(3004, 34564, 101, 98, 254),
(3005, 347, 101, 9, 51),
(3005, 448, 101, 52, 254),
(3006, 240, 101, 1, 14),
(3006, 250, 101, 5, 254),
(3006, 513, 101, 15, 254),
(3006, 3601, 101, 29, 254),
(3006, 5347, 101, 67, 70),
(3006, 9851, 101, 71, 75),
(3006, 9852, 101, 71, 75),
(3006, 9853, 101, 71, 75),
(3006, 14369, 101, 76, 80),
(3006, 14367, 101, 76, 80),
(3006, 14368, 101, 76, 80),
(3006, 18409, 101, 81, 85),
(3006, 18407, 101, 81, 85),
(3006, 18408, 101, 81, 85),
(3006, 25736, 101, 86, 90),
(3006, 25734, 101, 86, 90),
(3006, 25735, 101, 86, 90),
(3006, 28831, 101, 91, 95),
(3006, 28832, 101, 91, 95),
(3006, 28830, 101, 91, 95),
(3006, 34863, 101, 96, 254),
(3006, 34864, 101, 96, 254),
(3006, 34862, 101, 96, 254),
(3008, 728, 101, 8, 60),
(3008, 3361, 101, 61, 254),
(3008, 5370, 101, 66, 70),
(3008, 10403, 101, 71, 75),
(3008, 10401, 101, 71, 75),
(3008, 10402, 101, 71, 75),
(3008, 14002, 101, 76, 80),
(3008, 14000, 101, 76, 80),
(3008, 14001, 101, 76, 80),
(3008, 18001, 101, 81, 85),
(3008, 18002, 101, 81, 85),
(3008, 18000, 101, 81, 85),
(3008, 25978, 101, 86, 90),
(3008, 25979, 101, 86, 90),
(3008, 25977, 101, 86, 90),
(3008, 29079, 101, 91, 95),
(3008, 29080, 101, 91, 95),
(3008, 29078, 101, 91, 95),
(3008, 35131, 101, 96, 254),
(3008, 35132, 101, 96, 254),
(3008, 35133, 101, 96, 254),
(3011, 347, 101, 2, 22),
(3011, 448, 101, 23, 254),
(3014, 208, 101, 1, 5),
(3014, 501, 101, 6, 17),
(3014, 47, 101, 18, 34),
(3014, 45, 101, 35, 61),
(3014, 1541, 101, 51, 254),
(3014, 3197, 101, 62, 254),
(3014, 5506, 101, 67, 71),
(3014, 10601, 101, 72, 76),
(3014, 10599, 101, 72, 76),
(3014, 10600, 101, 72, 76),
(3014, 14510, 101, 77, 81),
(3014, 14511, 101, 77, 81),
(3014, 14509, 101, 77, 81),
(3014, 18568, 101, 82, 86),
(3014, 18569, 101, 82, 86),
(3014, 18567, 101, 82, 86),
(3014, 26921, 101, 87, 91),
(3014, 26922, 101, 87, 91),
(3014, 26920, 101, 87, 91),
(3014, 30054, 101, 92, 96),
(3014, 30055, 101, 92, 96),
(3014, 30056, 101, 92, 96),
(3014, 36116, 101, 97, 254),
(3014, 36117, 101, 97, 254),
(3014, 36118, 101, 97, 254),
(3006, 2183, 102, 18, 56),
(3006, 1567, 102, 57, 254),
(3012, 2184, 102, 18, 56),
(3012, 2558, 102, 56, 64),
(3012, 1628, 102, 57, 254),
(3012, 3244, 102, 65, 254),
(3002, 35, 103, 10, 254),
(3006, 35, 103, 12, 254),
(3010, 35, 103, 14, 254),
(3011, 35, 103, 12, 254),
(3012, 35, 103, 12, 254),
(3013, 35, 103, 12, 254),
(3014, 35, 103, 12, 254),
(3008, 737, 104, 14, 254),
(3011, 305, 104, 17, 254),
(3012, 305, 104, 14, 254),
(3013, 305, 104, 13, 254),
(3014, 305, 104, 15, 254),
(3004, 261, 105, 35, 64),
(3004, 2517, 105, 65, 254),
(3006, 261, 105, 14, 49),
(3006, 2894, 105, 50, 53),
(3006, 2517, 105, 54, 254),
(3006, 3185, 105, 62, 254),
(3008, 718, 105, 31, 50),
(3008, 1750, 105, 51, 254),
(3010, 261, 105, 10, 50),
(3010, 2894, 105, 51, 254),
(3011, 457, 105, 41, 254),
(3011, 1391, 105, 45, 254),
(3012, 261, 105, 22, 49),
(3012, 2894, 105, 50, 254),
(3014, 261, 105, 15, 50),
(3014, 2894, 105, 51, 254),
(3015, 261, 105, 32, 254),
(3003, 1743, 106, 55, 254),
(3008, 714, 106, 41, 254),
(3008, 748, 106, 47, 57),
(3008, 1450, 106, 49, 254),
(3008, 1752, 106, 52, 254),
(3008, 1763, 106, 58, 72),
(3008, 11881, 106, 73, 77),
(3008, 11879, 106, 73, 77),
(3008, 11880, 106, 73, 77),
(3008, 14055, 106, 78, 82),
(3008, 14056, 106, 78, 82),
(3008, 14054, 106, 78, 82),
(3008, 18040, 106, 83, 87),
(3008, 18041, 106, 83, 87),
(3008, 18039, 106, 83, 87),
(3008, 26026, 106, 88, 92),
(3008, 26027, 106, 88, 92),
(3008, 26025, 106, 88, 92),
(3008, 29120, 106, 93, 97),
(3008, 29121, 106, 93, 97),
(3008, 29122, 106, 93, 97),
(3008, 35170, 106, 98, 254),
(3008, 35171, 106, 98, 254),
(3008, 35172, 106, 98, 254),
(3012, 2559, 106, 58, 254),
(3014, 481, 106, 13, 21),
(3014, 21, 106, 19, 21),
(3014, 482, 106, 22, 32),
(3014, 483, 106, 33, 39),
(3014, 648, 106, 38, 39),
(3014, 484, 106, 40, 57),
(3014, 176, 106, 47, 57),
(3014, 1689, 106, 52, 57),
(3014, 1713, 106, 58, 60),
(3014, 3343, 106, 61, 66),
(3014, 6739, 106, 61, 68),
(3014, 3351, 106, 63, 66),
(3014, 5504, 106, 67, 70),
(3014, 5514, 106, 69, 70),
(3014, 6671, 106, 69, 254),
(3014, 10598, 106, 71, 75),
(3014, 10596, 106, 71, 75),
(3014, 10597, 106, 71, 75),
(3014, 10643, 106, 74, 75),
(3014, 10641, 106, 74, 75),
(3014, 10642, 106, 74, 75),
(3014, 11887, 106, 74, 78),
(3014, 11885, 106, 74, 78),
(3014, 11886, 106, 74, 78),
(3014, 14507, 106, 76, 80),
(3014, 14508, 106, 76, 80),
(3014, 14506, 106, 76, 80),
(3014, 14543, 106, 79, 80),
(3014, 14544, 106, 79, 80),
(3014, 14542, 106, 79, 80),
(3014, 14582, 106, 79, 83),
(3014, 14583, 106, 79, 83),
(3014, 14581, 106, 79, 83),
(3014, 18564, 106, 81, 85),
(3014, 18565, 106, 81, 85),
(3014, 18566, 106, 81, 85),
(3014, 18600, 106, 84, 85),
(3014, 18601, 106, 84, 85),
(3014, 18602, 106, 84, 85),
(3014, 18641, 106, 84, 88),
(3014, 18639, 106, 84, 88),
(3014, 18640, 106, 84, 88),
(3014, 26901, 106, 86, 90),
(3014, 26899, 106, 86, 90),
(3014, 26900, 106, 86, 90),
(3014, 26999, 106, 89, 90),
(3014, 26997, 106, 89, 90),
(3014, 26998, 106, 89, 90),
(3014, 27025, 106, 89, 93),
(3014, 27026, 106, 89, 93),
(3014, 27024, 106, 89, 93),
(3014, 30028, 106, 91, 95),
(3014, 30029, 106, 91, 95),
(3014, 30027, 106, 91, 95),
(3014, 30132, 106, 93, 95),
(3014, 30133, 106, 93, 95),
(3014, 30131, 106, 93, 95),
(3014, 30140, 106, 94, 95),
(3014, 30141, 106, 94, 95),
(3014, 30142, 106, 94, 95),
(3014, 30167, 106, 94, 98),
(3014, 30168, 106, 94, 98),
(3014, 30169, 106, 94, 98),
(3014, 36091, 106, 96, 254),
(3014, 36089, 106, 96, 254),
(3014, 36090, 106, 96, 254),
(3014, 36189, 106, 98, 254),
(3014, 36187, 106, 98, 254),
(3014, 36188, 106, 98, 254),
(3014, 36216, 106, 99, 254),
(3014, 36196, 106, 99, 254),
(3014, 36217, 106, 99, 254),
(3014, 36194, 106, 99, 254),
(3014, 36215, 106, 99, 254),
(3014, 36195, 106, 99, 254),
(3004, 86, 107, 20, 254),
(3006, 86, 107, 6, 49),
(3006, 2881, 107, 50, 254),
(3008, 729, 107, 16, 254),
(3010, 86, 107, 12, 50),
(3010, 2881, 107, 51, 254),
(3011, 457, 107, 41, 254),
(3011, 1391, 107, 45, 254),
(3014, 86, 107, 12, 43),
(3014, 3696, 107, 44, 50),
(3014, 2881, 107, 51, 254),
(3015, 86, 107, 25, 254),
(3010, 345, 108, 15, 254),
(3010, 2522, 108, 16, 254),
(3015, 345, 108, 23, 254),
(3002, 235, 109, 11, 254),
(3002, 1726, 109, 51, 254),
(3002, 6125, 109, 66, 254),
(3002, 14348, 109, 77, 81),
(3002, 14346, 109, 77, 81),
(3002, 14347, 109, 77, 81),
(3002, 18369, 109, 82, 254),
(3002, 18367, 109, 82, 254),
(3002, 18368, 109, 82, 254),
(3003, 235, 109, 17, 254),
(3004, 247, 109, 14, 46),
(3004, 80, 109, 32, 64),
(3004, 34, 109, 47, 254),
(3004, 2517, 109, 65, 254),
(3005, 235, 109, 4, 254),
(3006, 247, 109, 4, 17),
(3006, 255, 109, 8, 254),
(3006, 80, 109, 13, 53),
(3006, 34, 109, 18, 254),
(3006, 2516, 109, 52, 254),
(3006, 4058, 109, 52, 254),
(3006, 2517, 109, 54, 254),
(3006, 3185, 109, 62, 254),
(3006, 6123, 109, 68, 254),
(3008, 719, 109, 19, 50),
(3008, 735, 109, 24, 254),
(3008, 1750, 109, 51, 254),
(3010, 79, 109, 7, 55),
(3010, 255, 109, 10, 254),
(3010, 42, 109, 27, 254),
(3010, 1575, 109, 56, 254),
(3010, 2886, 109, 58, 254),
(3011, 235, 109, 1, 254),
(3011, 457, 109, 41, 254),
(3011, 1391, 109, 45, 254),
(3011, 6124, 109, 68, 254),
(3012, 80, 109, 4, 39),
(3012, 42, 109, 16, 254),
(3012, 3811, 109, 40, 254),
(3012, 6120, 109, 67, 254),
(3012, 15513, 109, 76, 80),
(3012, 15511, 109, 76, 80),
(3012, 15512, 109, 76, 80),
(3012, 19701, 109, 81, 254),
(3012, 19699, 109, 81, 254),
(3012, 19700, 109, 81, 254),
(3013, 42, 109, 8, 254),
(3013, 80, 109, 16, 254),
(3014, 42, 109, 4, 254),
(3014, 80, 109, 6, 43),
(3014, 235, 109, 14, 254),
(3014, 3696, 109, 44, 254),
(3014, 6122, 109, 66, 254),
(3015, 79, 109, 29, 64),
(3015, 42, 109, 43, 254),
(3015, 1575, 109, 65, 254),
(3004, 278, 110, 28, 64),
(3004, 4054, 110, 41, 64),
(3004, 4055, 110, 49, 254),
(3004, 2517, 110, 65, 254),
(3006, 278, 110, 10, 53),
(3006, 424, 110, 26, 254),
(3006, 4054, 110, 30, 53),
(3006, 169, 110, 35, 61),
(3006, 4055, 110, 35, 254),
(3006, 3579, 110, 45, 254),
(3006, 4058, 110, 52, 254),
(3006, 1554, 110, 53, 254),
(3006, 2517, 110, 54, 254),
(3006, 3185, 110, 62, 254),
(3008, 717, 110, 5, 48),
(3008, 4395, 110, 25, 48),
(3008, 2605, 110, 49, 50),
(3008, 1750, 110, 51, 254),
(3010, 278, 110, 9, 254),
(3010, 424, 110, 22, 254),
(3010, 4054, 110, 29, 254),
(3010, 4055, 110, 34, 254),
(3010, 2524, 110, 36, 254),
(3010, 1554, 110, 52, 254),
(3015, 278, 110, 24, 254),
(3015, 4054, 110, 39, 254),
(3015, 4055, 110, 44, 254),
(3012, 1422, 111, 50, 254),
(3012, 1334, 111, 52, 254),
(3005, 2213, 112, 12, 34),
(3005, 3, 112, 35, 56),
(3005, 1773, 112, 57, 70),
(3005, 10042, 112, 71, 75),
(3005, 14823, 112, 76, 80),
(3005, 18928, 112, 81, 85),
(3005, 25555, 112, 86, 90),
(3005, 28632, 112, 91, 95),
(3005, 34662, 112, 96, 254),
(3011, 2213, 112, 12, 34),
(3011, 3, 112, 35, 56),
(3011, 1773, 112, 57, 70),
(3011, 10042, 112, 71, 75),
(3011, 14823, 112, 76, 80),
(3011, 18928, 112, 81, 85),
(3011, 25555, 112, 86, 90),
(3011, 28632, 112, 91, 95),
(3011, 34662, 112, 96, 254);

DELETE
FROM bot_spells_entries
WHERE NOT EXISTS
(SELECT *
FROM spells_new
WHERE bot_spells_entries.spell_id = spells_new.id);
)"
	},
	ManifestEntry{
		.version = 9051,
		.description = "2024_11_26_remove_sk_icb.sql",
		.check = "SELECT * FROM `bot_spells_entries` where `type` = 24",
		.condition = "empty",
		.match = "",
		.sql = R"(
DELETE
FROM bot_spells_entries
WHERE `npc_spells_id` = 3005
AND `type` = 10;

INSERT INTO `bot_spells_entries` (`npc_spells_id`, `spell_id`, `type`, `minlevel`, `maxlevel`, `priority`) 
VALUES
(3003, 10173, 24, 72, 76, 3),
(3003, 10174, 24, 72, 76, 2),
(3003, 10175, 24, 72, 76, 1),
(3003, 14954, 24, 77, 81, 3),
(3003, 14955, 24, 77, 81, 2),
(3003, 14956, 24, 77, 81, 1),
(3003, 19068, 24, 82, 86, 3),
(3003, 19069, 24, 82, 86, 2),
(3003, 19070, 24, 82, 86, 1),
(3003, 25297, 24, 87, 91, 3),
(3003, 25298, 24, 87, 91, 2),
(3003, 25299, 24, 87, 91, 1),
(3003, 28347, 24, 92, 96, 3),
(3003, 28348, 24, 92, 96, 2),
(3003, 28349, 24, 92, 96, 1),
(3003, 34350, 24, 97, 254, 3),
(3003, 34351, 24, 97, 254, 2),
(3003, 34352, 24, 97, 254, 1),
(3003, 40078, 24, 98, 254, 3),
(3003, 40079, 24, 98, 254, 2),
(3003, 40080, 24, 98, 254, 1),
(3005, 1221, 24, 33, 41, 1),
(3005, 1222, 24, 42, 52, 1),
(3005, 1223, 24, 53, 58, 1),
(3005, 1224, 24, 59, 62, 1),
(3005, 3405, 24, 63, 66, 1),
(3005, 5329, 24, 67, 70, 5),
(3005, 5336, 24, 69, 73, 4),
(3005, 10257, 24, 71, 71, 3),
(3005, 10258, 24, 71, 71, 2),
(3005, 10259, 24, 71, 71, 1),
(3005, 10260, 24, 72, 76, 3),
(3005, 10261, 24, 72, 76, 2),
(3005, 10262, 24, 72, 76, 1),
(3005, 10291, 24, 74, 78, 3),
(3005, 10292, 24, 74, 78, 2),
(3005, 10293, 24, 74, 78, 1),
(3005, 15160, 24, 76, 76, 3),
(3005, 15161, 24, 76, 76, 2),
(3005, 15162, 24, 76, 76, 1),
(3005, 15163, 24, 77, 81, 3),
(3005, 15164, 24, 77, 81, 2),
(3005, 15165, 24, 77, 81, 1),
(3005, 15184, 24, 79, 83, 3),
(3005, 15185, 24, 79, 83, 2),
(3005, 15186, 24, 79, 83, 1),
(3005, 19313, 24, 81, 81, 3),
(3005, 19314, 24, 81, 81, 2),
(3005, 19315, 24, 81, 81, 1),
(3005, 19316, 24, 82, 86, 3),
(3005, 19317, 24, 82, 86, 2),
(3005, 19318, 24, 82, 86, 1),
(3005, 19337, 24, 84, 88, 3),
(3005, 19338, 24, 84, 88, 2),
(3005, 19339, 24, 84, 88, 1),
(3005, 25580, 24, 86, 86, 3),
(3005, 25581, 24, 86, 86, 2),
(3005, 25582, 24, 86, 86, 1),
(3005, 25586, 24, 87, 91, 3),
(3005, 25587, 24, 87, 91, 2),
(3005, 25588, 24, 87, 91, 1),
(3005, 25641, 24, 89, 93, 3),
(3005, 25642, 24, 89, 93, 2),
(3005, 25643, 24, 89, 93, 1),
(3005, 28657, 24, 91, 91, 3),
(3005, 28658, 24, 91, 91, 2),
(3005, 28659, 24, 91, 91, 1),
(3005, 28663, 24, 92, 96, 3),
(3005, 28664, 24, 92, 96, 2),
(3005, 28665, 24, 92, 96, 1),
(3005, 28733, 24, 94, 98, 3),
(3005, 28734, 24, 94, 98, 2),
(3005, 28735, 24, 94, 98, 1),
(3005, 34687, 24, 96, 96, 3),
(3005, 34688, 24, 96, 96, 2),
(3005, 34689, 24, 96, 96, 1),
(3005, 34693, 24, 97, 254, 3),
(3005, 34694, 24, 97, 254, 2),
(3005, 34695, 24, 97, 254, 1),
(3005, 34751, 24, 99, 254, 3),
(3005, 34752, 24, 99, 254, 2),
(3005, 34753, 24, 99, 254, 1);

DELETE
FROM bot_spells_entries
WHERE NOT EXISTS
(SELECT *
FROM spells_new
WHERE bot_spells_entries.spell_id = spells_new.id);
)"
	},
ManifestEntry{
	.version = 9052,
	.description = "2024_12_15_bot_blocked_buffs.sql",
	.check = "SHOW TABLES LIKE 'bot_blocked_buffs'",
	.condition = "empty",
	.match = "",
	.sql = R"(
CREATE TABLE `bot_blocked_buffs` (
	`bot_id` INT(11) UNSIGNED NOT NULL,
	`spell_id` INT(11) UNSIGNED NOT NULL,
	`blocked` TINYINT(4) UNSIGNED NULL DEFAULT '0',
	`blocked_pet` TINYINT(4) UNSIGNED NULL DEFAULT '0',
	PRIMARY KEY (`bot_id`, `spell_id`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;
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
