SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for `qs_player_npc_kill_record`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_npc_kill_record`;
CREATE TABLE `qs_player_npc_kill_record` (
  `fight_id` int(11) NOT NULL AUTO_INCREMENT,
  `npc_id` int(11) DEFAULT NULL,
  `type` int(11) DEFAULT NULL,
  `zone_id` int(11) DEFAULT NULL,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`fight_id`)
) ENGINE=InnoDB AUTO_INCREMENT=67 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_player_npc_kill_record_entries`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_npc_kill_record_entries`;
CREATE TABLE `qs_player_npc_kill_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_id` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
