-- zone
ALTER TABLE `zone` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone` ADD `content_flags` varchar(100) NULL;

-- doors
ALTER TABLE `doors` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `doors` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `doors` ADD `content_flags` varchar(100) NULL;

-- object
ALTER TABLE `object` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `object` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `object` ADD `content_flags` varchar(100) NULL;

-- spawn2
ALTER TABLE `spawn2` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `spawn2` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `spawn2` ADD `content_flags` varchar(100) NULL;

-- tradeskill_recipe
ALTER TABLE `tradeskill_recipe` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `tradeskill_recipe` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `tradeskill_recipe` ADD `content_flags` varchar(100) NULL;

-- merchantlist
ALTER TABLE `merchantlist` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `merchantlist` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `merchantlist` ADD `content_flags` varchar(100) NULL;

-- global_loot
ALTER TABLE `global_loot` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `global_loot` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `global_loot` ADD `content_flags` varchar(100) NULL;

-- fishing
ALTER TABLE `fishing` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `fishing` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `fishing` ADD `content_flags` varchar(100) NULL;

-- forage
ALTER TABLE `forage` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `forage` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `forage` ADD `content_flags` varchar(100) NULL;

-- ground_spawns
ALTER TABLE `ground_spawns` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `ground_spawns` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `ground_spawns` ADD `content_flags` varchar(100) NULL;

-- loottable
ALTER TABLE `loottable` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `loottable` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `loottable` ADD `content_flags` varchar(100) NULL;

-- lootdrop
ALTER TABLE `lootdrop` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `lootdrop` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `lootdrop` ADD `content_flags` varchar(100) NULL;

-- starting_items
ALTER TABLE `starting_items` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `starting_items` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `starting_items` ADD `content_flags` varchar(100) NULL;

-- start_zones
ALTER TABLE `start_zones` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `start_zones` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `start_zones` ADD `content_flags` varchar(100) NULL;

-- traps
ALTER TABLE `traps` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `traps` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `traps` ADD `content_flags` varchar(100) NULL;

-- zone_points
ALTER TABLE `zone_points` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone_points` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone_points` ADD `content_flags` varchar(100) NULL;

-- pok books
update doors set min_expansion = 4 where name like '%POKTELE%';

-- content flags
CREATE TABLE `content_flags` (`id` int AUTO_INCREMENT,`flag_name` varchar(75),`enabled` tinyint,`notes` text, PRIMARY KEY (id));

-- content flags disabled

ALTER TABLE `doors` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `fishing` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `forage` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `global_loot` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `ground_spawns` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `lootdrop` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `loottable` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `merchantlist` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `object` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `spawn2` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `start_zones` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `starting_items` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `tradeskill_recipe` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `traps` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `zone` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `zone_points` ADD `content_flags_disabled` varchar(100) NULL;