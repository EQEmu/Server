-- Nature's Bounty
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`, `sof_current_level`, `sof_next_id`, `level_inc`) VALUES ('1230', 'Nature''s Bounty', '1', '3', '4294967295', '4294967295', '1230', '1230', '7', '0', '0', '0', '0', '0', '80', '0', '51', '1', '8', '4294967295', '3', '0', '6', '1230', '1', '0', '0', '5000', '2');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2368', '1230', '1', '313', '15', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2369', '1231', '1', '313', '20', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2370', '1232', '1', '313', '25', '0');

-- Survivalist
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`, `sof_current_level`, `sof_next_id`, `level_inc`) VALUES ('5000', 'Survivalist', '2', '3', '4294967295', '4294967295', '5000', '5000', '7', '4294967295', '1230', '3', '0', '0', '80', '0', '71', '0', '12', '4294967295', '3', '0', '6', '1230', '1', '0', '3', '0', '2');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2371', '5000', '1', '313', '50', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2372', '5001', '1', '313', '75', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2373', '5002', '1', '313', '100', '0');

