ALTER TABLE `lootdrop_entries` CHANGE `minlevel` `trivial_min_level` tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `maxlevel` `trivial_max_level` tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` ADD COLUMN `npc_min_level` smallint unsigned NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `lootdrop_entries` ADD COLUMN `npc_max_level` smallint unsigned NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `trivial_min_level` `trivial_min_level` smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `trivial_max_level` `trivial_max_level` smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT '';
UPDATE `lootdrop_entries` SET `trivial_max_level` = 0 WHERE `trivial_max_level` = 127;