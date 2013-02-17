SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for `qs_player_npc_kill_record`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_trade_record`;
CREATE TABLE `qs_player_trade_record` (
  `trade_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char1_id` int(11) DEFAULT '0',
  `char1_pp` int(11) DEFAULT '0',
  `char1_gp` int(11) DEFAULT '0',
  `char1_sp` int(11) DEFAULT '0',
  `char1_cp` int(11) DEFAULT '0',
  `char1_items` mediumint(7) DEFAULT '0',
  `char2_id` int(11) DEFAULT '0',
  `char2_pp` int(11) DEFAULT '0',
  `char2_gp` int(11) DEFAULT '0',
  `char2_sp` int(11) DEFAULT '0',
  `char2_cp` int(11) DEFAULT '0',
  `char2_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`trade_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_player_npc_kill_record_entries`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_trade_record_entries`;
CREATE TABLE `qs_player_trade_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `from_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_id` int(11) DEFAULT '0',
  `to_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
