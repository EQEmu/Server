DROP TABLE IF EXISTS `saylink`;
CREATE TABLE `saylink` (
  `id` int(10) NOT NULL auto_increment,
  `phrase` varchar(64) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;