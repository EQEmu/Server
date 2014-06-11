INSERT INTO `rule_values` VALUES (1, 'Combat:BackstabBonus', 0, 'Modify Backstab skill damage by percent.');
INSERT INTO `rule_values` VALUES (1, 'Combat:AvoidanceCap', 1000, 'Avoidance Cap.');
ALTER TABLE `npc_types` CHANGE d_meele_texture1 d_melee_texture1 int(10);
ALTER TABLE `npc_types` CHANGE d_meele_texture2 d_melee_texture2 int(10);
ALTER TABLE `player_corpses` CHANGE IsBurried isBuried tinyint(3);
ALTER TABLE `player_corpses` CHANGE rezzed isResurrected tinyint(3);
ALTER TABLE `player_corpses_backup` CHANGE IsBurried isBuried tinyint(3);
ALTER TABLE `player_corpses_backup` CHANGE rezzed isResurrected tinyint(3);