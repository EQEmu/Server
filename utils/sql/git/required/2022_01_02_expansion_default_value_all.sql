-- forage

ALTER TABLE `forage` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `forage` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE forage set min_expansion = -1 where min_expansion = 0;
UPDATE forage set max_expansion = -1 where max_expansion = 0;

-- tradeskill_recipe

ALTER TABLE `tradeskill_recipe` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `tradeskill_recipe` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE tradeskill_recipe set min_expansion = -1 where min_expansion = 0;
UPDATE tradeskill_recipe set max_expansion = -1 where max_expansion = 0;

-- fishing

ALTER TABLE `fishing` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `fishing` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE fishing set min_expansion = -1 where min_expansion = 0;
UPDATE fishing set max_expansion = -1 where max_expansion = 0;

-- zone

ALTER TABLE `zone` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `zone` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE zone set min_expansion = -1 where min_expansion = 0;
UPDATE zone set max_expansion = -1 where max_expansion = 0;

-- traps

ALTER TABLE `traps` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `traps` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE traps set min_expansion = -1 where min_expansion = 0;
UPDATE traps set max_expansion = -1 where max_expansion = 0;

-- loottable

ALTER TABLE `loottable` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `loottable` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE loottable set min_expansion = -1 where min_expansion = 0;
UPDATE loottable set max_expansion = -1 where max_expansion = 0;

-- ground_spawns

ALTER TABLE `ground_spawns` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `ground_spawns` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE ground_spawns set min_expansion = -1 where min_expansion = 0;
UPDATE ground_spawns set max_expansion = -1 where max_expansion = 0;

-- starting_items

ALTER TABLE `starting_items` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `starting_items` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE starting_items set min_expansion = -1 where min_expansion = 0;
UPDATE starting_items set max_expansion = -1 where max_expansion = 0;

-- spawn2

ALTER TABLE `spawn2` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `spawn2` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE spawn2 set min_expansion = -1 where min_expansion = 0;
UPDATE spawn2 set max_expansion = -1 where max_expansion = 0;

-- zone_points

ALTER TABLE `zone_points` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `zone_points` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE zone_points set min_expansion = -1 where min_expansion = 0;
UPDATE zone_points set max_expansion = -1 where max_expansion = 0;

-- lootdrop

ALTER TABLE `lootdrop` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `lootdrop` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE lootdrop set min_expansion = -1 where min_expansion = 0;
UPDATE lootdrop set max_expansion = -1 where max_expansion = 0;

-- global_loot

ALTER TABLE `global_loot` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `global_loot` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE global_loot set min_expansion = -1 where min_expansion = 0;
UPDATE global_loot set max_expansion = -1 where max_expansion = 0;

-- doors

ALTER TABLE `doors` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `doors` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE doors set min_expansion = -1 where min_expansion = 0;
UPDATE doors set max_expansion = -1 where max_expansion = 0;

-- object

ALTER TABLE `object` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `object` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE object set min_expansion = -1 where min_expansion = 0;
UPDATE object set max_expansion = -1 where max_expansion = 0;

-- start_zones

ALTER TABLE `start_zones` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `start_zones` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE start_zones set min_expansion = -1 where min_expansion = 0;
UPDATE start_zones set max_expansion = -1 where max_expansion = 0;

-- merchantlist

ALTER TABLE `merchantlist` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `merchantlist` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE merchantlist set min_expansion = -1 where min_expansion = 0;
UPDATE merchantlist set max_expansion = -1 where max_expansion = 0;

-- spawnentry
ALTER TABLE `spawnentry` ADD `min_expansion` tinyint(4)  NOT NULL DEFAULT -1;
ALTER TABLE `spawnentry` ADD `max_expansion` tinyint(4)  NOT NULL DEFAULT -1;
ALTER TABLE `spawnentry` ADD `content_flags` varchar(100) NULL;
ALTER TABLE `spawnentry` ADD `content_flags_disabled` varchar(100) NULL;
