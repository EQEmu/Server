DROP VIEW IF EXISTS `vwGroups`;
DROP FUNCTION IF EXISTS `GetMobType`;

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

ALTER TABLE `bots` ADD COLUMN `LastZoneId` SMALLINT(6) NOT NULL DEFAULT 0 AFTER `TotalPlayTime`;

DROP TABLE IF EXISTS `botguildmembers`;
CREATE TABLE `botguildmembers` (
  `char_id` int(11) NOT NULL default '0',
  `guild_id` mediumint(8) unsigned NOT NULL default '0',
  `rank` tinyint(3) unsigned NOT NULL default '0',
  `tribute_enable` tinyint(3) unsigned NOT NULL default '0',
  `total_tribute` int(10) unsigned NOT NULL default '0',
  `last_tribute` int(10) unsigned NOT NULL default '0',
  `banker` tinyint(3) unsigned NOT NULL default '0',
  `public_note` text NULL,
  PRIMARY KEY  (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
cm.public_note
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
bm.public_note
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
unix_timestamp(b.LastSpawnDate) as timelaston,
b.LastZoneId as zoneid
from bots as b;