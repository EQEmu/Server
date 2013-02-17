-- Corrections to 2176_aa_updates.sql
-- Strengthed Strike
UPDATE `altadv_vars` SET `sof_type` = 3 ,`sof_next_skill` = 915 WHERE `skill_id` = 915;
-- Untamed Rage
UPDATE `altadv_vars` SET `sof_type` = 3, `hotkey_sid` = 1150, `hotkey_sid2` = 1150, `title_sid` = 1150, `desc_sid` = 1150, `spellid` = 5848 WHERE `skill_id` = 1150;

-- New Updates

-- Warcry
-- UPDATE `altadv_vars` SET `spellid` = 5229 WHERE `skill_id` = 260;
REPLACE INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('260', 'Warcry', '3', '3', '13797', '4294967295', '13795', '13796', '3', '5229', '117', '3', '3', '2160', '2', '0', '59', '3', '3', '4294967295', '3', '0', '1', '260', '1', '0');

-- Veteran's Wrath (x4 each for different classes)
REPLACE INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('1041', 'Veteran\'s Wrath', '3', '3', '4294967295', '4294967295', '30224', '30225', '7', '4294967295', '445', '3', '0', '0', '33192', '1', '67', '3', '8', '4294967295', '2', '0', '1', '1041', '1', '0');
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('1050', 'Veteran\'s Wrath', '3', '3', '4294967295', '4294967295', '1050', '1050', '7', '4294967295', '443', '3', '0', '0', '2', '1', '67', '3', '8', '4294967295', '2', '0', '1', '1050', '1', '0');
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('1044', 'Veteran\'s Wrath', '3', '3', '4294967295', '4294967295', '1044', '1044', '7', '4294967295', '443', '3', '0', '0', '512', '0', '67', '3', '8', '4294967295', '2', '0', '1', '1044', '1', '0');
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('1047', 'Veteran\'s Wrath', '3', '3', '4294967295', '4294967295', '1047', '1047', '7', '4294967295', '443', '3', '0', '0', '16', '0', '67', '3', '8', '4294967295', '2', '0', '1', '1047', '1', '0');

INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1041', '1', '330', '25', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1041', '2', '330', '25', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1041', '3', '330', '25', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1041', '4', '330', '25', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1041', '5', '330', '25', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1041', '6', '330', '25', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1042', '1', '330', '30', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1042', '2', '330', '30', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1042', '3', '330', '30', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1042', '4', '330', '30', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1042', '5', '330', '30', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1042', '6', '330', '30', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1043', '1', '330', '35', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1043', '2', '330', '35', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1043', '3', '330', '35', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1043', '4', '330', '35', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1043', '5', '330', '35', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1043', '6', '330', '35', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1044', '1', '330', '25', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1044', '2', '330', '25', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1044', '3', '330', '25', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1044', '4', '330', '25', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1044', '5', '330', '25', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1044', '6', '330', '25', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1045', '1', '330', '30', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1045', '2', '330', '30', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1045', '3', '330', '30', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1045', '4', '330', '30', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1045', '5', '330', '30', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1045', '6', '330', '30', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1046', '1', '330', '35', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1046', '2', '330', '35', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1046', '3', '330', '35', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1046', '4', '330', '35', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1046', '5', '330', '35', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1046', '6', '330', '35', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1047', '1', '330', '25', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1047', '2', '330', '25', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1047', '3', '330', '25', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1047', '4', '330', '25', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1047', '5', '330', '25', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1047', '6', '330', '25', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1048', '1', '330', '30', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1048', '2', '330', '30', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1048', '3', '330', '30', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1048', '4', '330', '30', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1048', '5', '330', '30', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1048', '6', '330', '30', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1049', '1', '330', '35', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1049', '2', '330', '35', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1049', '3', '330', '35', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1049', '4', '330', '35', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1049', '5', '330', '35', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1049', '6', '330', '35', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1050', '1', '330', '15', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1050', '2', '330', '15', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1050', '3', '330', '15', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1050', '4', '330', '15', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1050', '5', '330', '15', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1050', '6', '330', '15', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1051', '1', '330', '20', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1051', '2', '330', '20', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1051', '3', '330', '20', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1051', '4', '330', '20', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1051', '5', '330', '20', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1051', '6', '330', '20', '36');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1052', '1', '330', '25', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1052', '2', '330', '25', '1');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1052', '3', '330', '25', '2');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1052', '4', '330', '25', '3');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1052', '5', '330', '25', '28');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1052', '6', '330', '25', '36');

-- CombatFury/Fury of the Ages [198% at Rank 3 + 3)
REPLACE INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('443', 'Fury of the Ages', '3', '3', '4294967295', '4294967295', '5620', '5621', '5', '4294967295', '113', '3', '0', '0', '33722', '1', '62', '0', '4', '4294967295', '3', '0', '1', '113', '1', '0');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('113', '1', '169', '15', '-1');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('114', '1', '169', '30', '-1');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('115', '1', '169', '45', '-1');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('443', '1', '169', '50', '-1');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('444', '1', '169', '100', '-1');
REPLACE INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES ('445', '1', '169', '153', '-1');

