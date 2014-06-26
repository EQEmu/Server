-- AA MGB update
UPDATE altadv_vars SET spellid = 5228 WHERE skill_id = 128;
UPDATE aa_actions SET spell_id = 5228, nonspell_action = 0 WHERE aaid = 128;

-- AA Project Illusion update
UPDATE altadv_vars SET spellid = 5227 WHERE skill_id = 643;
UPDATE aa_actions SET spell_id = 5227, nonspell_action = 0 WHERE aaid = 643;

-- spells_new update
ALTER TABLE `spells_new` CHANGE `field175` `numhits_type` INT(11) NOT NULL DEFAULT '0';