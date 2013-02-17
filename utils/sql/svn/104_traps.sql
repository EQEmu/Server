ALTER TABLE `traps` DROP `spawnchance`;
ALTER TABLE `traps` ADD `respawn_time` INT(11) UNSIGNED DEFAULT '60' NOT NULL AFTER `skill`;
ALTER TABLE `traps` ADD `level` MEDIUMINT(4) UNSIGNED DEFAULT '1' NOT NULL AFTER `skill`;
ALTER TABLE `traps` ADD `respawn_var` INT(11) UNSIGNED DEFAULT '0' NOT NULL AFTER `respawn_time`;
