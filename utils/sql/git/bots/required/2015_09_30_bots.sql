-- '2015_09_30_bots' sql script file
-- current as of 12/11/2015
--
-- Use eqemu_update.pl to administer this script


-- Clean-up
DROP VIEW IF EXISTS `vwbotcharactermobs`;
DROP VIEW IF EXISTS `vwbotgroups`;
DROP VIEW IF EXISTS `vwgroups`;
DROP VIEW IF EXISTS `vwguildmembers`;

DROP VIEW IF EXISTS `vwBotCharacterMobs`;
DROP VIEW IF EXISTS `vwBotGroups`;
DROP VIEW IF EXISTS `vwGroups`;
DROP VIEW IF EXISTS `vwGuildMembers`;

DROP VIEW IF EXISTS `vw_bot_character_mobs`;
DROP VIEW IF EXISTS `vw_bot_groups`;
DROP VIEW IF EXISTS `vw_groups`;
DROP VIEW IF EXISTS `vw_guild_members`;

DROP FUNCTION IF EXISTS `GetMobType`;
DROP FUNCTION IF EXISTS `GetMobTypeByName`;
DROP FUNCTION IF EXISTS `GetMobTypeByID`;

DROP PROCEDURE IF EXISTS `LoadBotsSchema`;


-- Tables
CREATE TABLE `bot_data` (
	`bot_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`owner_id` INT(11) UNSIGNED NOT NULL,
	`spells_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`name` VARCHAR(64) NOT NULL DEFAULT '',
	`last_name` VARCHAR(64) NOT NULL DEFAULT '',
	`title` VARCHAR(32) NOT NULL DEFAULT '',				-- Unused
	`suffix` VARCHAR(32) NOT NULL DEFAULT '',				-- Unused
	`zone_id` SMALLINT(6) NOT NULL DEFAULT '0',
	`gender` TINYINT(2) NOT NULL DEFAULT '0',
	`race` SMALLINT(5) NOT NULL DEFAULT '0',
	`class` TINYINT(2) NOT NULL DEFAULT '0',
	`level` TINYINT(2) UNSIGNED NOT NULL DEFAULT '0',
	`deity` INT(11) UNSIGNED NOT NULL DEFAULT '0',			-- Unused
	`creation_day` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`last_spawn` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`time_spawned` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`size` FLOAT NOT NULL DEFAULT '0',
	`face` INT(10) NOT NULL DEFAULT '1',
	`hair_color` INT(10) NOT NULL DEFAULT '1',
	`hair_style` INT(10) NOT NULL DEFAULT '1',
	`beard` INT(10) NOT NULL DEFAULT '0',
	`beard_color` INT(10) NOT NULL DEFAULT '1',
	`eye_color_1` INT(10) NOT NULL DEFAULT '1',
	`eye_color_2` INT(10) NOT NULL DEFAULT '1',
	`drakkin_heritage` INT(10) NOT NULL DEFAULT '0',
	`drakkin_tattoo` INT(10) NOT NULL DEFAULT '0',
	`drakkin_details` INT(10) NOT NULL DEFAULT '0',
	`ac` SMALLINT(5) NOT NULL DEFAULT '0',
	`atk` MEDIUMINT(9) NOT NULL DEFAULT '0',
	`hp` INTEGER NOT NULL DEFAULT '0',
	`mana` INTEGER NOT NULL DEFAULT '0',
	`str` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`sta` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`cha` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`dex` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`int` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`agi` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`wis` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`fire` SMALLINT(5) NOT NULL DEFAULT '0',
	`cold` SMALLINT(5) NOT NULL DEFAULT '0',
	`magic` SMALLINT(5) NOT NULL DEFAULT '0',
	`poison` SMALLINT(5) NOT NULL DEFAULT '0',
	`disease` SMALLINT(5) NOT NULL DEFAULT '0',
	`corruption` SMALLINT(5) NOT NULL DEFAULT '0',
	`show_helm` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`follow_distance` INT(11) UNSIGNED NOT NULL DEFAULT '200',
	PRIMARY KEY (`bot_id`)
) ENGINE=InnoDB;

CREATE TABLE `bot_inspect_messages` (
	`bot_id` INT(11) UNSIGNED NOT NULL,
	`inspect_message` VARCHAR(256) NOT NULL DEFAULT '',
	PRIMARY KEY (`bot_id`),
	INDEX `bot_id` (`bot_id`)
) ENGINE=InnoDB;

