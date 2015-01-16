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
INSERT INTO `logsys_categories` VALUES ('0', 'Zone_Server', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('1', 'World_Server', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('2', 'UCS_Server', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('3', 'QS_Server', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('4', 'WebInterface_Server', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('5', 'AA', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('6', 'Doors', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('7', 'Guilds', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('8', 'Inventory', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('9', 'Launcher', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('10', 'Netcode - Does not log to gmsay for loop reasons', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('11', 'Object', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('12', 'Rules', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('13', 'Skills', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('14', 'Spawns', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('15', 'Spells', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('16', 'Tasks', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('17', 'Trading', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('18', 'Tradeskills', '0', '0', '0');
INSERT INTO `logsys_categories` VALUES ('19', 'Tribute', '0', '0', '0');
