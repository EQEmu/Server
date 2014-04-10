INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Spells:FearBreakCheckChance', '70', 'Chance for fear to do a resist check each tick. Decrease for longer fears.');

-- Updates rule value if server is using the OLD DEFAULT values
UPDATE rule_values SET rule_value = 55 WHERE rule_name LIKE 'Spells:RootBreakFromSpells' AND rule_value = 20;
UPDATE rule_values SET rule_value = 70 WHERE rule_name LIKE 'Spells:RootBreakCheckChance' AND rule_value = 40;
UPDATE rule_values SET rule_value = 255 WHERE rule_name LIKE 'Spells:CharismaResistCap' AND rule_value = 200;
