ALTER TABLE  `character_` ADD  `xtargets` TINYINT UNSIGNED NOT NULL DEFAULT  '5';
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Character:EnableXTargetting', 'true', '');
