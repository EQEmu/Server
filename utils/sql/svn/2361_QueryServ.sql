SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for `qs_merchant_transaction_record`
-- ----------------------------
DROP TABLE IF EXISTS `qs_merchant_transaction_record`;
CREATE TABLE `qs_merchant_transaction_record` (
  `transaction_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `zone_id` int(11) DEFAULT '0',
  `merchant_id` int(11) DEFAULT '0',
  `merchant_pp` int(11) DEFAULT '0',
  `merchant_gp` int(11) DEFAULT '0',
  `merchant_sp` int(11) DEFAULT '0',
  `merchant_cp` int(11) DEFAULT '0',
  `merchant_items` mediumint(7) DEFAULT '0',
  `char_id` int(11) DEFAULT '0',
  `char_pp` int(11) DEFAULT '0',
  `char_gp` int(11) DEFAULT '0',
  `char_sp` int(11) DEFAULT '0',
  `char_cp` int(11) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`transaction_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_merchant_transaction_record_entries`
-- ----------------------------
DROP TABLE IF EXISTS `qs_merchant_transaction_record_entries`;
CREATE TABLE `qs_merchant_transaction_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_player_delete_record`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_delete_record`;
CREATE TABLE `qs_player_delete_record` (
  `delete_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char_id` int(11) DEFAULT '0',
  `stack_size` mediumint(7) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`delete_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_player_delete_record_entries`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_delete_record_entries`;
CREATE TABLE `qs_player_delete_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_player_handin_record`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_handin_record`;
CREATE TABLE `qs_player_handin_record` (
  `handin_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `quest_id` int(11) DEFAULT '0',
  `char_id` int(11) DEFAULT '0',
  `char_pp` int(11) DEFAULT '0',
  `char_gp` int(11) DEFAULT '0',
  `char_sp` int(11) DEFAULT '0',
  `char_cp` int(11) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  `npc_id` int(11) DEFAULT '0',
  `npc_pp` int(11) DEFAULT '0',
  `npc_gp` int(11) DEFAULT '0',
  `npc_sp` int(11) DEFAULT '0',
  `npc_cp` int(11) DEFAULT '0',
  `npc_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`handin_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_player_handin_record_entries`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_handin_record_entries`;
CREATE TABLE `qs_player_handin_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `action_type` char(6) Default 'action',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_player_move_record`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_move_record`;
CREATE TABLE `qs_player_move_record` (
  `move_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_slot` mediumint(7) DEFAULT '0',
  `stack_size` mediumint(7) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  `postaction` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`move_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `qs_player_move_record_entries`
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_move_record_entries`;
CREATE TABLE `qs_player_move_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_slot`  mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;