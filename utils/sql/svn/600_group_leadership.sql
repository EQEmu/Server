ALTER TABLE `group_leaders` ADD `assist` VARCHAR( 64 ) NOT NULL ,
ADD `marknpc` VARCHAR( 64 ) NOT NULL DEFAULT '',
ADD `leadershipaa` TINYBLOB NOT NULL DEFAULT '';

INSERT INTO `rule_values` VALUES (1,'Character:KillsPerRaidLeadershipAA','50','');
INSERT INTO `rule_values` VALUES (1,'Character:KillsPerGroupLeadershipAA','50','');
