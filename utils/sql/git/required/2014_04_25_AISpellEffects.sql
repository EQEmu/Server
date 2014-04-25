-- TEST SQL --

ALTER TABLE  `npc_types` ADD  `npc_spells_effects` int( 11 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `npc_spells`;

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `npc_spells_effects_entries`
-- ----------------------------
DROP TABLE IF EXISTS `npc_spells_effects_entries`;
CREATE TABLE `npc_spells_effects_entries` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `npc_spells_effects_id` int(11) NOT NULL DEFAULT '0',
  `spell_effect_id` smallint(5) NOT NULL DEFAULT '0',
  `minlevel` tinyint(3) unsigned NOT NULL,
  `maxlevel` tinyint(3) unsigned NOT NULL,
  `se_base` int(11) NOT NULL DEFAULT '0',
  `se_limit` int(11) NOT NULL DEFAULT '0',
  `se_max` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `spellsid_spellid` (`npc_spells_effects_id`,`spell_effect_id`)
) ENGINE=InnoDB AUTO_INCREMENT=18374 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of npc_spells_effects_entries
-- ----------------------------
INSERT INTO `npc_spells_effects_entries` VALUES ('1', '1', '169', '0', '255', '10000', '-1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('2', '1', '168', '0', '255', '3999', '-1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('3', '2', '167', '0', '255', '98', '-1', '0');



SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `npc_spells_effects`
-- ----------------------------
DROP TABLE IF EXISTS `npc_spells_effects`;
CREATE TABLE `npc_spells_effects` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` tinytext,
  `parent_list` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1079 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of npc_spells_effects
-- ----------------------------
INSERT INTO `npc_spells_effects` VALUES ('1', 'Critical', '0');
INSERT INTO `npc_spells_effects` VALUES ('2', 'ParentTest', '1');
