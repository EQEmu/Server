-- 'drop_bots' sql script file
-- current as of 10/09/2015
--
-- Note: This file will remove bot schema loaded by 'load_bots' sql scripts.
-- There may still be remnants of bot activity in tables `guild_members` and
-- `group_id`. If these entries are causing issues, you may need to manually
-- remove them.


SELECT "dropping views...";
DROP VIEW IF EXISTS `vw_guild_members`;
DROP VIEW IF EXISTS `vw_groups`;
DROP VIEW IF EXISTS `vw_bot_groups`;
DROP VIEW IF EXISTS `vw_bot_character_mobs`;


SELECT "dropping functions...";
DROP FUNCTION IF EXISTS `GetMobTypeByName`;
DROP FUNCTION IF EXISTS `GetMobTypeByID`;
DROP FUNCTION IF EXISTS `GetMobType`;


SELECT "dropping tables...";
DROP TABLE IF EXISTS `botguildmembers_old`;
DROP TABLE IF EXISTS `botgroupmembers_old`;
DROP TABLE IF EXISTS `botgroup_old`;
DROP TABLE IF EXISTS `botpetinventory_old`;
DROP TABLE IF EXISTS `botpetbuffs_old`;
DROP TABLE IF EXISTS `botpets_old`;
DROP TABLE IF EXISTS `botinventory_old`;
DROP TABLE IF EXISTS `botbuffs_old`;
DROP TABLE IF EXISTS `bottimers_old`;
DROP TABLE IF EXISTS `botstances_old`;
DROP TABLE IF EXISTS `bots_old`;

DROP TABLE IF EXISTS `bot_guild_members`;
DROP TABLE IF EXISTS `bot_group_members`;
DROP TABLE IF EXISTS `bot_groups`;
DROP TABLE IF EXISTS `bot_pet_inventories`;
DROP TABLE IF EXISTS `bot_pet_buffs`;
DROP TABLE IF EXISTS `bot_pets`;
DROP TABLE IF EXISTS `bot_inventories`;
DROP TABLE IF EXISTS `bot_buffs`;
DROP TABLE IF EXISTS `bot_timers`;
DROP TABLE IF EXISTS `bot_stances`;
DROP TABLE IF EXISTS `bot_inspect_messages`;
DROP TABLE IF EXISTS `bot_data`;

DROP PROCEDURE IF EXISTS `DropBotsSchema`;


DELIMITER $$

CREATE PROCEDURE `DropBotsSchema` ()
BEGIN
	SELECT "deleting rules...";
	DELETE FROM `rule_values` WHERE `rule_name` LIKE 'Bots%';
	
	
	SELECT "deleting command...";
	DELETE FROM `commands` WHERE `command` LIKE 'bot';
	
	
	SELECT "restoring keys...";
	IF ((SELECT COUNT(*) FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'group_id' AND `CONSTRAINT_NAME` = 'PRIMARY') > 0) THEN
		ALTER TABLE `group_id` DROP PRIMARY KEY;
	END IF;
	ALTER TABLE `group_id` ADD PRIMARY KEY (`groupid`, `charid`, `ismerc`);
	
	IF ((SELECT COUNT(*) FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'guild_members' AND `CONSTRAINT_NAME` = 'PRIMARY') > 0) THEN
		ALTER TABLE `guild_members` DROP PRIMARY KEY;
	END IF;
	ALTER TABLE `guild_members` ADD PRIMARY KEY (`char_id`);
	
	
	SELECT "de-activating spawns...";
	UPDATE `spawn2` SET `enabled` = 0 WHERE `id` IN (59297,59298);
	
	
	SELECT "clearing database version...";
	IF ((SELECT COUNT(*) FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'db_version' AND `COLUMN_NAME` = 'bots_version') > 0) THEN
		UPDATE `db_version`
		SET `bots_version` = 0;
	END IF;
	
END$$

DELIMITER ;


CALL `DropBotsSchema`();

SELECT "dropping procedure...";
DROP PROCEDURE IF EXISTS `DropBotsSchema`;


-- End of File
