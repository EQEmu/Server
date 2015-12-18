DROP TABLE IF EXISTS `eqtime`;
CREATE TABLE `eqtime` (
  `minute` tinyint(4) not null default 0,
  `hour` tinyint(4) not null default 0,
  `day` tinyint(4) not null default 0,
  `month` tinyint(4) not null default 0,
  `year` int(4) not null default 0,
  `realtime` int(11) not null default 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

INSERT INTO eqtime values (0,1,28,12,3766,1444035661);