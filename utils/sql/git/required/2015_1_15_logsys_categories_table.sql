/*
Navicat MySQL Data Transfer

Source Server         : localhost
Source Server Version : 50505
Source Host           : localhost:3306
Source Database       : ez

Target Server Type    : MYSQL
Target Server Version : 50505
File Encoding         : 65001

Date: 2015-01-20 20:00:17
*/

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
INSERT INTO `logsys_categories` VALUES ('1', 'AA', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('2', 'AI', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('3', 'Aggro', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('4', 'Attack', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('5', 'Client_Server_Packet', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('6', 'Combat', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('7', 'Commands', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('8', 'Crash', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('9', 'Debug', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('10', 'Doors', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('11', 'Error', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('12', 'Guilds', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('13', 'Inventory', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('14', 'Launcher', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('15', 'Netcode', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('16', 'Normal', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('17', 'Object', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('18', 'Pathing', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('19', 'QS_Server', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('20', 'Quests', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('21', 'Rules', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('22', 'Skills', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('23', 'Spawns', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('24', 'Spells', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('25', 'Status', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('26', 'TCP_Connection', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('27', 'Tasks', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('28', 'Tradeskills', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('29', 'Trading', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('30', 'Tribute', '0', '1', '0');
INSERT INTO `logsys_categories` VALUES ('31', 'UCS_Server', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('32', 'WebInterface_Server', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('33', 'World_Server', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('34', 'Zone Server', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('35', 'MySQL Error', '1', '1', '1');
INSERT INTO `logsys_categories` VALUES ('36', 'MySQL Queries', '0', '1', '0');
