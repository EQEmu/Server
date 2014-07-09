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

-- AA Anatomy (Rogue Assassinate)
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`, `sof_current_level`,`sof_next_id`,`level_inc`) VALUES ('1604', 'Anatomy', '5', '3', '4294967295', '4294967295', '1604', '1604', '1', '4294967295', '0', '0', '0', '0', '512', '0', '60', '1', '10', '4294967295', '3', '0', '3', '1604', '1', '0', '0', '0', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('1604', '1', '439', '0', '32000');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('1604', '2', '345', '48', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('1605', '1', '439', '0', '32000');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('1605', '2', '345', '51', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('1606', '1', '439', '0', '32000');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('1606', '2', '345', '53', '0');

-- AA Finishing Blow Fix
DELETE FROM aa_effects WHERE aaid = 199 AND slot = 2;
DELETE FROM aa_effects WHERE aaid = 200 AND slot = 2;
DELETE FROM aa_effects WHERE aaid = 201 AND slot = 2;
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('119', '1', '278', '500', '32000');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('119', '2', '440', '50', '200');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('120', '1', '278', '500', '32000');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('120', '2', '440', '52', '200');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('121', '1', '278', '500', '32000');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('121', '2', '440', '54', '200');