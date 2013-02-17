alter table faction_values add column `temp` tinyint(3) not null default 0;
alter table npc_faction_entries add column `temp` tinyint(3) not null default 0;
alter table spawngroup add column `despawn` tinyint(3) not null default 0;
alter table spawngroup add column `despawn_timer` int(11) not null default 100;
alter table lootdrop_entries add column `minlevel` tinyint(3) not null default 0;
alter table lootdrop_entries add column `maxlevel` tinyint(3) not null default 127;

DROP TABLE IF EXISTS `player_corpses_backup`;
CREATE TABLE `player_corpses_backup` (
  `id` int(11) unsigned NOT NULL default '0',
  `charid` int(11) unsigned NOT NULL default '0',
  `charname` varchar(64) NOT NULL default '',
  `zoneid` smallint(5) NOT NULL default '0',
  `instanceid` SMALLINT UNSIGNED DEFAULT '0' NOT NULL,
  `x` float NOT NULL default '0',
  `y` float NOT NULL default '0',
  `z` float NOT NULL default '0',
  `heading` float NOT NULL default '0',
  `data` blob,
  `timeofdeath` datetime NOT NULL default '0000-00-00 00:00:00',
  `rezzed` tinyint(3) unsigned default '0',
  `IsBurried` tinyint(3) NOT NULL default '0',
  `WasAtGraveyard` tinyint(3) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

INSERT INTO `rule_values` VALUES (1, 'World:DeleteStaleCorpeBackups', 'true', 'Deletes stale corpse backups older than 2 weeks.');
INSERT INTO `rule_values` VALUES (1, 'Zone:UsePlayerCorpseBackups', 'true', 'Keeps backups of player corpses.');