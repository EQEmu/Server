CREATE TABLE `perl_event_export_settings` (
  `event_id` int(11) NOT NULL,
  `event_description` varchar(150) DEFAULT NULL,
  `export_qglobals` smallint(11) DEFAULT '0',
  `export_mob` smallint(11) DEFAULT '0',
  `export_zone` smallint(11) DEFAULT '0',
  `export_item` smallint(11) DEFAULT '0',
  `export_event` smallint(11) DEFAULT '0',
  PRIMARY KEY (`event_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of perl_event_export_settings
-- ----------------------------
INSERT INTO `perl_event_export_settings` VALUES ('0', 'EVENT_SAY', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('1', 'EVENT_ITEM', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('2', 'EVENT_DEATH', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('3', 'EVENT_SPAWN', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('4', 'EVENT_ATTACK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('5', 'EVENT_COMBAT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('6', 'EVENT_AGGRO', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('7', 'EVENT_SLAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('8', 'EVENT_NPC_SLAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('9', 'EVENT_WAYPOINT_ARRIVE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('10', 'EVENT_WAYPOINT_DEPART', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('11', 'EVENT_TIMER', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('12', 'EVENT_SIGNAL', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('13', 'EVENT_HP', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('14', 'EVENT_ENTER', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('15', 'EVENT_EXIT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('16', 'EVENT_ENTERZONE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('17', 'EVENT_CLICKDOOR', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('18', 'EVENT_LOOT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('19', 'EVENT_ZONE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('20', 'EVENT_LEVEL_UP', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('21', 'EVENT_KILLED_MERIT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('22', 'EVENT_CAST_ON', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('23', 'EVENT_TASKACCEPTED', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('24', 'EVENT_TASK_STAGE_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('25', 'EVENT_TASK_UPDATE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('26', 'EVENT_TASK_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('27', 'EVENT_TASK_FAIL', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('28', 'EVENT_AGGRO_SAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('29', 'EVENT_PLAYER_PICKUP', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('30', 'EVENT_POPUPRESPONSE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('31', 'EVENT_ENVIRONMENTAL_DAMAGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('32', 'EVENT_PROXIMITY_SAY', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('33', 'EVENT_CAST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('34', 'EVENT_CAST_BEGIN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('35', 'EVENT_SCALE_CALC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('36', 'EVENT_ITEM_ENTER_ZONE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('37', 'EVENT_TARGET_CHANGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('38', 'EVENT_HATE_LIST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('39', 'EVENT_SPELL_EFFECT_CLIENT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('40', 'EVENT_SPELL_EFFECT_NPC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('41', 'EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('42', 'EVENT_SPELL_EFFECT_BUFF_TIC_NPC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('43', 'EVENT_SPELL_FADE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('44', 'EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('45', 'EVENT_COMBINE_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('46', 'EVENT_COMBINE_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('47', 'EVENT_ITEM_CLICK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('48', 'EVENT_ITEM_CLICK_CAST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('49', 'EVENT_GROUP_CHANGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('50', 'EVENT_FORAGE_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('51', 'EVENT_FORAGE_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('52', 'EVENT_FISH_START', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('53', 'EVENT_FISH_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('54', 'EVENT_FISH_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('55', 'EVENT_CLICK_OBJECT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('56', 'EVENT_DISCOVER_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('57', 'EVENT_DISCONNECT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('58', 'EVENT_CONNECT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('59', 'EVENT_ITEM_TICK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('60', 'EVENT_DUEL_WIN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('61', 'EVENT_DUEL_LOSE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('62', 'EVENT_ENCOUNTER_LOAD', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('63', 'EVENT_ENCOUNTER_UNLOAD', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('64', 'EVENT_SAY', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('65', 'EVENT_DROP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('66', 'EVENT_DESTROY_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('67', 'EVENT_FEIGN_DEATH', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('68', 'EVENT_WEAPON_PROC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('69', 'EVENT_EQUIP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('70', 'EVENT_UNEQUIP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('71', 'EVENT_AUGMENT_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('72', 'EVENT_UNAUGMENT_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('73', 'EVENT_AUGMENT_INSERT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('74', 'EVENT_AUGMENT_REMOVE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('75', 'EVENT_ENTER_AREA', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('76', 'EVENT_LEAVE_AREA', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('77', 'EVENT_RESPAWN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('78', 'EVENT_DEATH_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('79', 'EVENT_UNHANDLED_OPCODE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('80', 'EVENT_TICK', '0', '1', '1', '0', '1');
