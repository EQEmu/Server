DROP TABLE IF EXISTS `botbuffs`;
DROP TABLE IF EXISTS `botpetinventory`;
DROP TABLE IF EXISTS `botpetbuffs`;
DROP TABLE IF EXISTS `botpets`;
DROP TABLE IF EXISTS `botgroupmembers`;
DROP TABLE IF EXISTS `botgroup`;
DROP TABLE IF EXISTS `botgroups`;
DROP TABLE IF EXISTS `botinventory`;
DROP TABLE IF EXISTS `botguildmembers`;
DROP TABLE IF EXISTS `botstances`;
DROP TABLE IF EXISTS `bottimers`;
DROP TABLE IF EXISTS `bots`;
DROP VIEW IF EXISTS `vwGuildMembers`;
DROP VIEW IF EXISTS `vwBotCharacterMobs`;
DROP VIEW IF EXISTS `vwBotGroups`;

delete from rule_values where rule_name like 'Bots%' and ruleset_id = 1;

delete from commands where command = 'bot';

update spawn2 set enabled = 0 where id in (59297,59298);