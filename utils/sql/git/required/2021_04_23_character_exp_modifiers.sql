CREATE TABLE `character_exp_modifiers`  (
  `character_id` int NOT NULL,
  `zone_id` int NOT NULL,
  `aa_modifier` float NOT NULL,
  `exp_modifier` float NOT NULL,
  PRIMARY KEY (`character_id`, `zone_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;
