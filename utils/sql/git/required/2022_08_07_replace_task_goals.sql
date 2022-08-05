-- backup original since this is a complex migration
CREATE TABLE `task_activities_backup_9203` LIKE `task_activities`;
INSERT INTO `task_activities_backup_9203` SELECT * FROM `task_activities`;

ALTER TABLE `task_activities`
  CHANGE COLUMN `description_override` `description_override` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `goalcount`,
  ADD COLUMN `npc_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `description_override`,
  ADD COLUMN `npc_goal_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `npc_id`,
  ADD COLUMN `npc_match_list` TEXT NULL DEFAULT NULL AFTER `npc_goal_id`,
  ADD COLUMN `item_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `npc_match_list`,
  ADD COLUMN `item_goal_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `item_id`,
  ADD COLUMN `item_id_list` TEXT NULL DEFAULT NULL AFTER `item_goal_id`,
  CHANGE COLUMN `item_list` `item_list` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `item_id_list`,
  ADD COLUMN `dz_switch_id` INT(11) NOT NULL DEFAULT '0' AFTER `delivertonpc`,
  ADD COLUMN `min_x` FLOAT NOT NULL DEFAULT 0 AFTER `dz_switch_id`,
  ADD COLUMN `min_y` FLOAT NOT NULL DEFAULT 0 AFTER `min_x`,
  ADD COLUMN `min_z` FLOAT NOT NULL DEFAULT 0 AFTER `min_y`,
  ADD COLUMN `max_x` FLOAT NOT NULL DEFAULT 0 AFTER `min_z`,
  ADD COLUMN `max_y` FLOAT NOT NULL DEFAULT 0 AFTER `max_x`,
  ADD COLUMN `max_z` FLOAT NOT NULL DEFAULT 0 AFTER `max_y`,
  CHANGE COLUMN `skill_list` `skill_list` VARCHAR(64) NOT NULL DEFAULT '-1' COLLATE 'latin1_swedish_ci' AFTER `max_z`,
  CHANGE COLUMN `spell_list` `spell_list` VARCHAR(64) NOT NULL DEFAULT '0' COLLATE 'latin1_swedish_ci' AFTER `skill_list`;

-- move Explore (5) goalid proximities to the new location fields
-- does not migrate where zone was different and ignores lists (unsupported)
UPDATE `task_activities`
INNER JOIN `proximities`
  ON `task_activities`.`goalid` = `proximities`.`exploreid`
  AND CAST(`task_activities`.`zones` AS INT) = `proximities`.`zoneid`
SET
  `task_activities`.`goalid` = 0,
  `task_activities`.`min_x` = `proximities`.`minx`,
  `task_activities`.`min_y` = `proximities`.`miny`,
  `task_activities`.`min_z` = `proximities`.`minz`,
  `task_activities`.`max_x` = `proximities`.`maxx`,
  `task_activities`.`max_y` = `proximities`.`maxy`,
  `task_activities`.`max_z` = `proximities`.`maxz`
WHERE
  `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` = 5;

-- dz_switch_id for Touch (11)
UPDATE `task_activities`
SET `task_activities`.`dz_switch_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` = 11;

-- single item ids for Deliver (1), Loot (3), TradeSkill (6), Fish (7), Forage (8)
UPDATE `task_activities`
SET `task_activities`.`item_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- item goallist id
UPDATE `task_activities`
SET `task_activities`.`item_goal_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- item id match list
UPDATE `task_activities`
SET `task_activities`.`item_id_list` = `task_activities`.`goal_match_list`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- single npc ids for Kill (2), SpeakWith (4)
UPDATE `task_activities`
SET `task_activities`.`npc_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` IN (2, 4);

-- npc goallist id
UPDATE `task_activities`
SET `task_activities`.`npc_goal_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (2, 4);

-- npc match list
UPDATE `task_activities`
SET `task_activities`.`npc_match_list` = `task_activities`.`goal_match_list`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (2, 4);

-- delivertonpc npc_ids for Deliver (1), GiveCash (100)
UPDATE `task_activities`
SET `task_activities`.`npc_id` = `task_activities`.`delivertonpc`
WHERE `task_activities`.`activitytype` IN (1, 100);

ALTER TABLE `task_activities`
  DROP COLUMN `goalid`,
  DROP COLUMN `goal_match_list`,
  DROP COLUMN `delivertonpc`;

-- leave proximities table backup in case of regressions
ALTER TABLE `proximities` RENAME `proximities_backup_9203`;