CREATE TABLE `bot_stances` (
	`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`stance_id` TINYINT UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`bot_id`),
	CONSTRAINT `FK_bot_stances_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
);

CREATE TABLE `bot_timers` (
	`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`timer_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`timer_value` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`bot_id`),
	CONSTRAINT `FK_bot_timers_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
);

CREATE TABLE `bot_buffs` (
	`buffs_index` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`spell_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`caster_level` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`duration_formula` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`tics_remaining` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`poison_counters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`disease_counters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`curse_counters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`corruption_counters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`numhits` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`melee_rune` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`magic_rune` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`dot_rune` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`persistent` TINYINT(1) NOT NULL DEFAULT '0',
	`caston_x` INT(10) NOT NULL DEFAULT '0',
	`caston_y` INT(10) NOT NULL DEFAULT '0',
	`caston_z` INT(10) NOT NULL DEFAULT '0',
	`extra_di_chance` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`instrument_mod` INT(10) NOT NULL DEFAULT '10',				-- Unused
	PRIMARY KEY (`buffs_index`),
	KEY `FK_bot_buffs_1` (`bot_id`),
	CONSTRAINT `FK_bot_buffs_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE `bot_inventories` (
	`inventories_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`slot_id` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`item_id` INT(11) UNSIGNED NULL DEFAULT '0',
	`inst_charges` TINYINT(3) UNSIGNED DEFAULT 0,
	`inst_color` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`inst_no_drop` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	`inst_custom_data` TEXT NULL,
	`ornament_icon` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornament_id_file` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornament_hero_model` INT(11) NOT NULL DEFAULT '0',
	`augment_1` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augment_2` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augment_3` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augment_4` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augment_5` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augment_6` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`inventories_index`),
	KEY `FK_bot_inventories_1` (`bot_id`),
	CONSTRAINT `FK_bot_inventories_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
) ENGINE=InnoDB;

