-- Note: The data entered into the new table are only examples and can be deleted/modified as needed.

ALTER TABLE  `npc_types` ADD  `npc_spells_effects_id` int( 11 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `npc_spells_id`;

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
) ENGINE=InnoDB AUTO_INCREMENT=1080 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of npc_spells_effects
-- ----------------------------
INSERT INTO `npc_spells_effects` VALUES ('1', 'Critical Melee [All Skills]', '0');
INSERT INTO `npc_spells_effects` VALUES ('2', 'Damage Shield', '0');
INSERT INTO `npc_spells_effects` VALUES ('3', 'Melee Haste', '0');
INSERT INTO `npc_spells_effects` VALUES ('4', 'Resist Spell Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('5', 'Resist Direct Dmg Spell Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('6', 'Reflect Spell Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('7', 'Spell Damage Shield', '0');
INSERT INTO `npc_spells_effects` VALUES ('8', 'Melee Mitigation [All]', '0');
INSERT INTO `npc_spells_effects` VALUES ('9', 'Avoid Melee', '0');
INSERT INTO `npc_spells_effects` VALUES ('10', 'Riposte Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('11', 'Dodge Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('12', 'Parry Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('13', 'Decrease Dmg Taken [2HS]', '0');
INSERT INTO `npc_spells_effects` VALUES ('14', 'Increase Dmg Taken [1HS]', '0');
INSERT INTO `npc_spells_effects` VALUES ('15', 'Block Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('16', 'Melee Lifetap', '0');
INSERT INTO `npc_spells_effects` VALUES ('17', 'Hit Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('18', 'Increase Dmg [1HS]', '0');
INSERT INTO `npc_spells_effects` VALUES ('19', 'Increase Archery Dmg', '0');
INSERT INTO `npc_spells_effects` VALUES ('20', 'Flurry Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('21', 'Add Damage [2HS]', '0');
INSERT INTO `npc_spells_effects` VALUES ('22', 'Divine Aura', '0');
INSERT INTO `npc_spells_effects` VALUES ('23', 'Cast CH on Kill', '0');
INSERT INTO `npc_spells_effects` VALUES ('24', 'Critical Heal', '0');
INSERT INTO `npc_spells_effects` VALUES ('25', 'Critical Direct Dmg', '0');
INSERT INTO `npc_spells_effects` VALUES ('26', 'Heal Rate', '0');
INSERT INTO `npc_spells_effects` VALUES ('27', 'Negate Damage Shield', '0');
INSERT INTO `npc_spells_effects` VALUES ('28', 'Increase Spell Vulnerability [All]', '0');
INSERT INTO `npc_spells_effects` VALUES ('29', 'Decrease Spell Vulnerability [FR]', '0');
INSERT INTO `npc_spells_effects` VALUES ('30', 'Movement Speed', '0');


SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `npc_spells_effects_entries`
-- ----------------------------
DROP TABLE IF EXISTS `npc_spells_effects_entries`;
CREATE TABLE `npc_spells_effects_entries` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `npc_spells_effects_id` int(11) NOT NULL DEFAULT '0',
  `spell_effect_id` smallint(5) NOT NULL DEFAULT '0',
  `minlevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `maxlevel` tinyint(3) unsigned NOT NULL DEFAULT '255',
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
INSERT INTO `npc_spells_effects_entries` VALUES ('2', '2', '59', '0', '255', '-60', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('3', '3', '11', '0', '255', '150', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('4', '4', '180', '0', '255', '50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('5', '5', '378', '0', '255', '85', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('6', '6', '158', '0', '255', '50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('7', '7', '157', '0', '255', '-300', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('8', '8', '168', '0', '255', '-50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('9', '9', '172', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('10', '10', '173', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('11', '11', '174', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('12', '12', '175', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('13', '13', '197', '0', '255', '-80', '3', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('14', '14', '197', '0', '255', '80', '1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('15', '15', '188', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('16', '16', '178', '0', '255', '90', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('17', '17', '184', '0', '255', '10000', '-1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('18', '18', '185', '0', '255', '100', '1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('19', '19', '301', '0', '255', '100', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('20', '20', '279', '0', '255', '50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('21', '21', '220', '0', '255', '2000', '1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('22', '22', '40', '0', '255', '1', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('23', '23', '360', '0', '255', '100', '13', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('24', '24', '274', '0', '255', '90', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('25', '25', '294', '0', '255', '100', '200', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('26', '26', '120', '0', '255', '50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('27', '27', '382', '0', '255', '0', '55', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('28', '28', '296', '0', '255', '1000', '-1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('29', '29', '296', '0', '255', '-50', '2', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('30', '30', '3', '0', '255', '60', '0', '0');
