ALTER TABLE `npc_types` ADD COLUMN `stuck_behavior` TINYINT(4) NOT NULL DEFAULT '0' AFTER `rare_spawn`;
UPDATE `npc_types` SET `stuck_behavior`=2 WHERE `underwater`=1;
