-- ----------------------------
-- Table structure for qs_player_events
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_events`;
CREATE TABLE `qs_player_events` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) DEFAULT '0',
  `event` int(11) unsigned DEFAULT '0',
  `event_desc` varchar(255) DEFAULT NULL,
  `time` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for qs_player_aa_rate_hourly
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_aa_rate_hourly`;
CREATE TABLE `qs_player_aa_rate_hourly` (
  `char_id` int(11) NOT NULL DEFAULT '0',
  `hour_time` int(11) NOT NULL,
  `aa_count` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`char_id`,`hour_time`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;