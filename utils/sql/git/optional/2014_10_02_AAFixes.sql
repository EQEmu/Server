-- Fix Salvage
UPDATE `aa_effects` SET `effectid` = '331' WHERE `aaid` IN (997, 998, 999);
-- Rapid Strikes missing entries
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('818', '1', '279', '17', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('819', '1', '279', '19', '0');
-- Secondary Forte fixes client side display issues
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('691', '1', '248', '100', '0');
-- Packrat this is what live uses
UPDATE `aa_effects` SET `base1` = '3' WHERE `aaid` = 678;
UPDATE `aa_effects` SET `base1` = '6' WHERE `aaid` = 679;
UPDATE `aa_effects` SET `base1` = '9' WHERE `aaid` = 680;
UPDATE `aa_effects` SET `base1` = '12' WHERE `aaid` = 681;
UPDATE `aa_effects` SET `base1` = '15' WHERE `aaid` = 682;
