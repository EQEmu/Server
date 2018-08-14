ALTER TABLE `inventory_version` ADD COLUMN `bot_step` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `step`;
