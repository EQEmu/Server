ALTER TABLE `npc_types` ADD `attack_delay` TINYINT(3) UNSIGNED DEFAULT '30' NOT NULL AFTER `attack_speed`;
UPDATE `npc_types` SET `attack_delay` = 36 + 36 * (`attack_speed` / 100);
UPDATE `npc_types` SET `attack_delay` = 30 WHERE `attack_speed` = 0;
