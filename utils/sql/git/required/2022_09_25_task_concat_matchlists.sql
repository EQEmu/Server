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
