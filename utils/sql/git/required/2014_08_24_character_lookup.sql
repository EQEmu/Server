-- chracter_lookup table structure --

CREATE TABLE `character_lookup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account_id` int(11) NOT NULL DEFAULT '0',
  `name` varchar(64) NOT NULL DEFAULT '',
  `timelaston` int(11) unsigned DEFAULT '0',
  `x` float NOT NULL DEFAULT '0',
  `y` float NOT NULL DEFAULT '0',
  `z` float NOT NULL DEFAULT '0',
  `zonename` varchar(30) NOT NULL DEFAULT '',
  `zoneid` smallint(6) NOT NULL DEFAULT '0',
  `instanceid` smallint(5) unsigned NOT NULL DEFAULT '0',
  `pktime` int(8) NOT NULL DEFAULT '0',
  `groupid` int(10) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(4) NOT NULL DEFAULT '0',
  `level` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `lfp` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `lfg` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `mailkey` char(16) NOT NULL,
  `xtargets` tinyint(3) unsigned NOT NULL DEFAULT '5',
  `firstlogon` tinyint(3) NOT NULL DEFAULT '0',
  `inspectmessage` varchar(256) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

-- Initial population of the character_lookup table --

INSERT INTO `character_lookup` (id, account_id, `name`, timelaston, x, y, z, zonename, zoneid, instanceid, pktime, groupid, class, `level`, lfp, lfg, mailkey, xtargets, firstlogon, inspectmessage)
SELECT id, account_id, `name`, timelaston, x, y, z, zonename, zoneid, instanceid, pktime, groupid, class, `level`, lfp, lfg, mailkey, xtargets, firstlogon, inspectmessage
FROM `character_`;