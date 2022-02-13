SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for bot_create_combinations
-- ----------------------------
DROP TABLE IF EXISTS `bot_create_combinations`;
CREATE TABLE `bot_create_combinations`  (
  `race` int UNSIGNED NOT NULL DEFAULT 0,
  `classes` int UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`race`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Records of bot_create_combinations
-- ----------------------------
INSERT INTO `bot_create_combinations` VALUES (1, 15871); -- Human
INSERT INTO `bot_create_combinations` VALUES (2, 49921); -- Barbarian
INSERT INTO `bot_create_combinations` VALUES (3, 15382); -- Erudite
INSERT INTO `bot_create_combinations` VALUES (4, 425); -- Wood Elf
INSERT INTO `bot_create_combinations` VALUES (5, 14342); -- High Elf
INSERT INTO `bot_create_combinations` VALUES (6, 15635); -- Dark Elf
INSERT INTO `bot_create_combinations` VALUES (7, 429); -- Half Elf
INSERT INTO `bot_create_combinations` VALUES (8, 33031); -- Dwarf
INSERT INTO `bot_create_combinations` VALUES (9, 49681); -- Troll
INSERT INTO `bot_create_combinations` VALUES (10, 49681); -- Ogre
INSERT INTO `bot_create_combinations` VALUES (11, 303); -- Halfling
INSERT INTO `bot_create_combinations` VALUES (12, 15639); -- Gnome
INSERT INTO `bot_create_combinations` VALUES (128, 18001); -- Iksar
INSERT INTO `bot_create_combinations` VALUES (130, 50049); -- Vah Shir
INSERT INTO `bot_create_combinations` VALUES (330, 3863); -- Froglok
INSERT INTO `bot_create_combinations` VALUES (522, 15871); -- Drakkin

SET FOREIGN_KEY_CHECKS = 1;
