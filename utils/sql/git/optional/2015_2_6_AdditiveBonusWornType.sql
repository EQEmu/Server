INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Spells:AdditiveBonusWornType', '0', 'Calcs worn bonuses to add together (instead of taking highest) if item set to THIS worn type. Will stack with regular worn bonuses. (2=Will cause all live items to use this behavior)');

-- This is no longer used - Set the above value equal to 2 to achieve the same effect.
DELETE FROM `rule_values` WHERE rule_name LIKE "Spells:AdditiveBonusValues";
