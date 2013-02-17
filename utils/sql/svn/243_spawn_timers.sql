DROP TABLE IF EXISTS `respawn_times`;
CREATE TABLE `respawn_times` (
  `id` int(11) NOT NULL default '0',
  `start` int(11) NOT NULL default '0',
  `duration` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `spawn2` DROP `timeleft`;