DROP TABLE IF EXISTS `botgroupmembers`;
DROP TABLE IF EXISTS `botgroup`;
DROP TABLE IF EXISTS `botbuffs`;
DROP TABLE IF EXISTS `botpetinventory`;
DROP TABLE IF EXISTS `botpetbuffs`;
DROP TABLE IF EXISTS `botpets`;
DROP TABLE IF EXISTS `botguildmembers`;
DROP TABLE IF EXISTS `botinventory`;
DROP TABLE IF EXISTS `bots`;

CREATE TABLE IF NOT EXISTS `bots` (
  `BotID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BotOwnerCharacterID` int(10) unsigned NOT NULL,
  `BotSpellsID` int(10) unsigned NOT NULL DEFAULT '0',
  `Name` varchar(64) NOT NULL,
  `LastName` varchar(32) DEFAULT NULL,
  `BotLevel` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `Race` smallint(5) NOT NULL DEFAULT '0',
  `Class` tinyint(2) NOT NULL DEFAULT '0',
  `Gender` tinyint(2) NOT NULL DEFAULT '0',
  `Size` float NOT NULL DEFAULT '0',
  `Face` int(10) NOT NULL DEFAULT '1',
  `LuclinHairStyle` int(10) NOT NULL DEFAULT '1',
  `LuclinHairColor` int(10) NOT NULL DEFAULT '1',
  `LuclinEyeColor` int(10) NOT NULL DEFAULT '1',
  `LuclinEyeColor2` int(10) NOT NULL DEFAULT '1',
  `LuclinBeardColor` int(10) NOT NULL DEFAULT '1',
  `LuclinBeard` int(10) NOT NULL DEFAULT '0',
  `DrakkinHeritage` int(10) NOT NULL DEFAULT '0',
  `DrakkinTattoo` int(10) NOT NULL DEFAULT '0',
  `DrakkinDetails` int(10) NOT NULL DEFAULT '0',
  `MR` smallint(5) NOT NULL DEFAULT '0',
  `CR` smallint(5) NOT NULL DEFAULT '0',
  `DR` smallint(5) NOT NULL DEFAULT '0',
  `FR` smallint(5) NOT NULL DEFAULT '0',
  `PR` smallint(5) NOT NULL DEFAULT '0',
  `AC` smallint(5) NOT NULL DEFAULT '0',
  `STR` mediumint(8) NOT NULL DEFAULT '75',
  `STA` mediumint(8) NOT NULL DEFAULT '75',
  `DEX` mediumint(8) NOT NULL DEFAULT '75',
  `AGI` mediumint(8) NOT NULL DEFAULT '75',
  `_INT` mediumint(8) NOT NULL DEFAULT '80',
  `WIS` mediumint(8) NOT NULL DEFAULT '75',
  `CHA` mediumint(8) NOT NULL DEFAULT '75',
  `ATK` mediumint(9) NOT NULL DEFAULT '0',
  `BotCreateDate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `LastSpawnDate` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `TotalPlayTime` int(10) unsigned NOT NULL DEFAULT '0',
  `LastZoneId` smallint(6) NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotID`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS botinventory (
  `BotInventoryID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BotID` int(10) unsigned NOT NULL DEFAULT '0',
  `SlotID` int(11) NOT NULL DEFAULT '0',
  `ItemID` int(10) unsigned NOT NULL DEFAULT '0',
  `charges` tinyint(3) unsigned DEFAULT '0',
  `color` int(10) unsigned NOT NULL DEFAULT '0',
  `augslot1` mediumint(7) unsigned NOT NULL DEFAULT '0',
  `augslot2` mediumint(7) unsigned NOT NULL DEFAULT '0',
  `augslot3` mediumint(7) unsigned NOT NULL DEFAULT '0',
  `augslot4` mediumint(7) unsigned NOT NULL DEFAULT '0',
  `augslot5` mediumint(7) unsigned DEFAULT '0',
  `instnodrop` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotInventoryID`),
  KEY `FK_botinventory_1` (`BotID`),
  CONSTRAINT `FK_botinventory_1` FOREIGN KEY (`BotID`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB;

delete from rule_values where rule_name like 'Bots%' and ruleset_id = 1;

INSERT INTO rule_values VALUES ('1', 'Bots:BotManaRegen', '3.0', 'Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.');
INSERT INTO rule_values VALUES ('1', 'Bots:BotFinishBuffing', 'false', 'Allow for buffs to complete even if the bot caster is out of mana.  Only affects buffing out of combat.');
INSERT INTO rule_values VALUES ('1', 'Bots:CreateBotCount', '150', 'Number of bots that each account can create');
INSERT INTO rule_values VALUES ('1', 'Bots:SpawnBotCount', '71', 'Number of bots a character can have spawned at one time, You + 71 bots is a 12 group raid');
INSERT INTO rule_values VALUES ('1', 'Bots:BotQuest', 'false', 'Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl');
INSERT INTO rule_values VALUES ('1', 'Bots:BotGroupBuffing', 'false', 'Bots will cast single target buffs as group buffs, default is false for single. Does not make single target buffs work for MGB.');
INSERT INTO rule_values VALUES ('1', 'Bots:BotSpellQuest', 'false', 'Anita Thrall\'s (Anita_Thrall.pl) Bot Spell Scriber quests.');

DELIMITER $$

DROP FUNCTION IF EXISTS `GetMobTypeByName` $$
CREATE FUNCTION `GetMobTypeByName` (mobname VARCHAR(64)) RETURNS CHAR(1)
BEGIN
    DECLARE Result CHAR(1);

    SET Result = NULL;

    IF (select id from character_ where name = mobname) > 0 THEN
      SET Result = 'C';
    ELSEIF (select BotID from bots where Name = mobname) > 0 THEN
      SET Result = 'B';
    END IF;

    RETURN Result;
END $$

DELIMITER ;

DELIMITER $$

DROP FUNCTION IF EXISTS `GetMobTypeById` $$
CREATE FUNCTION `GetMobTypeById` (mobid INTEGER UNSIGNED) RETURNS CHAR(1)
BEGIN
    DECLARE Result CHAR(1);

    SET Result = NULL;

    IF (select id from character_ where id = mobid) > 0 THEN
      SET Result = 'C';
    ELSEIF (select BotID from bots where BotID = mobid) > 0 THEN
      SET Result = 'B';
    END IF;

    RETURN Result;
END $$

DELIMITER ;

DROP VIEW IF EXISTS `vwGroups`;
CREATE VIEW `vwGroups` AS
  select g.groupid as groupid,
GetMobTypeByName(g.name) as mobtype,
g.name as name,
g.charid as mobid,
ifnull(c.level, b.BotLevel) as level
from group_id as g
left join character_ as c on g.name = c.name
left join bots as b on g.name = b.Name;

ALTER TABLE `group_id` DROP PRIMARY KEY, ADD PRIMARY KEY  USING BTREE(`groupid`, `charid`, `name`);
ALTER TABLE `guild_members` DROP PRIMARY KEY;
ALTER TABLE `group_id` ADD UNIQUE INDEX `U_group_id_1`(`name`);
ALTER TABLE `group_leaders` ADD UNIQUE INDEX `U_group_leaders_1`(`leadername`);

CREATE TABLE IF NOT EXISTS `botbuffs` (
  `BotBuffId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BotId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpellId` int(10) unsigned NOT NULL DEFAULT '0',
  `CasterLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `DurationFormula` int(10) unsigned NOT NULL DEFAULT '0',
  `TicsRemaining` int(11) unsigned NOT NULL DEFAULT '0',
  `PoisonCounters` int(11) unsigned NOT NULL DEFAULT '0',
  `DiseaseCounters` int(11) unsigned NOT NULL DEFAULT '0',
  `CurseCounters` int(11) unsigned NOT NULL DEFAULT '0',
  `HitCount` int(10) unsigned NOT NULL DEFAULT '0',
  `MeleeRune` int(10) unsigned NOT NULL DEFAULT '0',
  `MagicRune` int(10) unsigned NOT NULL DEFAULT '0',
  `DeathSaveSuccessChance` int(10) unsigned NOT NULL DEFAULT '0',
  `CasterAARank` int(10) unsigned NOT NULL DEFAULT '0',
  `Persistent` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotBuffId`),
  KEY `FK_botbuff_1` (`BotId`),
  CONSTRAINT `FK_botbuff_1` FOREIGN KEY (`BotId`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `botpets` (
  `BotPetsId` integer unsigned NOT NULL AUTO_INCREMENT,
  `PetId` integer unsigned NOT NULL DEFAULT '0',
  `BotId` integer unsigned NOT NULL DEFAULT '0',
  `Name` varchar(64) NULL,
  `Mana` integer NOT NULL DEFAULT '0',
  `HitPoints` integer NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotPetsId`),
  KEY `FK_botpets_1` (`BotId`),
  CONSTRAINT `FK_botpets_1` FOREIGN KEY (`BotId`) REFERENCES `bots` (`BotID`),
  CONSTRAINT `U_botpets_1` UNIQUE (`BotId`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `botpetbuffs` (
  `BotPetBuffId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BotPetsId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpellId` int(10) unsigned NOT NULL DEFAULT '0',
  `CasterLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `Duration` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotPetBuffId`),
  KEY `FK_botpetbuffs_1` (`BotPetsId`),
  CONSTRAINT `FK_botpetbuffs_1` FOREIGN KEY (`BotPetsId`) REFERENCES `botpets` (`BotPetsID`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `botpetinventory` (
  `BotPetInventoryId` integer unsigned NOT NULL AUTO_INCREMENT,
  `BotPetsId` integer unsigned NOT NULL DEFAULT '0',
  `ItemId` integer unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotPetInventoryId`),
  KEY `FK_botpetinventory_1` (`BotPetsId`),
  CONSTRAINT `FK_botpetinventory_1` FOREIGN KEY (`BotPetsId`) REFERENCES `botpets` (`BotPetsID`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `botguildmembers` (
  `char_id` int(11) NOT NULL default '0',
  `guild_id` mediumint(8) unsigned NOT NULL default '0',
  `rank` tinyint(3) unsigned NOT NULL default '0',
  `tribute_enable` tinyint(3) unsigned NOT NULL default '0',
  `total_tribute` int(10) unsigned NOT NULL default '0',
  `last_tribute` int(10) unsigned NOT NULL default '0',
  `banker` tinyint(3) unsigned NOT NULL default '0',
  `public_note` text NULL,
  PRIMARY KEY  (`char_id`)
) ENGINE=InnoDB;

DROP VIEW IF EXISTS `vwGuildMembers`;
CREATE VIEW `vwGuildMembers` AS
  select 'C' as mobtype,
cm.char_id,
cm.guild_id,
cm.rank,
cm.tribute_enable,
cm.total_tribute,
cm.last_tribute,
cm.banker,
cm.public_note,
cm.alt
from guild_members as cm
union all
select 'B' as mobtype,
bm.char_id,
bm.guild_id,
bm.rank,
bm.tribute_enable,
bm.total_tribute,
bm.last_tribute,
bm.banker,
bm.public_note,
0 as alt
from botguildmembers as bm;

DROP VIEW IF EXISTS `vwBotCharacterMobs`;
CREATE VIEW `vwBotCharacterMobs` AS
  select 'C' as mobtype,
c.id,
c.name,
c.class,
c.level,
c.timelaston,
c.zoneid
from character_ as c
union all
select 'B' as mobtype,
b.BotID as id,
b.Name as name,
b.Class as class,
b.BotLevel as level,
0 as timelaston,
0 as zoneid
from bots as b;

CREATE TABLE IF NOT EXISTS `botgroup` (
  `BotGroupId` integer unsigned NOT NULL AUTO_INCREMENT,
  `BotGroupLeaderBotId` integer unsigned NOT NULL DEFAULT '0',
  `BotGroupName` varchar(64) NOT NULL,
  PRIMARY KEY  (`BotGroupId`),
  KEY FK_botgroup_1 (BotGroupLeaderBotId),
  CONSTRAINT FK_botgroup_1 FOREIGN KEY (BotGroupLeaderBotId) REFERENCES bots (BotID)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `botgroupmembers` (
  `BotGroupMemberId` integer unsigned NOT NULL AUTO_INCREMENT,
  `BotGroupId` integer unsigned NOT NULL DEFAULT '0',
  `BotId` integer unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY  (`BotGroupMemberId`),
  KEY FK_botgroupmembers_1 (BotGroupId),
  CONSTRAINT FK_botgroupmembers_1 FOREIGN KEY (BotGroupId) REFERENCES botgroup (BotGroupId),
  KEY FK_botgroupmembers_2 (BotId),
  CONSTRAINT FK_botgroupmembers_2 FOREIGN KEY (BotId) REFERENCES bots (BotID)
) ENGINE=InnoDB;

DROP VIEW IF EXISTS `vwBotGroups`;
CREATE VIEW `vwBotGroups` AS
select g.BotGroupId,
g.BotGroupName,
g.BotGroupLeaderBotId,
b.Name as BotGroupLeaderName,
b.BotOwnerCharacterId,
c.name as BotOwnerCharacterName
from botgroup as g
join bots as b on g.BotGroupLeaderBotId = b.BotID
join character_ as c on b.BotOwnerCharacterID = c.id
order by b.BotOwnerCharacterId, g.BotGroupName;