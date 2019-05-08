CREATE TABLE IF NOT EXISTS `spawn2_groups` (
  `spawn2_id` int(11) unsigned NOT NULL,
  `cond_value` mediumint(9) unsigned NOT NULL,
  `spawngroupID` int(11) unsigned NOT NULL,
  PRIMARY KEY (`spawn2_id`,`cond_value`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
