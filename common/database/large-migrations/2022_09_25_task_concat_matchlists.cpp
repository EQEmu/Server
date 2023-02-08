std::string _2022_09_25_task_concat_matchlists = R"(
SET SESSION group_concat_max_len = 1048576;
SET collation_connection = latin1_swedish_ci;

-- backup original(s)
CREATE TABLE `goallists_backup_9_25_2022` LIKE `goallists`;
INSERT INTO `goallists_backup_9_25_2022` SELECT * FROM `goallists`;
CREATE TABLE `tasks_backup_9_25_2022` LIKE `tasks`;
INSERT INTO `tasks_backup_9_25_2022` SELECT * FROM `tasks`;

-- npc id
UPDATE `task_activities`
SET `task_activities`.`npc_match_list` = CONCAT_WS('|', `npc_match_list`, `npc_id`)
WHERE npc_id != 0;

-- npc_goal_id goallists
UPDATE `task_activities`
INNER JOIN
(
  SELECT `goallists`.`listid`, GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids`
  FROM `goallists`
  GROUP BY `goallists`.`listid`
) AS `goallist_group`
  ON `task_activities`.`npc_goal_id` = `goallist_group`.`listid`
SET `task_activities`.`npc_match_list` = CONCAT_WS('|', `npc_match_list`, `goallist_ids`)
WHERE npc_goal_id != 0;

-- item id
UPDATE `task_activities`
SET `task_activities`.`item_id_list` = CONCAT_WS('|', `item_id_list`, `item_id`)
WHERE item_id != 0;

-- item_goal_id goallists
UPDATE `task_activities`
INNER JOIN
(
  SELECT `goallists`.`listid`, GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids`
  FROM `goallists`
  GROUP BY `goallists`.`listid`
) AS `goallist_group`
  ON `task_activities`.`item_goal_id` = `goallist_group`.`listid`
SET `task_activities`.`item_id_list` = CONCAT_WS('|', `item_id_list`, `goallist_ids`)
WHERE item_goal_id != 0;

ALTER TABLE `task_activities`
  DROP COLUMN `npc_id`,
  DROP COLUMN `npc_goal_id`,
  DROP COLUMN `item_id`,
  DROP COLUMN `item_goal_id`;


-- Reward cleanup and task table cleanup

ALTER TABLE `tasks`
    CHANGE COLUMN `reward` `reward_text` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `description`,
    CHANGE COLUMN `rewardid` `reward_id_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL AFTER `reward_text`,
    CHANGE COLUMN `cashreward` `cash_reward` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `reward_id_list`,
    CHANGE COLUMN `rewardmethod` `reward_method` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `xpreward`,
    CHANGE COLUMN `minlevel` `min_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `reward_point_type`,
    CHANGE COLUMN `maxlevel` `max_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `min_level`;

ALTER Table `tasks` CHANGE COLUMN `xpreward` `exp_reward` int(10) NOT NULL DEFAULT 0 AFTER `cash_reward`;

UPDATE tasks SET reward_id_list =
 (
     SELECT GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids` FROM `goallists` WHERE listid = reward_id_list)
WHERE
reward_method = 1;

-- deprecated table
DROP table goallists;

)";