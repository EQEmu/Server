CREATE TABLE `account_ip` (
  `accid` int(11) NOT NULL default '0',
  `ip` varchar(32) NOT NULL default '',
  `count` int(11) NOT NULL default 1,
  `lastused` timestamp NOT NULL default CURRENT_TIMESTAMP,
  UNIQUE KEY `ip` (`accid`,`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;