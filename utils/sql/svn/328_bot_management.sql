DROP TABLE IF EXISTS `botleader`;
CREATE TABLE `botleader` (
  `botid` int(11) NOT NULL default '0',
  `leaderid` int(11) default NULL,
  `bot_name` text NOT NULL,
  `zone_name` text NOT NULL,
  PRIMARY KEY  (`botid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
DELETE from rule_values where rule_name='EQOffline:BotCount';

