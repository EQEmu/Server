--
-- Table structure for table `item_tick`
--

CREATE TABLE IF NOT EXISTS `item_tick` (
  `it_itemid` int(11) NOT NULL,
  `it_chance` int(11) NOT NULL,
  `it_level` int(11) NOT NULL,
  `it_id` int(11) NOT NULL AUTO_INCREMENT,
  `it_qglobal` varchar(50) NOT NULL,
  `it_bagslot` tinyint(4) NOT NULL,
  PRIMARY KEY (`it_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;
