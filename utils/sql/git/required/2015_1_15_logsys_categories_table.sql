
SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for logsys_categories
-- ----------------------------
DROP TABLE IF EXISTS `logsys_categories`;
CREATE TABLE `logsys_categories` (
  `log_category_id` int(11) NOT NULL,
  `log_category_description` varchar(150) DEFAULT NULL,
  `log_to_console` smallint(11) DEFAULT '0',
  `log_to_file` smallint(11) DEFAULT '0',
  `log_to_gmsay` smallint(11) DEFAULT '0',
  PRIMARY KEY (`log_category_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of logsys_categories
-- ----------------------------
INSERT INTO `logsys_categories` VALUES ('1', 'Zone_Server', '1', '0', '1');
INSERT INTO `logsys_categories` VALUES ('2', 'World_Server', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('3', 'UCS_Server', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('4', 'QS_Server', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('5', 'WebInterface_Server', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('6', 'AA', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('7', 'Doors', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('8', 'Guilds', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('9', 'Inventory', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('10', 'Launcher', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('11', 'Netcode - Does not log to gmsay for loop reasons', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('12', 'Object', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('13', 'Rules', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('14', 'Skills', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('15', 'Spawns', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('16', 'Spells', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('17', 'Tasks', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('18', 'Trading', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('19', 'Tradeskills', '0', '0', '1');
INSERT INTO `logsys_categories` VALUES ('20', 'Tribute', '0', '0', '1');
