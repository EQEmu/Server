ALTER TABLE `zone_points` ADD `version` int UNSIGNED default 0 NOT NULL AFTER `zone`;
ALTER TABLE `zone_points` ADD `target_instance` int UNSIGNED default 0 NOT NULL AFTER `target_zone_id`;
ALTER TABLE `zone_points` ADD `client_version_mask` int UNSIGNED default 4294967295 NOT NULL AFTER `buffer`;
ALTER TABLE `zone_points` DROP INDEX `NewIndex`, ADD INDEX `NewIndex` (`number`, `zone`);
ALTER TABLE `zone` ADD fog_density float default 0.0 NOT NULL AFTER fog_maxclip4;
ALTER TABLE `doors` ADD `dest_instance` int UNSIGNED default 0 NOT NULL AFTER `dest_zone`;
ALTER TABLE `doors` ADD `client_version_mask` int UNSIGNED default 4294967295 NOT NULL AFTER `buffer`;