DROP TABLE IF EXISTS `trader`;
CREATE TABLE `trader` (
  `char_id` int(10) unsigned NOT NULL default '0',
  `item_id` int(10) unsigned NOT NULL default '0',
  `serialnumber` int(10) unsigned NOT NULL default '0',
  `charges` int(11) NOT NULL default '0',
  `item_cost` int(10) unsigned NOT NULL default '0',
  `slot_id` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`slot_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `trader_audit`;
CREATE TABLE `trader_audit` (
  `time` datetime NOT NULL,
  `seller` varchar(64) NOT NULL,
  `buyer` varchar(64) NOT NULL,
  `itemname` varchar(64) NOT NULL,
  `quantity` int(11) NOT NULL,
  `totalcost` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

UPDATE doors set opentype=155 where opentype=154 and zone='bazaar';
