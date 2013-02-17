DROP TABLE IF EXISTS `botgroups`;
CREATE TABLE `botgroups` (
  `groupid` int(11) NOT NULL default '0',
  `charid` int(11) NOT NULL default '0',
  `botid` int(11) NOT NULL default '0',
  `slot` int(11) NOT NULL default '0',
  PRIMARY KEY  (`botid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
