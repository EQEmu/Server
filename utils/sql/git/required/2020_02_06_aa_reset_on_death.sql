ALTER TABLE `aa_ability` ADD `reset_on_death` TINYINT(4) NOT NULL DEFAULT '0';
UPDATE `aa_ability` SET `reset_on_death` = '1' WHERE `id` = 6001;
