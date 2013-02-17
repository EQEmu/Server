-- MISC AA fixes mostly display related

-- Fix innate cold on SOD
UPDATE altadv_vars SET sof_current_level = 0 WHERE skill_id = 42;

-- Fix for innate run speed display
UPDATE altadv_vars SET sof_current_level = 3 WHERE skill_id = 672;

-- Fix for istrument master/singing
UPDATE altadv_vars SET sof_next_id = 700 WHERE skill_id = 213;
UPDATE altadv_vars SET sof_next_id = 701 WHERE skill_id = 275;
UPDATE altadv_vars SET sof_type = 3 WHERE skill_id = 700;
UPDATE altadv_vars SET sof_type = 3 WHERE skill_id = 701;
UPDATE altadv_vars SET sof_current_level = 3 WHERE skill_id = 700;
UPDATE altadv_vars SET sof_current_level = 3 WHERE skill_id = 701;

UPDATE altadv_vars SET hotkey_sid = 4294967295 WHERE skill_id = 700;
UPDATE altadv_vars SET hotkey_sid = 4294967295 WHERE skill_id = 701;
UPDATE altadv_vars SET hotkey_sid2 = 4294967295 WHERE skill_id = 700;
UPDATE altadv_vars SET hotkey_sid2 = 4294967295 WHERE skill_id = 701;

-- Fix for Wizards having beast AA Frenzy Spirit/Warders x
UPDATE altadv_vars SET sof_next_skill = 921 WHERE skill_id = 1340;
UPDATE altadv_vars SET sof_next_skill = 922 WHERE skill_id = 1341;
UPDATE altadv_vars SET sof_next_skill = 923 WHERE skill_id = 1342;
UPDATE altadv_vars SET sof_next_skill = 155 WHERE skill_id = 1344;

-- Fix for spell casting reinforcement
UPDATE altadv_vars SET sof_max_level = 3 WHERE skill_id = 86;
UPDATE altadv_vars SET sof_max_level = 1 WHERE skill_id = 266;
UPDATE altadv_vars SET sof_next_skill = 266 WHERE skill_id = 266;

-- General Sturdiness Fix
UPDATE altadv_vars SET sof_next_id = 7527 WHERE skill_id = 6119;
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('6119', '1', '69', '500', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('6120', '1', '69', '500', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('6121', '1', '69', '500', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('6122', '1', '69', '500', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('6123', '1', '69', '500', '0');

UPDATE altadv_vars SET skill_id = 7527 WHERE skill_id = 7526;
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('7527', '1', '69', '500', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('7528', '1', '69', '500', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('7529', '1', '69', '500', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('7530', '1', '69', '500', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('7531', '1', '69', '500', '0');

-- Energetic Attument III  --
REPLACE INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '7522', '1', '317', '1', '0');
REPLACE INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '7523', '1', '317', '1', '0');
REPLACE INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '7524', '1', '317', '1', '0');
REPLACE INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '7525', '1', '317', '1', '0');
REPLACE INTO `aa_effects` (`id` ,`aaid` ,`slot` ,`effectid` ,`base1` ,`base2`) VALUES (NULL , '7526', '1', '317', '1', '0');