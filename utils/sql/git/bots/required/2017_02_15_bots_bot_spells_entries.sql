-- Delete any existing `bots_spells_entries` table
DROP TABLE IF EXISTS `bots_spells_entries`;

-- Create new bot spells entries table (new table does not have spells_id_spellid constraint)
CREATE TABLE `bot_spells_entries` (
	`id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_spells_id` INT(11) NOT NULL DEFAULT '0',
	`spellid` SMALLINT(5) NOT NULL DEFAULT '0',
	`type` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`minlevel` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`maxlevel` TINYINT(3) UNSIGNED NOT NULL DEFAULT '255',
	`manacost` SMALLINT(5) NOT NULL DEFAULT '-1',
	`recast_delay` INT(11) NOT NULL DEFAULT '-1',
	`priority` SMALLINT(5) NOT NULL DEFAULT '0',
	`resist_adjust` INT(11) NULL DEFAULT NULL,
	PRIMARY KEY (`id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1
;

-- Copy bots spells into new table
INSERT INTO `bot_spells_entries` (`npc_spells_id`, `spellid`, `type`, `minlevel`, `maxlevel`, `manacost`, `recast_delay`, `priority`, `resist_adjust`)
SELECT `npc_spells_id`, `spellid`, `type`, `minlevel`, `maxlevel`, `manacost`, `recast_delay`, `priority`, `resist_adjust`
FROM `npc_spells_entries` WHERE `npc_spells_id` >= '701' AND `npc_spells_id` <= '712';

-- Delete bot spells from old table
DELETE FROM `npc_spells_entries` WHERE `npc_spells_id` >= '701' AND `npc_spells_id` <= '712';
