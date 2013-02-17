UPDATE `rule_values` SET `rule_value`='6' WHERE `ruleset_id`=1 AND `rule_name`='Combat:BerserkBaseCritChance';
UPDATE `rule_values` SET `rule_value`='0', `rule_name`='Combat:MeleeBaseCritChance' WHERE `rule_name`='Combat:BaseCritChance';
UPDATE `rule_values` SET `rule_value`='3' WHERE `rule_name`='Combat:WarBerBaseCritChance';
UPDATE `rule_values` SET `rule_value`='0' WHERE `rule_name`='Combat:ClientBaseCritChance';

UPDATE `aa_effects` SET `base2`=-1 WHERE `aaid`=113 LIMIT 1;
UPDATE `aa_effects` SET `base2`=-1 WHERE `aaid`=114 LIMIT 1;
UPDATE `aa_effects` SET `base2`=-1 WHERE `aaid`=115 LIMIT 1;

UPDATE `aa_effects` SET `base2`=-1 WHERE `aaid`=443 LIMIT 1;
UPDATE `aa_effects` SET `base2`=-1 WHERE `aaid`=444 LIMIT 1;
UPDATE `aa_effects` SET `base2`=-1 WHERE `aaid`=445 LIMIT 1;