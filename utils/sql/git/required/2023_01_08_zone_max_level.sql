ALTER TABLE `zone` 
ADD COLUMN `max_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 255 AFTER `min_level`;