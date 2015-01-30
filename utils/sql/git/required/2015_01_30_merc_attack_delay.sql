ALTER TABLE `merc_stats` ADD `attack_delay` TINYINT(3) UNSIGNED DEFAULT '30' NOT NULL AFTER `attack_speed`;
UPDATE `merc_stats` SET `attack_delay` = 36 + 36 * (`attack_speed` / 100);
UPDATE `merc_stats` SET `attack_delay` = 30 WHERE `attack_speed` = 0;
