--
-- Table structure for table `lfguild`
--

CREATE TABLE IF NOT EXISTS `lfguild` (
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `name` varchar(32) NOT NULL,
  `comment` varchar(256) NOT NULL,
  `fromlevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `tolevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `classes` int(10) unsigned NOT NULL DEFAULT '0',
  `aacount` int(10) unsigned NOT NULL DEFAULT '0',
  `timezone` int(10) unsigned NOT NULL DEFAULT '0',
  `timeposted` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`type`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
