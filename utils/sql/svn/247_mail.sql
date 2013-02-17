CREATE TABLE `mail` (
  `msgid` int(10) unsigned NOT NULL auto_increment,
  `charid` int(10) unsigned NOT NULL,
  `timestamp` int(11) NOT NULL default '0',
  `from` varchar(100) NOT NULL,
  `subject` varchar(200) NOT NULL,
  `body` text NOT NULL,
  `to` text NOT NULL,
  `status` tinyint(4) NOT NULL,
  PRIMARY KEY  (`msgid`),
  KEY `charid` (`charid`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

ALTER TABLE `character_` ADD `mailkey` CHAR( 16 ) NOT NULL default '0';
