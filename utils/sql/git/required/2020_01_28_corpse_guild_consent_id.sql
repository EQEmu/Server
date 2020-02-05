ALTER TABLE `character_corpses` ADD COLUMN `guild_consent_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `time_of_death`;
