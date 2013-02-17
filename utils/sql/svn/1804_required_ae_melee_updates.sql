-- Persistent Casting
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES (692, 1, 229, 5, 0);
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES (693, 1, 229, 12, 0);
INSERT INTO `aa_effects` (`aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES (694, 1, 229, 25, 0);
-- Destructive Force
UPDATE `aa_actions` SET `nonspell_action`=0 WHERE `aaid`=828 AND `rank`=0 LIMIT 1;
UPDATE `aa_actions` SET `nonspell_action`=0 WHERE `aaid`=828 AND `rank`=1 LIMIT 1;
UPDATE `aa_actions` SET `nonspell_action`=0 WHERE `aaid`=828 AND `rank`=2 LIMIT 1;
-- Rampage
UPDATE `aa_actions` SET `spell_id`=5233, `nonspell_action`=0 WHERE `aaid`=258 AND `rank`=0 LIMIT 1;
UPDATE `altadv_vars` SET `spellid`=5233 WHERE `skill_id`=258 LIMIT 1;