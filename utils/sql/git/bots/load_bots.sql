-- 'load_bots' sql script file
-- current as of 10/15/2014
--
-- Use this file on databases where the player profile blob has been converted.
--
-- Note: This file assumes a database free of bot remnants. If you have a prior
-- bot installation and wish to reload the default schema and entries, then
-- source 'drop_bots.sql' before sourcing this file.


ALTER TABLE `guild_members` DROP PRIMARY KEY;
ALTER TABLE `group_id` DROP PRIMARY KEY, ADD PRIMARY KEY USING BTREE(`groupid`, `charid`, `name`, `ismerc`);

UPDATE `spawn2` SET `enabled` = 1 WHERE `id` IN (59297,59298);

-- old command kept for reference (`commands` now only has 2 columns - `command` and `access`)
-- INSERT INTO `commands` VALUES ('bot', '0', 'Type \"#bot help\" to the see the list of available commands for bots.');
INSERT INTO `commands` VALUES ('bot', '0');

INSERT INTO `rule_values` VALUES
	('1', 'Bots:BotAAExpansion', '8', 'The expansion through which bots will obtain AAs'),
	('1', 'Bots:BotFinishBuffing', 'false', 'Allow for buffs to complete even if the bot caster is out of mana.  Only affects buffing out of combat.'),
	('1', 'Bots:BotGroupBuffing', 'false', 'Bots will cast single target buffs as group buffs, default is false for single. Does not make single target buffs work for MGB.'),
	('1', 'Bots:BotManaRegen', '3.0', 'Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.'),
	('1', 'Bots:BotQuest', 'false', 'Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl'),
	('1', 'Bots:BotSpellQuest', 'false', 'Anita Thrall\'s (Anita_Thrall.pl) Bot Spell Scriber quests.'),
	('1', 'Bots:CreateBotCount', '150', 'Number of bots that each account can create'),
	('1', 'Bots:SpawnBotCount', '71', 'Number of bots a character can have spawned at one time, You + 71 bots is a 12 group raid');

CREATE TABLE `bots` (
	`BotID` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`BotOwnerCharacterID` INT(10) UNSIGNED NOT NULL,
	`BotSpellsID` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`Name` VARCHAR(64) NOT NULL,
	`LastName` VARCHAR(32) DEFAULT NULL,
	`BotLevel` TINYINT(2) UNSIGNED NOT NULL DEFAULT '0',
	`Race` SMALLINT(5) NOT NULL DEFAULT '0',
	`Class` TINYINT(2) NOT NULL DEFAULT '0',
	`Gender` TINYINT(2) NOT NULL DEFAULT '0',
	`Size` FLOAT NOT NULL DEFAULT '0',
	`Face` INT(10) NOT NULL DEFAULT '1',
	`LuclinHairStyle` INT(10) NOT NULL DEFAULT '1',
	`LuclinHairColor` INT(10) NOT NULL DEFAULT '1',
	`LuclinEyeColor` INT(10) NOT NULL DEFAULT '1',
	`LuclinEyeColor2` INT(10) NOT NULL DEFAULT '1',
	`LuclinBeardColor` INT(10) NOT NULL DEFAULT '1',
	`LuclinBeard` INT(10) NOT NULL DEFAULT '0',
	`DrakkinHeritage` INT(10) NOT NULL DEFAULT '0',
	`DrakkinTattoo` INT(10) NOT NULL DEFAULT '0',
	`DrakkinDetails` INT(10) NOT NULL DEFAULT '0',
	`HP` INTEGER NOT NULL DEFAULT '0',
	`Mana` INTEGER NOT NULL DEFAULT '0',
	`MR` SMALLINT(5) NOT NULL DEFAULT '0',
	`CR` SMALLINT(5) NOT NULL DEFAULT '0',
	`DR` SMALLINT(5) NOT NULL DEFAULT '0',
	`FR` SMALLINT(5) NOT NULL DEFAULT '0',
	`PR` SMALLINT(5) NOT NULL DEFAULT '0',
	`Corrup` SMALLINT(5) NOT NULL DEFAULT '0',
	`AC` SMALLINT(5) NOT NULL DEFAULT '0',
	`STR` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`STA` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`DEX` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`AGI` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`_INT` MEDIUMINT(8) NOT NULL DEFAULT '80',
	`WIS` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`CHA` MEDIUMINT(8) NOT NULL DEFAULT '75',
	`ATK` MEDIUMINT(9) NOT NULL DEFAULT '0',
	`BotCreateDate` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	`LastSpawnDate` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
	`TotalPlayTime` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`LastZoneId` SMALLINT(6) NOT NULL DEFAULT '0',
	`BotInspectMessage` VARCHAR(256) NOT NULL DEFAULT '',
	PRIMARY KEY (`BotID`)
) ENGINE=InnoDB;

