ALTER TABLE `merchantlist` 
ADD COLUMN `min_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `level_required`,
ADD COLUMN `max_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 255 AFTER `min_status`;