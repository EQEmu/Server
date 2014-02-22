-- UPDATE BUFF TABLES
ALTER TABLE `character_buffs` CHANGE `death_save_chance` `dot_rune` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` CHANGE `DeathSaveSuccessChance` `dot_rune` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` CHANGE `DeathSaveSuccessChance` `dot_rune` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` CHANGE `death_save_aa_chance` `caston_x` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` CHANGE `CasterAARank` `caston_x` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` CHANGE `CasterAARank` `caston_x` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `spells_new` CHANGE `not_reflectable` `reflectable` INT(11) NOT NULL DEFAULT '0';