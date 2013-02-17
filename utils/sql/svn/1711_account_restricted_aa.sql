ALTER TABLE `altadv_vars` ADD `account_time_required` INT UNSIGNED DEFAULT '0' NOT NULL AFTER `clientver`;
ALTER TABLE `account` ADD `time_creation` INT UNSIGNED DEFAULT '0' NOT NULL AFTER `suspendeduntil`;
UPDATE `account` SET `time_creation` = UNIX_TIMESTAMP() WHERE `time_creation` = 0;