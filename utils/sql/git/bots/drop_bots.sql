-- 'drop_bots' sql script file
-- current as of 12/11/2015
--
-- Note: This file will remove bot schema loaded by 'load_bots' sql scripts.
-- There may still be remnants of bot activity in tables `guild_members` and
-- `group_id`. If these entries are causing issues, you may need to manually
-- remove them.
--
-- Use eqemu_update.pl to administer this script


DROP VIEW IF EXISTS `vwguildmembers`;
DROP VIEW IF EXISTS `vwgroups`;
DROP VIEW IF EXISTS `vwbotgroups`;
DROP VIEW IF EXISTS `vwbotcharactermobs`;

DROP VIEW IF EXISTS `vwGuildMembers`;
DROP VIEW IF EXISTS `vwGroups`;
DROP VIEW IF EXISTS `vwBotGroups`;
DROP VIEW IF EXISTS `vwBotCharacterMobs`;

DROP VIEW IF EXISTS `vw_guild_members`;
DROP VIEW IF EXISTS `vw_groups`;
DROP VIEW IF EXISTS `vw_bot_groups`;
DROP VIEW IF EXISTS `vw_bot_character_mobs`;

DROP FUNCTION IF EXISTS `GetMobTypeByName`;
DROP FUNCTION IF EXISTS `GetMobTypeByID`;
DROP FUNCTION IF EXISTS `GetMobType`;

DROP TABLE IF EXISTS `botguildmembers`;
DROP TABLE IF EXISTS `botgroupmembers`;
DROP TABLE IF EXISTS `botgroup`;
DROP TABLE IF EXISTS `botpetinventory`;
DROP TABLE IF EXISTS `botpetbuffs`;
DROP TABLE IF EXISTS `botpets`;
DROP TABLE IF EXISTS `botinventory`;
DROP TABLE IF EXISTS `botbuffs`;
DROP TABLE IF EXISTS `bottimers`;
DROP TABLE IF EXISTS `botstances`;
DROP TABLE IF EXISTS `bots`;

DROP TABLE IF EXISTS `botgroups`; -- this table is not a part of 'load_bots.sql'

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

DROP PROCEDURE IF EXISTS `LoadBotsSchema`;
DROP PROCEDURE IF EXISTS `DropBotsSchema`;


-- End of File
