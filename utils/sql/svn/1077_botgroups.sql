DROP TABLE IF EXISTS `botgroupmembers`;
DROP TABLE IF EXISTS `botgroup`;

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