CREATE TABLE `bot_pets` (
	`pets_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`pet_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`bot_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`name` VARCHAR(64) NULL,
	`mana` INTEGER NOT NULL DEFAULT '0',
	`hp` INTEGER NOT NULL DEFAULT '0',
	PRIMARY KEY (`pets_index`),
	KEY `FK_bot_pets_1` (`bot_id`),
	CONSTRAINT `FK_bot_pets_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`),
	CONSTRAINT `U_bot_pets_1` UNIQUE (`bot_id`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE `bot_pet_buffs` (
	`pet_buffs_index` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`pets_index` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`spell_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`caster_level` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`duration` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`pet_buffs_index`),
	KEY `FK_bot_pet_buffs_1` (`pets_index`),
	CONSTRAINT `FK_bot_pet_buffs_1` FOREIGN KEY (`pets_index`) REFERENCES `bot_pets` (`pets_index`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE `bot_pet_inventories` (
	`pet_inventories_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`pets_index` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`item_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`pet_inventories_index`),
	KEY `FK_bot_pet_inventories_1` (`pets_index`),
	CONSTRAINT `FK_bot_pet_inventories_1` FOREIGN KEY (`pets_index`) REFERENCES `bot_pets` (`pets_index`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE `bot_groups` (
	`groups_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`group_leader_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`group_name` VARCHAR(64) NOT NULL,
	PRIMARY KEY  (`groups_index`),
	KEY `FK_bot_groups_1` (`group_leader_id`),
	CONSTRAINT `FK_bot_groups_1` FOREIGN KEY (`group_leader_id`) REFERENCES `bot_data` (`bot_id`)
) ENGINE=InnoDB;

CREATE TABLE `bot_group_members` (
	`group_members_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`groups_index` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`bot_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY  (`group_members_index`),
	KEY `FK_bot_group_members_1` (`groups_index`),
	CONSTRAINT `FK_bot_group_members_1` FOREIGN KEY (`groups_index`) REFERENCES `bot_groups` (`groups_index`),
	KEY `FK_bot_group_members_2` (`bot_id`),
	CONSTRAINT `FK_bot_group_members_2` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
) ENGINE=InnoDB;

CREATE TABLE `bot_guild_members` (
	`bot_id` INT(11) NOT NULL DEFAULT '0',
	`guild_id` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
	`rank` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`tribute_enable` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`total_tribute` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`last_tribute` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`banker` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`public_note` TEXT NULL,
	`alt` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY  (`bot_id`)
) ENGINE=InnoDB;


-- Functions
DELIMITER $$

-- (no code references - see `vw_groups` below)
CREATE FUNCTION `GetMobType` (mob_name VARCHAR(64)) RETURNS CHAR(1)
BEGIN
	DECLARE Result CHAR(1);
	
	SET Result = NULL;
	
	IF ((SELECT COUNT(*) FROM `character_data` WHERE `name` = mob_name) > 0) THEN
		SET Result = 'C';
	ELSEIF ((SELECT COUNT(*) FROM `bot_data` WHERE `name` = mob_name) > 0) THEN
		SET Result = 'B';
	END IF;
	
	RETURN Result;
END$$

-- (one code reference in /common/database.cpp)
CREATE FUNCTION `GetMobTypeById` (mob_id INTEGER UNSIGNED) RETURNS CHAR(1)
BEGIN
	DECLARE Result CHAR(1);
	
	SET Result = NULL;
	
	IF ((select `id` from `character_data` where `id` = mob_id) > 0) THEN
		SET Result = 'C';
	ELSEIF ((select `bot_id` from `bot_data` where `bot_id` = mob_id) > 0) THEN
		SET Result = 'B';
	END IF;
	
	RETURN Result;
END$$

-- (for reference only)
-- CREATE FUNCTION `GetMobTypeByName` (mob_name VARCHAR(64)) RETURNS CHAR(1)
-- BEGIN
--	DECLARE Result CHAR(1);
--	
--	SET Result = NULL;
--	
--	IF (select `id` from `character_data` where `name` = mob_name) > 0 THEN
--		SET Result = 'C';
--	ELSEIF (select `bot_id` from `bot_data` where `name` = mob_name) > 0 THEN
--		SET Result = 'B';
--	END IF;
--	
--	RETURN Result;
-- END $$

DELIMITER ;


-- Views
CREATE VIEW `vw_bot_character_mobs` AS
SELECT
_utf8'C' AS mob_type,
c.`id`,
c.`name`,
c.`class`,
c.`level`,
c.`last_login`,
c.`zone_id`
FROM `character_data` AS c
UNION ALL
SELECT _utf8'B' AS mob_type,
b.`bot_id` AS id,
b.`name`,
b.`class`,
b.`level`,
b.`last_spawn` AS last_login,
b.`zone_id`
FROM `bot_data` AS b;

CREATE VIEW `vw_bot_groups` AS
SELECT
g.`groups_index`,
g.`group_name`,
g.`group_leader_id`,
b.`name` AS group_leader_name,
b.`owner_id`,
c.`name` AS owner_name
FROM `bot_groups` AS g
JOIN `bot_data` AS b ON g.`group_leader_id` = b.`bot_id`
JOIN `character_data` AS c ON b.`owner_id` = c.`id`
ORDER BY b.`owner_id`, g.`group_name`;

CREATE VIEW `vw_groups` AS
SELECT
g.`groupid` AS group_id,
GetMobType(g.`name`) AS mob_type,
g.`name` AS name,
g.`charid` AS mob_id,
IFNULL(c.`level`, b.`level`) AS level
FROM `group_id` AS g
LEFT JOIN `character_data` AS c ON g.`name` = c.`name`
LEFT JOIN `bot_data` AS b ON g.`name` = b.`name`;

CREATE VIEW `vw_guild_members` AS
SELECT
'C' AS mob_type,
cm.`char_id`,
cm.`guild_id`,
cm.`rank`,
cm.`tribute_enable`,
cm.`total_tribute`,
cm.`last_tribute`,
cm.`banker`,
cm.`public_note`,
cm.`alt`
FROM `guild_members` AS cm
UNION ALL
SELECT
'B' AS mob_type,
bm.`bot_id` AS char_id,
bm.`guild_id`,
bm.`rank`,
bm.`tribute_enable`,
bm.`total_tribute`,
bm.`last_tribute`,
bm.`banker`,
bm.`public_note`,
bm.`alt`
FROM `bot_guild_members` AS bm;


-- End of File
