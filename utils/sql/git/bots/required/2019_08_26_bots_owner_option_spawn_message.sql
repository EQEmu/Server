ALTER TABLE `bot_owner_options` ADD COLUMN `spawn_message_enabled` SMALLINT(3) UNSIGNED NULL DEFAULT '1' AFTER `stats_update`;
ALTER TABLE `bot_owner_options` ADD COLUMN `spawn_message_type` SMALLINT(3) UNSIGNED NULL DEFAULT '1' AFTER `spawn_message_enabled`;
