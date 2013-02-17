CREATE TABLE `instance_lockout` (
  `id` int(11) NOT NULL auto_increment,
  `zone` int(11) unsigned NOT NULL default '0',
  `version` tinyint(4) unsigned NOT NULL default '0',
  `start_time` int(11) unsigned NOT NULL default '0',
  `duration` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `id_2` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `instance_lockout_player` (
  `id` int(11) unsigned NOT NULL default '0',
  `charid` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`charid`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `respawn_times` ADD `instance_id` SMALLINT DEFAULT '0' NOT NULL AFTER `duration`;
ALTER TABLE `respawn_times` DROP PRIMARY KEY, ADD PRIMARY KEY (`id`, `instance_id`);

ALTER TABLE `character_` ADD `instanceid` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zoneid`;
ALTER TABLE `character_` DROP `instZflagNum`;
ALTER TABLE `character_` DROP `instZOrgID`;

ALTER TABLE `spawn2` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zone`;

ALTER TABLE `player_corpses` ADD `instanceid` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zoneid`;
ALTER TABLE `player_corpses` ADD INDEX `instanceid` (`instanceid`);

ALTER TABLE `traps` ADD INDEX `zone` (`zone`);
ALTER TABLE `traps` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zone`;

ALTER TABLE `ground_spawns` ADD INDEX `zone` (`zoneid`);
ALTER TABLE `ground_spawns` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zoneid`;

ALTER TABLE `object` ADD INDEX `zone` (`zoneid`);
ALTER TABLE `object` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zoneid`;
ALTER TABLE `object` DROP `linked_list_addr_01`;
ALTER TABLE `object` DROP `linked_list_addr_02`;
ALTER TABLE `object` DROP `unknown88`;
DELETE FROM object WHERE object.type=1 AND object.itemid!=0;

ALTER TABLE `doors` ADD `version` SMALLINT UNSIGNED DEFAULT '0' NOT NULL AFTER `zone`;
ALTER TABLE `doors` ADD `is_ldon_door` TINYINT UNSIGNED DEFAULT '0' NOT NULL AFTER `dest_heading`;

ALTER TABLE `npc_types` ADD `adventure_template_id` INT UNSIGNED DEFAULT '0' NOT NULL AFTER `npc_faction_id`;

CREATE TABLE `adventure_template` (
  `id` int(10) unsigned NOT NULL,
  `zone` varchar(64) NOT NULL,
  `zone_version` tinyint(3) unsigned NOT NULL default '0',
  `is_hard` tinyint(3) unsigned NOT NULL default '0',
  `is_raid` tinyint(3) unsigned NOT NULL default '0',
  `min_level` tinyint(3) unsigned NOT NULL default '1',
  `max_level` tinyint(3) unsigned NOT NULL default '65',
  `type` tinyint(3) unsigned NOT NULL default '0',
  `type_data` int(10) unsigned NOT NULL default '0',
  `type_count` smallint(5) unsigned NOT NULL default '0',
  `text` varchar(512) NOT NULL,
  `duration` int(10) unsigned NOT NULL default '7200',
  `zone_in_time` int(10) unsigned NOT NULL default '1800',
  `win_points` smallint(5) unsigned NOT NULL default '0',
  `lose_points` smallint(5) unsigned NOT NULL default '0',
  `theme` tinyint(3) unsigned NOT NULL default '1',
  `zone_in_zone_id` smallint(5) unsigned NOT NULL default '0',
  `zone_in_x` float NOT NULL default '0',
  `zone_in_y` float NOT NULL default '0',
  `zone_in_object_id` smallint(4) NOT NULL default '0',
  `dest_x` float NOT NULL default '0',
  `dest_y` float NOT NULL default '0',
  `dest_z` float NOT NULL default '0',
  `dest_h` float NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `id_2` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `adventure_template_entry` (
  `id` int(10) unsigned NOT NULL,
  `template_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`id`,`template_id`),
  KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `adventure_details` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `adventure_id` smallint(5) unsigned NOT NULL,
  `instance_id` int(11) NOT NULL default '-1',
  `count` smallint(5) unsigned NOT NULL default '0',
  `assassinate_count` smallint(5) unsigned NOT NULL default '0',
  `status` tinyint(3) unsigned NOT NULL default '0',
  `time_created` int(10) unsigned NOT NULL default '0',
  `time_zoned` int(10) unsigned NOT NULL default '0',
  `time_completed` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `adventure_members` (
  `id` int(10) unsigned NOT NULL,
  `charid` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`charid`),
  KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `adventure_stats` (
  `player_id` int(10) unsigned NOT NULL,
  `guk_wins` mediumint(8) unsigned NOT NULL default '0',
  `mir_wins` mediumint(8) unsigned NOT NULL default '0',
  `mmc_wins` mediumint(8) unsigned NOT NULL default '0',
  `ruj_wins` mediumint(8) unsigned NOT NULL default '0',
  `tak_wins` mediumint(8) unsigned NOT NULL default '0',
  `guk_losses` mediumint(8) unsigned NOT NULL default '0',
  `mir_losses` mediumint(8) unsigned NOT NULL default '0',
  `mmc_losses` mediumint(8) unsigned NOT NULL default '0',
  `ruj_losses` mediumint(8) unsigned NOT NULL default '0',
  `tak_losses` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`player_id`),
  UNIQUE KEY `player_id` (`player_id`),
  KEY `player_id_2` (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;