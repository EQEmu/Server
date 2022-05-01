SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for character_peqzone_flags
-- ----------------------------
DROP TABLE IF EXISTS `character_peqzone_flags`;
CREATE TABLE `character_peqzone_flags`  (
  `id` int NOT NULL DEFAULT 0,
  `zone_id` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`, `zone_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

SET FOREIGN_KEY_CHECKS = 1;
