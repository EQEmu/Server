-- Destructive Fury II 
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('4755', '1', '294', '0', '130');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('4756', '1', '294', '0', '135');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('4757', '1', '294', '0', '140');

-- Set correct base2 values for effect 294 
UPDATE aa_effects SET base2 = 100 WHERE effectid = 294 AND base2 = 0;

-- Consumption of Soul (Need live values - These are what had hard coded, but using spell effect)
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('559', '1', '303', '200', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('559', '2', '139', '2766', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('560', '1', '303', '400', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('560', '2', '139', '2766', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('561', '1', '303', '600', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('561', '2', '139', '2766', '0');

REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('893', '1', '303', '800', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('893', '2', '139', '2766', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('894', '1', '303', '1000', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('894', '2', '139', '2766', '0');




