DELETE FROM `altadv_vars` WHERE `skill_id` = '844';
DELETE FROM `altadv_vars` WHERE `skill_id` = '1319';
DELETE FROM `altadv_vars` WHERE `skill_id` = '209';
DELETE FROM `altadv_vars` WHERE `skill_id` = '1134';
DELETE FROM `altadv_vars` WHERE `skill_id` = '1158';

UPDATE `altadv_vars` SET `prereq_skill`=`prereq_skill`+1 Where `prereq_skill`>=86;
UPDATE `altadv_vars` SET `prereq_skill`=`prereq_skill`+1 Where `prereq_skill`>=226;

INSERT INTO altadv_vars (skill_id, name, cost, max_level, hotkey_sid, hotkey_sid2, title_sid, desc_sid, type, spellid, prereq_skill, prereq_minpoints, spell_type, spell_refresh, classes, berserker, class_type, cost_inc) VALUES
(844, 'Advanced Theft of Life', 5, 2, 31455, 31456, 31453, 31454, 6, 4294967295, 202, 3, 0, 0, 2080, 0, 65, 0);

INSERT INTO altadv_vars (skill_id, name, cost, max_level, hotkey_sid, hotkey_sid2, title_sid, desc_sid, type, spellid, prereq_skill, prereq_minpoints, spell_type, spell_refresh, classes, berserker, class_type, cost_inc) VALUES
(1319, 'Soul Thief', 5, 3, 31459, 31460, 31457, 31458, 7, 4294967295, 227, 2, 0, 0, 2080, 0, 68, 0);

INSERT INTO altadv_vars (skill_id, name, cost, max_level, hotkey_sid, hotkey_sid2, title_sid, desc_sid, type, spellid, prereq_skill, prereq_minpoints, spell_type, spell_refresh, classes, berserker, class_type, cost_inc) VALUES
(209, 'Death Peace', 5, 1, 13738, 13739, 13736, 13737, 7, 4294967295, 0, 0, 0, 5, 2080, 0, 65, 0);

INSERT INTO altadv_vars (skill_id, name, cost, max_level, hotkey_sid, hotkey_sid2, title_sid, desc_sid, type, spellid, prereq_skill, prereq_minpoints, spell_type, spell_refresh, classes, berserker, class_type, cost_inc) VALUES
(1134, 'Blur of Axes', 3, 3, 31463, 31464, 31461, 31462, 5, 4294967295, 0, 0, 0, 0, 0, 1, 61, 1); 

INSERT INTO altadv_vars (skill_id, name, cost, max_level, hotkey_sid, hotkey_sid2, title_sid, desc_sid, type, spellid, prereq_skill, prereq_minpoints, spell_type, spell_refresh, classes, berserker, class_type, cost_inc) VALUES
(1158, 'Vicious Frenzy', 4, 5, 31467, 31468, 31465, 31466, 7, 4294967295, 250, 3, 0, 0, 0, 1, 67, 0);

INSERT INTO aa_actions (aaid, rank, reuse_time, spell_id, target, nonspell_action, nonspell_mana, nonspell_duration, redux_aa, redux_rate) VALUES
(209, 0, 5, 5919, 0, 0, 0, 0, 0, 0); 