-- Re-ordered entries according to actual class values and added melee types (for future expansion)
DELETE FROM `npc_spells` WHERE `id` >= '701' AND `id` <= '712';

INSERT INTO `npc_spells` VALUES (3001, 'Warrior Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3002, 'Cleric Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3003, 'Paladin Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3004, 'Ranger Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3005, 'Shadowknight Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3006, 'Druid Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3007, 'Monk Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3008, 'Bard Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3009, 'Rogue Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3010, 'Shaman Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3011, 'Necromancer Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3012, 'Wizard Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3013, 'Magician Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3014, 'Enchanter Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3015, 'Beastlord Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3016, 'Berserker Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
