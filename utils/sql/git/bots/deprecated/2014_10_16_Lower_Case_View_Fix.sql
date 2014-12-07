-- A fix for the auto-conversion view renaming issue


DROP VIEW IF EXISTS `vwbotcharactermobs`;
DROP VIEW IF EXISTS `vwbotgroups`;
DROP VIEW IF EXISTS `vwgroups`;
DROP VIEW IF EXISTS `vwguildmembers`;


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
