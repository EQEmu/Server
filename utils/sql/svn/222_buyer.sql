DROP TABLE IF EXISTS `buyer`;
CREATE TABLE `buyer` (
  `charid` int(11) NOT NULL,
  `buyslot` int(11) NOT NULL,
  `itemid` int(11) NOT NULL,
  `itemname` varchar(65) NOT NULL,
  `quantity` int(11) NOT NULL,
  `price` int(11) NOT NULL,
  PRIMARY KEY  (`charid`,`buyslot`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

ALTER TABLE `trader_audit` ADD `trantype` TINYINT NOT NULL DEFAULT '0';

