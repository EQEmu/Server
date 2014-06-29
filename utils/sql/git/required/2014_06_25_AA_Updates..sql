-- AA MGB update
UPDATE altadv_vars SET spellid = 5228 WHERE skill_id = 128;
UPDATE aa_actions SET spell_id = 5228, nonspell_action = 0 WHERE aaid = 128;

-- AA Project Illusion update
UPDATE altadv_vars SET spellid = 5227 WHERE skill_id = 643;
UPDATE aa_actions SET spell_id = 5227, nonspell_action = 0 WHERE aaid = 643;

-- AA Improved Reclaim Energy
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('180', '1', '241', '95', '0');

-- AA Headshot
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('644', '1', '217', '0', '32000');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('644', '2', '346', '46', '0');

-- spells_new update
ALTER TABLE `spells_new` CHANGE `field175` `numhits_type` INT(11) NOT NULL DEFAULT '0';