CREATE TABLE `botstances` (
	`BotID` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`StanceID` TINYINT UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`BotID`),
	CONSTRAINT `FK_botstances_1` FOREIGN KEY (`BotID`) REFERENCES `bots` (`BotID`)
);

CREATE TABLE `bottimers` (
	`BotID` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`TimerID` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`Value` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`BotID`),
	CONSTRAINT `FK_bottimers_1` FOREIGN KEY (`BotID`) REFERENCES `bots` (`BotID`)
);

CREATE TABLE `botbuffs` (
	`BotBuffId` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`BotId` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`SpellId` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`CasterLevel` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`DurationFormula` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`TicsRemaining` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`PoisonCounters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`DiseaseCounters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`CurseCounters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`CorruptionCounters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`HitCount` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`MeleeRune` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`MagicRune` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`DeathSaveSuccessChance` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`CasterAARank` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`Persistent` TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY (`BotBuffId`),
	KEY `FK_botbuff_1` (`BotId`),
	CONSTRAINT `FK_botbuff_1` FOREIGN KEY (`BotId`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE `botinventory` (
	`BotInventoryID` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`BotID` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`SlotID` INTEGER SIGNED NOT NULL DEFAULT '0',
	`ItemID` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`charges` TINYINT(3) UNSIGNED DEFAULT 0,
	`color` INTEGER UNSIGNED NOT NULL DEFAULT 0,
	`augslot1` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT 0,
	`augslot2` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT 0,
	`augslot3` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT 0,
	`augslot4` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT 0,
	`augslot5` MEDIUMINT(7) UNSIGNED DEFAULT 0,
	`instnodrop` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`BotInventoryID`),
	KEY `FK_botinventory_1` (`BotID`),
	CONSTRAINT `FK_botinventory_1` FOREIGN KEY (`BotID`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB; 

CREATE TABLE `botpets` (
	`BotPetsId` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`PetId` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`BotId` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`Name` VARCHAR(64) NULL,
	`Mana` INTEGER NOT NULL DEFAULT '0',
	`HitPoints` INTEGER NOT NULL DEFAULT '0',
	PRIMARY KEY (`BotPetsId`),
	KEY `FK_botpets_1` (`BotId`),
	CONSTRAINT `FK_botpets_1` FOREIGN KEY (`BotId`) REFERENCES `bots` (`BotID`),
	CONSTRAINT `U_botpets_1` UNIQUE (`BotId`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE `botpetbuffs` (
	`BotPetBuffId` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`BotPetsId` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`SpellId` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`CasterLevel` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`Duration` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`BotPetBuffId`),
	KEY `FK_botpetbuffs_1` (`BotPetsId`),
	CONSTRAINT `FK_botpetbuffs_1` FOREIGN KEY (`BotPetsId`) REFERENCES `botpets` (`BotPetsID`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE `botpetinventory` (
	`BotPetInventoryId` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`BotPetsId` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`ItemId` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`BotPetInventoryId`),
	KEY `FK_botpetinventory_1` (`BotPetsId`),
	CONSTRAINT `FK_botpetinventory_1` FOREIGN KEY (`BotPetsId`) REFERENCES `botpets` (`BotPetsID`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE `botgroup` (
	`BotGroupId` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`BotGroupLeaderBotId` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`BotGroupName` VARCHAR(64) NOT NULL,
	PRIMARY KEY  (`BotGroupId`),
	KEY `FK_botgroup_1` (`BotGroupLeaderBotId`),
	CONSTRAINT `FK_botgroup_1` FOREIGN KEY (`BotGroupLeaderBotId`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB;

CREATE TABLE `botgroupmembers` (
	`BotGroupMemberId` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	`BotGroupId` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	`BotId` INTEGER UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY  (`BotGroupMemberId`),
	KEY `FK_botgroupmembers_1` (`BotGroupId`),
	CONSTRAINT `FK_botgroupmembers_1` FOREIGN KEY (`BotGroupId`) REFERENCES `botgroup` (`BotGroupId`),
	KEY `FK_botgroupmembers_2` (`BotId`),
	CONSTRAINT `FK_botgroupmembers_2` FOREIGN KEY (`BotId`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB;

CREATE TABLE `botguildmembers` (
	`char_id` INT(11) NOT NULL DEFAULT '0',
	`guild_id` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
	`rank` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`tribute_enable` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`total_tribute` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`last_tribute` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`banker` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`public_note` TEXT NULL,
	`alt` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY  (`char_id`)
) ENGINE=InnoDB;

DELIMITER $$

CREATE FUNCTION `GetMobType` (mobname VARCHAR(64)) RETURNS CHAR(1)
BEGIN
	DECLARE Result CHAR(1);
	
	SET Result = NULL;
	
	IF (SELECT COUNT(*) FROM `character_data` WHERE `name` = mobname) > 0 THEN
		SET Result = 'C';
	ELSEIF (SELECT COUNT(*) FROM `bots` WHERE `Name` = mobname) > 0 THEN
		SET Result = 'B';
	END IF;
	
	RETURN Result;
END$$

DELIMITER ;

CREATE VIEW `vwBotCharacterMobs` AS
SELECT _utf8'C' AS mobtype,
c.`id`,
c.`name`,
c.`class`,
c.`level`,
c.`last_login`,
c.`zone_id`
FROM `character_data` AS c
UNION ALL
SELECT _utf8'B' AS mobtype,
b.`BotID` AS id,
b.`Name` AS name,
b.`Class` AS class,
b.`BotLevel` AS level,
0 AS timelaston,
0 AS zoneid
FROM bots AS b;

CREATE VIEW `vwGroups` AS
SELECT g.`groupid` AS groupid,
GetMobType(g.`name`) AS mobtype,
g.`name` AS name,
g.`charid` AS mobid,
IFNULL(c.`level`, b.`BotLevel`) AS level
FROM `group_id` AS g
LEFT JOIN `character_data` AS c ON g.`name` = c.`name`
LEFT JOIN `bots` AS b ON g.`name` = b.`Name`;

CREATE VIEW `vwBotGroups` AS
SELECT g.`BotGroupId`,
g.`BotGroupName`,
g.`BotGroupLeaderBotId`,
b.`Name` AS BotGroupLeaderName,
b.`BotOwnerCharacterId`,
c.`name` AS BotOwnerCharacterName
FROM `botgroup` AS g
JOIN `bots` AS b ON g.`BotGroupLeaderBotId` = b.`BotID`
JOIN `character_data` AS c ON b.`BotOwnerCharacterID` = c.`id`
ORDER BY b.`BotOwnerCharacterId`, g.`BotGroupName`;

CREATE VIEW `vwGuildMembers` AS
SELECT 'C' AS mobtype,
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
SELECT 'B' AS mobtype,
bm.`char_id`,
bm.`guild_id`,
bm.`rank`,
bm.`tribute_enable`,
bm.`total_tribute`,
bm.`last_tribute`,
bm.`banker`,
bm.`public_note`,
bm.`alt`
FROM `botguildmembers` AS bm;
