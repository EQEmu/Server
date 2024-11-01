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
	`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`char_id` INT UNSIGNED NOT NULL,
	`bot_id` INT UNSIGNED NOT NULL,
	`setting_id` INT UNSIGNED NOT NULL,
	`setting_type` INT UNSIGNED NOT NULL,
	`value` INT UNSIGNED NOT NULL,
	`category_name` VARCHAR(64) NULL DEFAULT '',
	`setting_name` VARCHAR(64) NULL DEFAULT '',
	PRIMARY KEY (`id`) USING BTREE
)
COLLATE='utf8mb4_general_ci';

INSERT INTO bot_settings SELECT NULL, 0, bd.`bot_id`, 0, 0, bd.`expansion_bitmask`, 'BaseSetting', 'ExpansionBitmask' FROM bot_data bd
JOIN rule_values rv
WHERE rv.rule_name LIKE 'Bots:BotExpansionSettings'
AND bd.expansion_bitmask != rv.rule_value;

INSERT INTO bot_settings SELECT NULL, 0, `bot_id`, 1, 0, `show_helm`, 'BaseSetting', 'ShowHelm' FROM bot_data WHERE `show_helm` != 1;
INSERT INTO bot_settings SELECT NULL, 0, `bot_id`, 2, 0, sqrt(`follow_distance`), 'BaseSetting', 'FollowDistance' FROM bot_data WHERE `follow_distance` != 184;

INSERT INTO bot_settings 
SELECT NULL, 0, `bot_id`, 3, 0, `stop_melee_level`, 'BaseSetting', 'StopMeleeLevel'
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

INSERT INTO bot_settings SELECT NULL, 0, `bot_id`, 4, 0, `enforce_spell_settings`, 'BaseSetting', 'EnforceSpellSettings' FROM bot_data WHERE `enforce_spell_settings` != 0;
INSERT INTO bot_settings SELECT NULL, 0, `bot_id`, 5, 0, `archery_setting`, 'BaseSetting', 'RangedSetting' FROM bot_data WHERE `archery_setting` != 0;

INSERT INTO bot_settings
SELECT NULL, 0, `bot_id`, 8, 0, `caster_range`, 'BaseSetting', 'CasterRange'
FROM (
    SELECT `bot_id`, 
           (CASE 
               WHEN (`class` IN (1, 7, 19, 16)) THEN 0
                WHEN `class` = 8 THEN 0
               ELSE 90
           END) AS `casterRange`,
           `caster_range`
    FROM bot_data
) AS `subquery`
WHERE `casterRange` != `caster_range`;

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
UPDATE `bot_command_settings` SET `aliases`= CASE WHEN LENGTH(`aliases`) > 0 THEN CONCAT(`aliases`, '|cr') ELSE 'cr' END WHERE `bot_command`='casterrange' AND `aliases` NOT LIKE '%cr%';
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
WHERE b.spellid = s.id
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
		.check = "SELECT * FROM `bot_spells_entries` where `npc_spells_id` = 3002 AND `spellid` = 14312",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Class fixes
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3002 WHERE b.`spellid` = 14312;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3002 WHERE b.`spellid` = 14313;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3002 WHERE b.`spellid` = 14314;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3005 WHERE b.`spellid` = 15186;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3005 WHERE b.`spellid` = 15187;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3005 WHERE b.`spellid` = 15188;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spellid` = 14446;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spellid` = 14447;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spellid` = 14467;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spellid` = 14468;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spellid` = 14469;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3003 WHERE b.`spellid` = 14955;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3003 WHERE b.`spellid` = 14956;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spellid` = 14387;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spellid` = 14388;
UPDATE bot_spells_entries b SET b.`npc_spells_id` = 3006 WHERE b.`spellid` = 14389;

-- Minlevel fixes
UPDATE bot_spells_entries SET `minlevel` = 34 WHERE `spellid` = 1445 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 2 WHERE `spellid` = 229 AND `npc_spells_id` = 3011;
UPDATE bot_spells_entries SET `minlevel` = 13 WHERE `spellid` = 333 AND `npc_spells_id` = 3013;
UPDATE bot_spells_entries SET `minlevel` = 29 WHERE `spellid` = 106 AND `npc_spells_id` = 3013;
UPDATE bot_spells_entries SET `minlevel` = 38 WHERE `spellid` = 754 AND `npc_spells_id` = 3010;
UPDATE bot_spells_entries SET `minlevel` = 58 WHERE `spellid` = 2589 AND `npc_spells_id` = 3003;
UPDATE bot_spells_entries SET `minlevel` = 67 WHERE `spellid` = 5305 AND `npc_spells_id` = 3004;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 14267 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 14268 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 14269 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 23 WHERE `spellid` = 738 AND `npc_spells_id` = 3008;
UPDATE bot_spells_entries SET `minlevel` = 51 WHERE `spellid` = 1751 AND `npc_spells_id` = 3008;
UPDATE bot_spells_entries SET `minlevel` = 7 WHERE `spellid` = 734 AND `npc_spells_id` = 3008;
UPDATE bot_spells_entries SET `minlevel` = 5 WHERE `spellid` = 717 AND `npc_spells_id` = 3008;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 15186 AND `npc_spells_id` = 3005;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 15187 AND `npc_spells_id` = 3005;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 15188 AND `npc_spells_id` = 3005;
UPDATE bot_spells_entries SET `minlevel` = 80 WHERE `spellid` = 14446 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 80 WHERE `spellid` = 14447 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 14467 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 14468 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 79 WHERE `spellid` = 14469 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spellid` = 14955 AND `npc_spells_id` = 3003;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spellid` = 14956 AND `npc_spells_id` = 3003;
UPDATE bot_spells_entries SET `minlevel` = 78 WHERE `spellid` = 14387 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spellid` = 14388 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spellid` = 14389 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spellid` = 14312 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spellid` = 14313 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `minlevel` = 77 WHERE `spellid` = 14314 AND `npc_spells_id` = 3002;