-- Slay Undead/Vanquish Undead
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('1524', 'Vanquish Undead', '3', '3', '4294967295', '4294967295', '1524', '1524', '3', '4294967295', '190', '3', '0', '0', '8', '0', '59', '3', '10', '4294967295', '3', '0', '1', '190', '1', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('190', '1', '219', '225', '680');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('191', '1', '219', '235', '725');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('192', '1', '219', '245', '770');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1524', '1', '219', '485', '815');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1525', '1', '219', '495', '860');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('1526', '1', '219', '515', '905');

-- Elemental Fury
REPLACE INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('790', 'Elemental Fury', '3', '5', '4294967295', '4294967295', '790', '790', '6', '0', '0', '0', '0', '0', '8192', '0', '65', '0', '7', '4294967295', '2', '0', '1', '790', '1', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('790', '1', '218', '1', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('791', '1', '218', '2', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('792', '1', '218', '3', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('793', '1', '218', '4', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('794', '1', '218', '5', '0');

-- Death's Fury
REPLACE INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('834', 'Deaths Fury', '3', '5', '4294967295', '4294967295', '834', '834', '6', '0', '0', '0', '0', '0', '2048', '0', '65', '0', '7', '4294967295', '3', '0', '1', '834', '1', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('834', '1', '218', '1', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('835', '1', '218', '2', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('836', '1', '218', '3', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('837', '1', '218', '4', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('838', '1', '218', '5', '0');

-- Warder's Fury
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('724', '1', '218', '1', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('725', '1', '218', '2', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('726', '1', '218', '3', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('727', '1', '218', '4', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('728', '1', '218', '5', '0');

-- Compainion's Fury 
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('8201', 'Companion\'s Fury', '7', '3', '4294967295', '4294967295', '8201', '8201', '6', '4294967295', '0', '0', '0', '0', '60448', '0', '81', '0', '15', '4294967295', '2', '0', '1', '8201', '4', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('8201', '1', '218', '1', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('8202', '1', '218', '2', '0');
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`)  VALUES  ('8203', '1', '218', '3', '0');

-- Ancestral Aid
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('1327', 'Ancestral Aid', '5', '3', '1327', '1327', '1327', '1327', '7', '5933', '0', '0', '2', '900', '1024', '0', '67', '0', '8', '4294967295', '3', '0', '1', '1327', '1', '0');
INSERT INTO `aa_actions` (`aaid`, `rank`, `reuse_time`, `spell_id`, `target`, `nonspell_action`, `nonspell_mana`, `nonspell_duration`, `redux_aa`, `redux_rate`, `redux_aa2`, `redux_rate2`) VALUES('1327', '0', '900', '5933', '2', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `aa_actions` (`aaid`, `rank`, `reuse_time`, `spell_id`, `target`, `nonspell_action`, `nonspell_mana`, `nonspell_duration`, `redux_aa`, `redux_rate`, `redux_aa2`, `redux_rate2`) VALUES ('1327', '1', '900', '5934', '2', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `aa_actions` (`aaid`, `rank`, `reuse_time`, `spell_id`, `target`, `nonspell_action`, `nonspell_mana`, `nonspell_duration`, `redux_aa`, `redux_rate`, `redux_aa2`, `redux_rate2`) VALUES ('1327', '2', '900', '5935', '2', '0', '0', '0', '0', '0', '0', '0');

-- Spirit Channeling
INSERT INTO `altadv_vars` (`skill_id`, `name`, `cost`, `max_level`, `hotkey_sid`, `hotkey_sid2`, `title_sid`, `desc_sid`, `type`, `spellid`, `prereq_skill`, `prereq_minpoints`, `spell_type`, `spell_refresh`, `classes`, `berserker`, `class_type`, `cost_inc`, `aa_expansion`, `special_category`, `sof_type`, `sof_cost_inc`, `sof_max_level`, `sof_next_skill`, `clientver`, `account_time_required`) VALUES ('1323', 'Spiritual Channeling', '12', '1', '1323', '1323', '1323', '1323', '1', '5932', '146', '1', '0', '2160', '1024', '0', '70', '0', '8', '4294967295', '3', '0', '1', '1323', '1', '0');
INSERT INTO `aa_actions` (`aaid`, `rank`, `reuse_time`, `spell_id`, `target`, `nonspell_action`, `nonspell_mana`, `nonspell_duration`, `redux_aa`, `redux_rate`, `redux_aa2`, `redux_rate2`) VALUES('1323', '0', '2160', '5932', '1', '0', '0', '0', '0', '0', '0', '0');

