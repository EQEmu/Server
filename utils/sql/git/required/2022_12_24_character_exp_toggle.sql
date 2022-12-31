ALTER TABLE `character_data`
ADD COLUMN `exp_enabled` tinyint(1) UNSIGNED NOT NULL DEFAULT 1 AFTER `exp`;
