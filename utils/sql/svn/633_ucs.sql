CREATE TABLE `friends` (
  `charid` int(10) unsigned NOT NULL,
  `type` tinyint(1) unsigned NOT NULL default '1' COMMENT '1 = Friend, 0 = Ignore',
  `name` varchar(64) NOT NULL,
  PRIMARY KEY  (`charid`,`type`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
