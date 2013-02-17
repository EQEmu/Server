DROP TABLE IF EXISTS `bugs`;
CREATE TABLE `bugs` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `zone` varchar(32) NOT NULL,
  `name` varchar(64) NOT NULL,
  `ui` varchar(128) NOT NULL,
  `x` float NOT NULL default '0',
  `y` float NOT NULL default '0',
  `z` float NOT NULL default '0',
  `type` varchar(64) NOT NULL,
  `flag` tinyint(3) unsigned NOT NULL,
  `target` varchar(64) default NULL,
  `bug` varchar(1024) NOT NULL,
  `date` date NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8;