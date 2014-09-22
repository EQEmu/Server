ALTER TABLE `account` ADD COLUMN `ban_reason` TEXT NULL DEFAULT NULL AFTER `expansion`, ADD COLUMN `suspend_reason` TEXT NULL DEFAULT NULL AFTER `ban_reason`;