-- Maxlevel fixes
UPDATE bot_spells_entries SET `maxlevel` = 83 WHERE `spellid` = 14267 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 83 WHERE `spellid` = 14268 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 83 WHERE `spellid` = 14269 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spellid` = 14446 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spellid` = 14447 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spellid` = 14467 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spellid` = 14468 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 84 WHERE `spellid` = 14469 AND `npc_spells_id` = 3006;
UPDATE bot_spells_entries SET `maxlevel` = 81 WHERE `spellid` = 14312 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 81 WHERE `spellid` = 14313 AND `npc_spells_id` = 3002;
UPDATE bot_spells_entries SET `maxlevel` = 81 WHERE `spellid` = 14314 AND `npc_spells_id` = 3002;

-- Type fixes
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 201;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spellid` = 752;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spellid` = 2117;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 2542;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 2544;
UPDATE bot_spells_entries SET `type` = 6 WHERE `spellid` = 2115;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 1403;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 1405;
UPDATE bot_spells_entries SET `type` = 9 WHERE `spellid` = 289;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 294;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 302;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 521;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 185;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 450;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 186;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 4074;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 195;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 1712;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 1703;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spellid` = 3229;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 3345;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 5509;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 6826;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 270;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 281;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 505;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 526;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 110;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 506;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 162;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 111;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 507;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 527;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 163;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 112;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 1588;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 1573;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 1592;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 1577;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 1578;
UPDATE bot_spells_entries SET `type` = 1 WHERE `spellid` = 1576;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 3386;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 3387;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 4900;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 3395;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 5394;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 5392;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 6827;
UPDATE bot_spells_entries SET `type` = 1 WHERE `spellid` = 5416;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 1437;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 1436;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 5348;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 8008;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 2571;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 370;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spellid` = 1741;
UPDATE bot_spells_entries SET `type` = 17 WHERE `spellid` = 1296;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 270;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 2634;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 2942;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 3462;
UPDATE bot_spells_entries SET `type` = 13 WHERE `spellid` = 6828;
UPDATE bot_spells_entries SET `type` = 4 WHERE `spellid` = 14312;
UPDATE bot_spells_entries SET `type` = 4 WHERE `spellid` = 14313;
UPDATE bot_spells_entries SET `type` = 4 WHERE `spellid` = 14314;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 18392;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 18393;
UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 18394;
UPDATE bot_spells_entries SET `type` = 10 WHERE `spellid` = 15186;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 15187;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 15188;
UPDATE bot_spells_entries SET `type` = 1 WHERE `spellid` = 14446;
UPDATE bot_spells_entries SET `type` = 1 WHERE `spellid` = 14447;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 14467;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 14468;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 14469;
UPDATE bot_spells_entries SET `type` = 0 WHERE `spellid` = 14267;
UPDATE bot_spells_entries SET `type` = 0 WHERE `spellid` = 14268;
UPDATE bot_spells_entries SET `type` = 0 WHERE `spellid` = 14269;
UPDATE bot_spells_entries SET `type` = 10 WHERE `spellid` = 14955;
UPDATE bot_spells_entries SET `type` = 10 WHERE `spellid` = 14956;
UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 14387;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 14388;
UPDATE bot_spells_entries SET `type` = 3 WHERE `spellid` = 14389;
UPDATE bot_spells_entries SET `type` = 4 WHERE `spellid` = 10436;

-- UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 3440; -- Ro's Illumination [#3440] from DoT [#8] to Debuff [#14] [Should be 0]
-- UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 303; -- Whirl till you hurl [#303] from Nuke [#0] to Debuff [#14] [Should be 0]
-- UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 619; -- Dyn's Dizzying Draught [#619] from Nuke [#0] to Debuff [#14] [Should be 0]

-- UPDATE bot_spells_entries SET `type` = 14 WHERE `spellid` = 74; -- Mana Sieve [#74] from Nuke [#0] to Debuff [#14]
-- UPDATE bot_spells_entries SET `type` = 6 WHERE `spellid` = 1686; -- Theft of Thought [#1686] from Nuke [#0] to Lifetap [#6]

-- UPDATE bot_spells_entries SET `type` = 1 WHERE `spellid` = 3694; -- Stoicism [#3694] from In-Combat Buff [#10] to Regular Heal [#1]
-- UPDATE bot_spells_entries SET `type` = 1 WHERE `spellid` = 4899; -- Breath of Trushar [#4899] from In-Combat Buff [#10] to Regular Heal [#1]

-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spellid` = 738; -- Selo's Consonant Chain [#738] from Slow [#13] to Snare [#7]
-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spellid` = 1751; -- Largo's Assonant Binding [#1751] from Slow [#13] to Snare [#7]
-- UPDATE bot_spells_entries SET `type` = 8 WHERE `spellid` = 1748; -- Angstlich's Assonance [#1748] from Slow [#13] to DoT [#8]
-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spellid` = 738; -- Selo's Consonant Chain [#738] from Slow [#13] to Snare [#7]
-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spellid` = 1751; -- Largo's Assonant Binding [#1751] from Slow [#13] to Snare [#7]
-- UPDATE bot_spells_entries SET `type` = 7 WHERE `spellid` = 738; -- Selo's Consonant Chain [#738] from Slow [#13] to Snare [#7]

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
