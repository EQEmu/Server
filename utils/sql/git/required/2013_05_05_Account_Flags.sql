--
-- Table structure for table `account_flags`
--

CREATE TABLE IF NOT EXISTS `account_flags` (
  `p_accid` int(10) unsigned NOT NULL,
  `p_flag` varchar(50) NOT NULL,
  `p_value` varchar(80) NOT NULL,
  PRIMARY KEY (`p_accid`,`p_flag`),
  KEY `p_accid` (`p_accid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
