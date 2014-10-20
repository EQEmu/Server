ALTER TABLE `merc_stats`
MODIFY COLUMN `special_abilities`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL AFTER `attack_speed`;

ALTER TABLE `npc_types`
MODIFY COLUMN `special_abilities`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL AFTER `npcspecialattks`;

