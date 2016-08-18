ALTER TABLE `character_buffs` CHANGE COLUMN `ticsremaining` `ticsremaining` INT(11) SIGNED NOT NULL;
ALTER TABLE `merc_buffs` CHANGE COLUMN `TicsRemaining` `TicsRemaining` INT(11) SIGNED NOT NULL DEFAULT 0;
