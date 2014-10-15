-- 'drop_bots' sql script file
-- current as of 10/15/2014
--
-- Note: This file will revert all changes made by either 'load_bots' sql file.
-- There may still be remnants of bot activity in tables `guild_members` and
-- `group_id`. If these entries are causing issues, you may need to manually
-- remove them.


ALTER TABLE `guild_members` ADD PRIMARY KEY (`char_id`);
ALTER TABLE `group_id` DROP PRIMARY KEY, ADD PRIMARY KEY (`groupid`, `charid`, `ismerc`);

UPDATE `spawn2` SET `enabled` = 0 WHERE `id` IN (59297,59298);

DELETE FROM `commands` WHERE `command` = 'bot';
DELETE FROM `rule_values` WHERE `rule_name` LIKE 'Bots%';

DROP VIEW IF EXISTS `vwbotcharactermobs`;
DROP VIEW IF EXISTS `vwbotgroups`;
DROP VIEW IF EXISTS `vwgroups`;
DROP VIEW IF EXISTS `vwguildmembers`;

DROP FUNCTION IF EXISTS `GetMobType`;

DROP TABLE IF EXISTS `botguildmembers`;
DROP TABLE IF EXISTS `botgroupmembers`;
-- this table is not a part of 'load_bots.sql'
DROP TABLE IF EXISTS `botgroups`;
DROP TABLE IF EXISTS `botgroup`;
DROP TABLE IF EXISTS `botpetinventory`;
DROP TABLE IF EXISTS `botpetbuffs`;
DROP TABLE IF EXISTS `botpets`;
DROP TABLE IF EXISTS `botinventory`;
DROP TABLE IF EXISTS `botbuffs`;
DROP TABLE IF EXISTS `bottimers`;
DROP TABLE IF EXISTS `botstances`;
DROP TABLE IF EXISTS `bots`;
