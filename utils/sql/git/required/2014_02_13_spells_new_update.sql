ALTER TABLE `spells_new` CHANGE `field161` `not_reflectable` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field151` `no_partial_resist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field189` `MinResist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field190` `MaxResist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field194` `ConeStartAngle` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field195` `ConeStopAngle` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field208` `rank` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field159` `npc_no_los` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field213` `NotOutofCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field214` `NotInCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field168` `IsDiscipline` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field211` `CastRestriction` INT(11) NOT NULL DEFAULT '0';

UPDATE altadv_vars SET sof_next_id = 8261 WHERE skill_id = 8232;
UPDATE altadv_vars SET sof_next_id = 0 WHERE skill_id = 8261;
UPDATE altadv_vars SET sof_current_level = 3 WHERE skill_id = 8261;

