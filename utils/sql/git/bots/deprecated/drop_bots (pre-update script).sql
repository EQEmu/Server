-- 'drop_bots (pre-update script)' sql script file
-- current as of 11/30/2015
--
-- Note: This file will remove bot schema loaded by 'load_bots' sql scripts.
-- There may still be remnants of bot activity in tables `guild_members` and
-- `group_id`. If these entries are causing issues, you may need to manually
-- remove them.


SELECT "dropping views...";
DROP VIEW IF EXISTS `vwGuildMembers`;
DROP VIEW IF EXISTS `vwGroups`;
DROP VIEW IF EXISTS `vwBotGroups`;
DROP VIEW IF EXISTS `vwBotCharacterMobs`;


SELECT "dropping functions...";
DROP FUNCTION IF EXISTS `GetMobTypeByName`;
DROP FUNCTION IF EXISTS `GetMobTypeByID`;
DROP FUNCTION IF EXISTS `GetMobType`;


SELECT "dropping tables...";
DROP TABLE IF EXISTS `botguildmembers`;
DROP TABLE IF EXISTS `botgroupmembers`;
DROP TABLE IF EXISTS `botgroup`;
DROP TABLE IF EXISTS `botgroups`; -- this table is not a part of 'load_bots.sql'
DROP TABLE IF EXISTS `botpetinventory`;
DROP TABLE IF EXISTS `botpetbuffs`;
DROP TABLE IF EXISTS `botpets`;
DROP TABLE IF EXISTS `botinventory`;
DROP TABLE IF EXISTS `botbuffs`;
DROP TABLE IF EXISTS `bottimers`;
DROP TABLE IF EXISTS `botstances`;
DROP TABLE IF EXISTS `bots`;

DROP PROCEDURE IF EXISTS `DropBotsSchema`;


DELIMITER $$

CREATE PROCEDURE `DropBotsSchema` ()
BEGIN
	SELECT "deleting rules...";
	DELETE FROM `rule_values` WHERE `rule_name` LIKE 'Bots%';
	
	
	SELECT "deleting command...";
	DELETE FROM `commands` WHERE `command` LIKE 'bot';
	
	
	SELECT "restoring keys...";
	IF (EXISTS(SELECT `CONSTRAINT_NAME` FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'group_id' AND `CONSTRAINT_NAME` = 'PRIMARY')) THEN
		ALTER TABLE `group_id` DROP PRIMARY KEY;
	END IF;
	ALTER TABLE `group_id` ADD PRIMARY KEY (`groupid`, `charid`, `ismerc`);
	
	IF (EXISTS(SELECT `CONSTRAINT_NAME` FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'guild_members' AND `CONSTRAINT_NAME` = 'PRIMARY')) THEN
		ALTER TABLE `guild_members` DROP PRIMARY KEY;
	END IF;
	ALTER TABLE `guild_members` ADD PRIMARY KEY (`char_id`);
	
	
	SELECT "de-activating spawns...";
	UPDATE `spawn2` SET `enabled` = 0 WHERE `id` IN (59297,59298);
	
	
	SELECT "clearing database version...";
	IF (EXISTS(SELECT `COLUMN_NAME` FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'db_version' AND `COLUMN_NAME` = 'bots_version')) THEN
		UPDATE `db_version`
		SET `bots_version` = 0;
	END IF;
	
END$$

DELIMITER ;


CALL `DropBotsSchema`();

SELECT "dropping procedure...";
DROP PROCEDURE IF EXISTS `DropBotsSchema`;


-- End of File
