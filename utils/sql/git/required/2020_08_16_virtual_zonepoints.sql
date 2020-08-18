ALTER TABLE `zone_points` ADD COLUMN `is_virtual` tinyint NOT NULL DEFAULT '0' COMMENT '' AFTER `content_flags_disabled`;
ALTER TABLE `zone_points` ADD COLUMN `height` int NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `zone_points` ADD COLUMN `width` int NOT NULL DEFAULT '0' COMMENT '';