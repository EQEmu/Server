ALTER TABLE `tasks` ADD `type` TINYINT NOT NULL DEFAULT '0' AFTER `id`;
ALTER TABLE `tasks` ADD `duration_code` TINYINT NOT NULL DEFAULT '0' AFTER `duration`;
UPDATE `tasks` SET `type` = '2'; -- we were treating them all as quests
ALTER TABLE `character_tasks` ADD `type` TINYINT NOT NULL DEFAULT '0' AFTER `slot`;
UPDATE `character_tasks` SET `type` = '2'; -- we were treating them all as quests
ALTER TABLE `activities` ADD `target_name` VARCHAR(64) NOT NULL DEFAULT '' AFTER `activitytype`;
ALTER TABLE `activities` ADD `item_list` VARCHAR(128) NOT NULL DEFAULT '' AFTER `target_name`;
ALTER TABLE `activities` ADD `skill_list` VARCHAR(64) NOT NULL DEFAULT '-1' AFTER `item_list`;
ALTER TABLE `activities` ADD `spell_list` VARCHAR(64) NOT NULL DEFAULT '0' AFTER `skill_list`;
ALTER TABLE `activities` ADD `description_override` VARCHAR(128) NOT NULL DEFAULT '' AFTER `spell_list`;
ALTER TABLE `activities` ADD `zones` VARCHAR(64) NOT NULL DEFAULT '' AFTER `zoneid`;
UPDATE `activities` SET `description_override` = `text3`;
UPDATE `activities` SET `target_name` = `text1`;
UPDATE `activities` SET `item_list` = `text2`;
UPDATE `activities` SET `zones` = `zoneid`; -- should be safe for us ...
ALTER TABLE `activities` DROP COLUMN `text1`;
ALTER TABLE `activities` DROP COLUMN `text2`;
ALTER TABLE `activities` DROP COLUMN `text3`;
ALTER TABLE `activities` DROP COLUMN `zoneid`;
ALTER TABLE `tasks` DROP COLUMN `startzone`;
ALTER TABLE `tasks` ADD `faction_reward` INT(10) NOT NULL DEFAULT '0';
RENAME TABLE `activities` TO `task_activities`;