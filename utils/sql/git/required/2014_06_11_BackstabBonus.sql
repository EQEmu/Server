INSERT INTO `rule_values` VALUES ('Combat:BackstabBonus', 0, 'Modify Backstab skill damage by percent.');
ALTER TABLE `player_corpses` CHANGE IsBurried isBuried tinyint(3);
ALTER TABLE `player_corpses` CHANGE rezzed isResurrected tinyint(3);
ALTER TABLE `player_corpses_backup` CHANGE IsBurried isBuried tinyint(3);
ALTER TABLE `player_corpses_backup` CHANGE rezzed isResurrected tinyint(3);