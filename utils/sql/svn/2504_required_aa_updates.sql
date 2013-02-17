-- Fix for Extended Ingenuity
UPDATE altadv_vars SET max_level = 3 WHERE skill_id = 8232;
UPDATE altadv_vars SET sof_max_level = 4 WHERE skill_id = 8232;
UPDATE altadv_vars SET sof_max_level = 4 WHERE skill_id = 8261;

REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8232', '1', '128', '5', '5');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8232', '2', '138', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8232', '3', '140', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8232', '4', '311', '0', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8232', '5', '411', '66434', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8232', '6', '137', '-40', '0');

REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8233', '1', '128', '15', '15');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8233', '2', '138', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8233', '3', '140', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8233', '4', '311', '0', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8233', '5', '411', '66434', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8233', '6', '137', '-40', '0');

REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8234', '1', '128', '25', '25');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8234', '2', '138', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8234', '3', '140', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8234', '4', '311', '0', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8234', '5', '411', '66434', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8234', '6', '137', '-40', '0');

REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8261', '1', '128', '65', '65');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8261', '2', '138', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8261', '3', '140', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8261', '4', '311', '0', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8261', '5', '411', '66434', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('8261', '6', '137', '-40', '0');
