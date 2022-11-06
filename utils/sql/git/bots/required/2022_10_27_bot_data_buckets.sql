ALTER TABLE `bot_spells_entries` 
ADD COLUMN `bucket_name` varchar(100) NOT NULL DEFAULT '' AFTER `max_hp`,
ADD COLUMN `bucket_value` varchar(100) NOT NULL DEFAULT '' AFTER `bucket_name`,
ADD COLUMN `bucket_comparison` tinyint UNSIGNED NULL DEFAULT 0 AFTER `bucket_value`; 