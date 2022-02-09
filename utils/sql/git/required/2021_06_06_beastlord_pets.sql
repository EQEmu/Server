SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for pets_beastlord_data
-- ----------------------------
DROP TABLE IF EXISTS `pets_beastlord_data`;
CREATE TABLE `pets_beastlord_data`  (
  `player_race` int UNSIGNED NOT NULL DEFAULT 1,
  `pet_race` int UNSIGNED NOT NULL DEFAULT 42,
  `texture` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `helm_texture` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `gender` tinyint UNSIGNED NOT NULL DEFAULT 2,
  `size_modifier` float UNSIGNED NULL DEFAULT 1,
  `face` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`player_race`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Records of pets_beastlord_data
-- ----------------------------
INSERT INTO `pets_beastlord_data` VALUES (2, 42, 2, 0, 2, 1, 0); -- Barbarian
INSERT INTO `pets_beastlord_data` VALUES (9, 91, 0, 0, 2, 2.5, 0); -- Troll
INSERT INTO `pets_beastlord_data` VALUES (10, 43, 3, 0, 2, 1, 0); -- Ogre
INSERT INTO `pets_beastlord_data` VALUES (128, 42, 0, 0, 1, 2, 0); -- Iksar
INSERT INTO `pets_beastlord_data` VALUES (130, 63, 0, 0, 2, 0.8, 0); -- Vah Shir

SET FOREIGN_KEY_CHECKS = 1;