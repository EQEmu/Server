INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES
(1, 'Bots:CommandSpellRank', '1', 'Filters bot command spells by rank (1, 2 and 3 are valid filters - any other number allows all ranks)'),
(1, 'Bots:HealRotationMaxMembers', '24', 'Maximum number of heal rotation members'),
(1, 'Bots:HealRotationMaxTargets', '12', 'Maximum number of heal rotation targets'),
(1, 'Bots:PreferNoManaCommandSpells', 'true', 'Give sorting priority to newer no-mana spells (i.e., \'Bind Affinity\')');
