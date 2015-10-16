-- '2015_09_30_bots' sql script file
-- current as of 10/13/2015
--
-- Use eqemu_update.pl to administer this script


-- Clean-up
DROP VIEW IF EXISTS `vwBotCharacterMobs`;
DROP VIEW IF EXISTS `vwBotGroups`;
DROP VIEW IF EXISTS `vwGroups`;
DROP VIEW IF EXISTS `vwGuildMembers`;

DROP VIEW IF EXISTS `vw_bot_character_mobs`;
DROP VIEW IF EXISTS `vw_bot_groups`;
DROP VIEW IF EXISTS `vw_groups`;
DROP VIEW IF EXISTS `vw_guild_members`;

DROP FUNCTION IF EXISTS `GetMobType`;
DROP FUNCTION IF EXISTS `GetMobTypeByName`;
DROP FUNCTION IF EXISTS `GetMobTypeByID`;

DROP PROCEDURE IF EXISTS `LoadBotsSchema`;


DELIMITER $$

CREATE PROCEDURE `LoadBotsSchema` ()
BEGIN
	-- Activate
	UPDATE `spawn2` SET `enabled` = 1 WHERE `id` IN (59297,59298);
	
	
	-- Alter
	IF ((SELECT COUNT(*) FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'guild_members' AND `CONSTRAINT_NAME` = 'PRIMARY') > 0) THEN
		ALTER TABLE `guild_members` DROP PRIMARY KEY;
	END IF;
	
	IF ((SELECT COUNT(*) FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'group_id' AND `CONSTRAINT_NAME` = 'PRIMARY') > 0) THEN
		ALTER TABLE `group_id` DROP PRIMARY KEY;
	END IF;
	ALTER TABLE `group_id` ADD PRIMARY KEY USING BTREE(`groupid`, `charid`, `name`, `ismerc`);
	--
	-- From original bots.sql (for reference)
	-- ALTER TABLE `group_id` ADD UNIQUE INDEX `U_group_id_1`(`name`);
	-- ALTER TABLE `group_leaders` ADD UNIQUE INDEX `U_group_leaders_1`(`leadername`);
	
	
	-- Commands
	IF ((SELECT COUNT(`command`) FROM `commands` WHERE `command` LIKE 'bot') = 0) THEN
		INSERT INTO `commands` VALUES ('bot', '0');
	END IF;
	
	
	-- Rules
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotAAExpansion') > 0) THEN
		UPDATE `rule_values` SET `rule_name` = 'Bots:AAExpansion' WHERE `rule_name` LIKE 'Bots:BotAAExpansion';
	END IF;
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:AAExpansion') = 0) THEN
		INSERT INTO `rule_values` VALUES ('1', 'Bots:AAExpansion', '8', 'The expansion through which bots will obtain AAs');
	END IF;
	
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:CreateBotCount') > 0) THEN
		UPDATE `rule_values` SET `rule_name` = 'Bots:CreationLimit' WHERE `rule_name` LIKE 'Bots:CreateBotCount';
	END IF;
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:CreationLimit') = 0) THEN
		INSERT INTO `rule_values` VALUES ('1', 'Bots:CreationLimit', '150', 'Number of bots that each account can create');
	END IF;

	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotFinishBuffing') > 0) THEN
		UPDATE `rule_values` SET `rule_name` = 'Bots:FinishBuffing' WHERE `rule_name` LIKE 'Bots:BotFinishBuffing';
	END IF;
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:FinishBuffing') = 0) THEN
		INSERT INTO `rule_values` VALUES ('1', 'Bots:FinishBuffing', 'false', 'Allow for buffs to complete even if the bot caster is out of mana.  Only affects buffing out of combat.');
	END IF;

	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotGroupBuffing') > 0) THEN
		UPDATE `rule_values` SET `rule_name` = 'Bots:GroupBuffing' WHERE `rule_name` LIKE 'Bots:BotGroupBuffing';
	END IF;
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:GroupBuffing') = 0) THEN
		INSERT INTO `rule_values` VALUES ('1', 'Bots:GroupBuffing', 'false', 'Bots will cast single target buffs as group buffs, default is false for single. Does not make single target buffs work for MGB.');
	END IF;
	
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotManaRegen') > 0) THEN
		UPDATE `rule_values` SET `rule_name` = 'Bots:ManaRegen' WHERE `rule_name` LIKE 'Bots:BotManaRegen';
	END IF;
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:ManaRegen') = 0) THEN
		INSERT INTO `rule_values` VALUES ('1', 'Bots:ManaRegen', '3.0', 'Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.');
	END IF;
	
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotQuest') > 0) THEN
		UPDATE `rule_values` SET `rule_name` = 'Bots:QuestableSpawnLimit' WHERE `rule_name` LIKE 'Bots:BotQuest';
	END IF;
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:QuestableSpawnLimit') = 0) THEN
		INSERT INTO `rule_values` VALUES ('1', 'Bots:QuestableSpawnLimit', 'false', 'Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl');
	END IF;
	
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:BotSpellQuest') > 0) THEN
		UPDATE `rule_values` SET `rule_name` = 'Bots:QuestableSpells' WHERE `rule_name` LIKE 'Bots:BotSpellQuest';
	END IF;
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:QuestableSpells') = 0) THEN
		INSERT INTO `rule_values` VALUES ('1', 'Bots:QuestableSpells', 'false', 'Anita Thrall\'s (Anita_Thrall.pl) Bot Spell Scriber quests.');
	END IF;
	
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:SpawnBotCount') > 0) THEN
		UPDATE `rule_values` SET `rule_name` = 'Bots:SpawnLimit' WHERE `rule_name` LIKE 'Bots:SpawnBotCount';
	END IF;
	IF ((SELECT COUNT(`rule_name`) FROM `rule_values` WHERE `rule_name` LIKE 'Bots:SpawnLimit') = 0) THEN
		INSERT INTO `rule_values` VALUES ('1', 'Bots:SpawnLimit', '71', 'Number of bots a character can have spawned at one time, You + 71 bots is a 12 group raid');
	END IF;
	
	
	-- Tables
	CREATE TABLE `bot_data` (
		`bot_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
		`owner_id` INT(11) UNSIGNED NOT NULL,
		`spells_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`name` VARCHAR(64) NOT NULL DEFAULT '',
		`last_name` VARCHAR(64) NOT NULL DEFAULT '',			-- Change unused (64) from (32)
		`title` VARCHAR(32) NOT NULL DEFAULT '',				-- Unused
		`suffix` VARCHAR(32) NOT NULL DEFAULT '',				-- Unused
		`zone_id` SMALLINT(6) NOT NULL DEFAULT '0',
		`gender` TINYINT(2) NOT NULL DEFAULT '0',
		`race` SMALLINT(5) NOT NULL DEFAULT '0',
		`class` TINYINT(2) NOT NULL DEFAULT '0',
		`level` TINYINT(2) UNSIGNED NOT NULL DEFAULT '0',
		`deity` INT(11) UNSIGNED NOT NULL DEFAULT '0',			-- Unused
		`creation_day` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`last_spawn` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`time_spawned` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`size` FLOAT NOT NULL DEFAULT '0',
		`face` INT(10) NOT NULL DEFAULT '1',
		`hair_color` INT(10) NOT NULL DEFAULT '1',
		`hair_style` INT(10) NOT NULL DEFAULT '1',
		`beard` INT(10) NOT NULL DEFAULT '0',
		`beard_color` INT(10) NOT NULL DEFAULT '1',
		`eye_color_1` INT(10) NOT NULL DEFAULT '1',
		`eye_color_2` INT(10) NOT NULL DEFAULT '1',
		`drakkin_heritage` INT(10) NOT NULL DEFAULT '0',
		`drakkin_tattoo` INT(10) NOT NULL DEFAULT '0',
		`drakkin_details` INT(10) NOT NULL DEFAULT '0',
		`ac` SMALLINT(5) NOT NULL DEFAULT '0',
		`atk` MEDIUMINT(9) NOT NULL DEFAULT '0',
		`hp` INTEGER NOT NULL DEFAULT '0',
		`mana` INTEGER NOT NULL DEFAULT '0',
		`str` MEDIUMINT(8) NOT NULL DEFAULT '75',
		`sta` MEDIUMINT(8) NOT NULL DEFAULT '75',
		`cha` MEDIUMINT(8) NOT NULL DEFAULT '75',
		`dex` MEDIUMINT(8) NOT NULL DEFAULT '75',
		`int` MEDIUMINT(8) NOT NULL DEFAULT '75',
		`agi` MEDIUMINT(8) NOT NULL DEFAULT '75',
		`wis` MEDIUMINT(8) NOT NULL DEFAULT '75',
		`fire` SMALLINT(5) NOT NULL DEFAULT '0',
		`cold` SMALLINT(5) NOT NULL DEFAULT '0',
		`magic` SMALLINT(5) NOT NULL DEFAULT '0',
		`poison` SMALLINT(5) NOT NULL DEFAULT '0',
		`disease` SMALLINT(5) NOT NULL DEFAULT '0',
		`corruption` SMALLINT(5) NOT NULL DEFAULT '0',
		`show_helm` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`follow_distance` INT(11) UNSIGNED NOT NULL DEFAULT '200',
		PRIMARY KEY (`bot_id`)
	) ENGINE=InnoDB;
	CREATE TABLE `bot_inspect_messages` (
		`bot_id` INT(11) UNSIGNED NOT NULL,
		`inspect_message` VARCHAR(256) NOT NULL DEFAULT '',
		PRIMARY KEY (`bot_id`),
		INDEX `bot_id` (`bot_id`)
	) ENGINE=InnoDB;
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'bots') > 0) THEN
		INSERT INTO `bot_data` (
			`bot_id`,
			`owner_id`,
			`spells_id`,
			`name`,
			`last_name`,
			`zone_id`,
			`gender`,
			`race`,
			`class`,
			`level`,
			`creation_day`,
			`last_spawn`,
			`time_spawned`,
			`size`,
			`face`,
			`hair_color`,
			`hair_style`,
			`beard`,
			`beard_color`,
			`eye_color_1`,
			`eye_color_2`,
			`drakkin_heritage`,
			`drakkin_tattoo`,
			`drakkin_details`,
			`ac`,
			`atk`,
			`hp`,
			`mana`,
			`str`,
			`sta`,
			`cha`,
			`dex`,
			`int`,
			`agi`,
			`wis`,
			`fire`,
			`cold`,
			`magic`,
			`poison`,
			`disease`,
			`corruption`
		)
		SELECT
			`BotID`,
			`BotOwnerCharacterID`,
			`BotSpellsID`,
			`Name`,
			`LastName`,
			`LastZoneId`,
			`Gender`,
			`Race`,
			`Class`,
			`BotLevel`,
			UNIX_TIMESTAMP(`BotCreateDate`),
			UNIX_TIMESTAMP(`LastSpawnDate`),
			`TotalPlayTime`,
			`Size`,
			`Face`,
			`LuclinHairColor`,
			`LuclinHairStyle`,
			`LuclinBeard`,
			`LuclinBeardColor`,
			`LuclinEyeColor`,
			`LuclinEyeColor2`,
			`DrakkinHeritage`,
			`DrakkinTattoo`,
			`DrakkinDetails`,
			`AC`,
			`ATK`,
			`HP`,
			`Mana`,
			`STR`,
			`STA`,
			`CHA`,
			`DEX`,
			`_INT`,
			`AGI`,
			`WIS`,
			`FR`,
			`CR`,
			`MR`,
			`PR`,
			`DR`,
			`Corrup`
		FROM `bots`;
		
		INSERT INTO `bot_inspect_messages` (
			`bot_id`,
			`inspect_message`
		)
		SELECT
			`BotID`,
			`BotInspectMessage`
		FROM `bots`;
		
		RENAME TABLE `bots` TO `bots_old`;
	END IF;
	
	CREATE TABLE `bot_stances` (
		`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`stance_id` TINYINT UNSIGNED NOT NULL DEFAULT '0',
		PRIMARY KEY (`bot_id`),
		CONSTRAINT `FK_bot_stances_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
	);
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botstances') > 0) THEN
		INSERT INTO `bot_stances` (
			`bot_id`,
			`stance_id`
		)
		SELECT
			bs.`BotID`,
			bs.`StanceID`
		FROM `botstances` bs
		INNER JOIN `bot_data` bd
		ON bs.`BotID` = bd.`bot_id`;
		
		RENAME TABLE `botstances` TO `botstances_old`;
	END IF;
	
	CREATE TABLE `bot_timers` (
		`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`timer_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`timer_value` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		PRIMARY KEY (`bot_id`),
		CONSTRAINT `FK_bot_timers_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
	);
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'bottimers') > 0) THEN
		INSERT INTO `bot_timers` (
			`bot_id`,
			`timer_id`,
			`timer_value`
		)
		SELECT
			bt.`BotID`,
			bt.`TimerID`,
			bt.`Value`
		FROM `bottimers` bt
		INNER JOIN `bot_data` bd
		ON bt.`BotID` = bd.`bot_id`;
		
		RENAME TABLE `bottimers` TO `bottimers_old`;
	END IF;
	
	CREATE TABLE `bot_buffs` (
		`buffs_index` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
		`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`spell_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`caster_level` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
		`duration_formula` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`tics_remaining` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`poison_counters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`disease_counters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`curse_counters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`corruption_counters` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`numhits` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`melee_rune` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`magic_rune` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`dot_rune` INT(10) UNSIGNED NOT NULL DEFAULT '0',			-- Fix
		`persistent` TINYINT(1) NOT NULL DEFAULT '0',
		`caston_x` INT(10) NOT NULL DEFAULT '0',					-- Fix
		`caston_y` INT(10) NOT NULL DEFAULT '0',					-- Fix
		`caston_z` INT(10) NOT NULL DEFAULT '0',					-- Fix
		`extra_di_chance` INT(10) UNSIGNED NOT NULL DEFAULT '0',	-- Fix
		`instrument_mod` INT(10) NOT NULL DEFAULT '10',				-- Unused
		PRIMARY KEY (`buffs_index`),
		KEY `FK_bot_buffs_1` (`bot_id`),
		CONSTRAINT `FK_bot_buffs_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
	) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botbuffs') > 0) THEN
		INSERT INTO `bot_buffs` (
			`buffs_index`,
			`bot_id`,
			`spell_id`,
			`caster_level`,
			`duration_formula`,
			`tics_remaining`,
			`poison_counters`,
			`disease_counters`,
			`curse_counters`,
			`corruption_counters`,
			`numhits`,
			`melee_rune`,
			`magic_rune`,
			`persistent`
		)
		SELECT
			bb.`BotBuffId`,
			bb.`BotId`,
			bb.`SpellId`,
			bb.`CasterLevel`,
			bb.`DurationFormula`,
			bb.`TicsRemaining`,
			bb.`PoisonCounters`,
			bb.`DiseaseCounters`,
			bb.`CurseCounters`,
			bb.`CorruptionCounters`,
			bb.`HitCount`,
			bb.`MeleeRune`,
			bb.`MagicRune`,
			bb.`Persistent`
		FROM `botbuffs` bb
		INNER JOIN `bot_data` bd
		ON bb.`BotId` = bd.`bot_id`;
		
		IF ((SELECT COUNT(*) FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botbuffs' AND `COLUMN_NAME` = 'dot_rune') > 0) THEN
			UPDATE `bot_buffs` bb
			INNER JOIN `botbuffs` bbo
			ON bb.`buffs_index` = bbo.`BotBuffId`
			SET bb.`dot_rune` = bbo.`dot_rune`
			WHERE bb.`bot_id` = bbo.`BotID`;
		END IF;
		
		IF ((SELECT COUNT(*) FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botbuffs' AND `COLUMN_NAME` = 'caston_x') > 0) THEN
			UPDATE `bot_buffs` bb
			INNER JOIN `botbuffs` bbo
			ON bb.`buffs_index` = bbo.`BotBuffId`
			SET bb.`caston_x` = bbo.`caston_x`
			WHERE bb.`bot_id` = bbo.`BotID`;
		END IF;
		
		IF ((SELECT COUNT(*) FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botbuffs' AND `COLUMN_NAME` = 'caston_y') > 0) THEN
			UPDATE `bot_buffs` bb
			INNER JOIN `botbuffs` bbo
			ON bb.`buffs_index` = bbo.`BotBuffId`
			SET bb.`caston_y` = bbo.`caston_y`
			WHERE bb.`bot_id` = bbo.`BotID`;
		END IF;
		
		IF ((SELECT COUNT(*) FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botbuffs' AND `COLUMN_NAME` = 'caston_z') > 0) THEN
			UPDATE `bot_buffs` bb
			INNER JOIN `botbuffs` bbo
			ON bb.`buffs_index` = bbo.`BotBuffId`
			SET bb.`caston_z` = bbo.`caston_z`
			WHERE bb.`bot_id` = bbo.`BotID`;
		END IF;
		
		IF ((SELECT COUNT(*) FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botbuffs' AND `COLUMN_NAME` = 'ExtraDIChance') > 0) THEN
			UPDATE `bot_buffs` bb
			INNER JOIN `botbuffs` bbo
			ON bb.`buffs_index` = bbo.`BotBuffId`
			SET bb.`extra_di_chance` = bbo.`ExtraDIChance`
			WHERE bb.`bot_id` = bbo.`BotID`;
		END IF;
		
		RENAME TABLE `botbuffs` TO `botbuffs_old`;
	END IF;
	
	CREATE TABLE `bot_inventories` (
		`inventories_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
		`bot_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`slot_id` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
		`item_id` INT(11) UNSIGNED NULL DEFAULT '0',
		`inst_charges` TINYINT(3) UNSIGNED DEFAULT 0,
		`inst_color` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`inst_no_drop` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
		`inst_custom_data` TEXT NULL,
		`ornament_icon` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`ornament_id_file` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		`ornament_hero_model` INT(11) NOT NULL DEFAULT '0',
		`augment_1` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
		`augment_2` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
		`augment_3` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
		`augment_4` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
		`augment_5` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
		`augment_6` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
		PRIMARY KEY (`inventories_index`),
		KEY `FK_bot_inventories_1` (`bot_id`),
		CONSTRAINT `FK_bot_inventories_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
	) ENGINE=InnoDB; 
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botinventory') > 0) THEN
		INSERT INTO `bot_inventories` (
			`inventories_index`,
			`bot_id`,
			`slot_id`,
			`item_id`,
			`inst_charges`,
			`inst_color`,
			`inst_no_drop`,
			`augment_1`,
			`augment_2`,
			`augment_3`,
			`augment_4`,
			`augment_5`
		)
		SELECT
			bi.`BotInventoryID`,
			bi.`BotID`,
			bi.`SlotID`,
			bi.`ItemID`,
			bi.`charges`,
			bi.`color`,
			bi.`instnodrop`,
			bi.`augslot1`,
			bi.`augslot2`,
			bi.`augslot3`,
			bi.`augslot4`,
			bi.`augslot5`
		FROM `botinventory` bi
		INNER JOIN `bot_data` bd
		ON bi.`BotID` = bd.`bot_id`;
		
		IF ((SELECT COUNT(*) FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botinventory' AND `COLUMN_NAME` = 'augslot6') > 0) THEN
			UPDATE `bot_inventories` bi
			INNER JOIN `botinventory` bio
			ON bi.`inventories_index` = bio.`BotInventoryID`
			SET bi.`augment_6` = bio.`augslot6`
			WHERE bi.`bot_id` = bio.`BotID`;
		END IF;
		
		RENAME TABLE `botinventory` TO `botinventory_old`;
	END IF;
	
	CREATE TABLE `bot_pets` (
		`pets_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
		`pet_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
		`bot_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
		`name` VARCHAR(64) NULL,
		`mana` INTEGER NOT NULL DEFAULT '0',
		`hp` INTEGER NOT NULL DEFAULT '0',
		PRIMARY KEY (`pets_index`),
		KEY `FK_bot_pets_1` (`bot_id`),
		CONSTRAINT `FK_bot_pets_1` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`),
		CONSTRAINT `U_bot_pets_1` UNIQUE (`bot_id`)
	) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botpets') > 0) THEN
		INSERT INTO `bot_pets` (
			`pets_index`,
			`pet_id`,
			`bot_id`,
			`name`,
			`mana`,
			`hp`
		)
		SELECT
			bp.`BotPetsId`,
			bp.`PetId`,
			bp.`BotId`,
			bp.`Name`,
			bp.`Mana`,
			bp.`HitPoints`
		FROM `botpets` bp
		INNER JOIN `bot_data` bd
		ON bp.`BotId` = bd.`bot_id`;
		
		RENAME TABLE `botpets` TO `botpets_old`;
	END IF;
	
	CREATE TABLE `bot_pet_buffs` (
		`pet_buffs_index` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
		`pets_index` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`spell_id` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`caster_level` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`duration` INT(11) UNSIGNED NOT NULL DEFAULT '0',
		PRIMARY KEY (`pet_buffs_index`),
		KEY `FK_bot_pet_buffs_1` (`pets_index`),
		CONSTRAINT `FK_bot_pet_buffs_1` FOREIGN KEY (`pets_index`) REFERENCES `bot_pets` (`pets_index`)
	) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botpetbuffs') > 0) THEN
		INSERT INTO `bot_pet_buffs` (
			`pet_buffs_index`,
			`pets_index`,
			`spell_id`,
			`caster_level`,
			`duration`
		)
		SELECT
			bpb.`BotPetBuffId`,
			bpb.`BotPetsId`,
			bpb.`SpellId`,
			bpb.`CasterLevel`,
			bpb.`Duration`
		FROM `botpetbuffs` bpb
		INNER JOIN `bot_pets` bp
		ON bpb.`BotPetsId` = bp.`pets_index`;
		
		RENAME TABLE `botpetbuffs` TO `botpetbuffs_old`;
	END IF;
	
	CREATE TABLE `bot_pet_inventories` (
		`pet_inventories_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
		`pets_index` INTEGER UNSIGNED NOT NULL DEFAULT '0',
		`item_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
		PRIMARY KEY (`pet_inventories_index`),
		KEY `FK_bot_pet_inventories_1` (`pets_index`),
		CONSTRAINT `FK_bot_pet_inventories_1` FOREIGN KEY (`pets_index`) REFERENCES `bot_pets` (`pets_index`)
	) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botpetinventory') > 0) THEN
		INSERT INTO `bot_pet_inventories` (
			`pet_inventories_index`,
			`pets_index`,
			`item_id`
		)
		SELECT
			bpi.`BotPetInventoryId`,
			bpi.`BotPetsId`,
			bpi.`ItemId`
		FROM `botpetinventory` bpi
		INNER JOIN `bot_pets` bp
		ON bpi.`BotPetsId` = bp.`pets_index`;
		
		RENAME TABLE `botpetinventory` TO `botpetinventory_old`;
	END IF;
	
	CREATE TABLE `bot_groups` (
		`groups_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
		`group_leader_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
		`group_name` VARCHAR(64) NOT NULL,
		PRIMARY KEY  (`groups_index`),
		KEY `FK_bot_groups_1` (`group_leader_id`),
		CONSTRAINT `FK_bot_groups_1` FOREIGN KEY (`group_leader_id`) REFERENCES `bot_data` (`bot_id`)
	) ENGINE=InnoDB;
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botgroup') > 0) THEN
		INSERT INTO `bot_groups` (
			`groups_index`,
			`group_leader_id`,
			`group_name`
		)
		SELECT
			bg.`BotGroupId`,
			bg.`BotGroupLeaderBotId`,
			bg.`BotGroupName`
		FROM `botgroup` bg
		INNER JOIN `bot_data` bd
		ON bg.`BotGroupLeaderBotId` = bd.`bot_id`;
		
		RENAME TABLE `botgroup` TO `botgroup_old`;
	END IF;
	
	CREATE TABLE `bot_group_members` (
		`group_members_index` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
		`groups_index` INTEGER UNSIGNED NOT NULL DEFAULT '0',
		`bot_id` INTEGER UNSIGNED NOT NULL DEFAULT '0',
		PRIMARY KEY  (`group_members_index`),
		KEY `FK_bot_group_members_1` (`groups_index`),
		CONSTRAINT `FK_bot_group_members_1` FOREIGN KEY (`groups_index`) REFERENCES `bot_groups` (`groups_index`),
		KEY `FK_bot_group_members_2` (`bot_id`),
		CONSTRAINT `FK_bot_group_members_2` FOREIGN KEY (`bot_id`) REFERENCES `bot_data` (`bot_id`)
	) ENGINE=InnoDB;
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botgroupmembers') > 0) THEN
		INSERT INTO `bot_group_members` (
			`group_members_index`,
			`groups_index`,
			`bot_id`
		)
		SELECT
			bgm.`BotGroupMemberId`,
			bgm.`BotGroupId`,
			bgm.`BotId`
		FROM `botgroupmembers` bgm
		INNER JOIN `bot_groups` bg
		ON bgm.`BotGroupId` = bg.`groups_index`
		INNER JOIN `bot_data` bd
		ON bgm.`BotId` = bd.`bot_id`;
		
		RENAME TABLE `botgroupmembers` TO `botgroupmembers_old`;
	END IF;
	
	CREATE TABLE `bot_guild_members` (
		`bot_id` INT(11) NOT NULL DEFAULT '0',
		`guild_id` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
		`rank` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
		`tribute_enable` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
		`total_tribute` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`last_tribute` INT(10) UNSIGNED NOT NULL DEFAULT '0',
		`banker` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
		`public_note` TEXT NULL,
		`alt` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
		PRIMARY KEY  (`bot_id`)
	) ENGINE=InnoDB;
	IF ((SELECT COUNT(*) FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'botguildmembers') > 0) THEN
		INSERT INTO `bot_guild_members` (
			`bot_id`,
			`guild_id`,
			`rank`,
			`tribute_enable`,
			`total_tribute`,
			`last_tribute`,
			`banker`,
			`public_note`,
			`alt`
		)
		SELECT
			bgm.`char_id`,
			bgm.`guild_id`,
			bgm.`rank`,
			bgm.`tribute_enable`,
			bgm.`total_tribute`,
			bgm.`last_tribute`,
			bgm.`banker`,
			bgm.`public_note`,
			bgm.`alt`
		FROM `botguildmembers` bgm
		INNER JOIN `guilds` g
		ON bgm.`guild_id` = g.`id`
		INNER JOIN `bot_data` bd
		ON bgm.`char_id` = bd.`bot_id`;
		
		RENAME TABLE `botguildmembers` TO `botguildmembers_old`;
	END IF;
	
END$$

DELIMITER ;


CALL `LoadBotsSchema`();

DROP PROCEDURE IF EXISTS `LoadBotsSchema`;


-- Functions
DELIMITER $$

-- (no code references - see `vw_groups` below)
CREATE FUNCTION `GetMobType` (mob_name VARCHAR(64)) RETURNS CHAR(1)
BEGIN
	DECLARE Result CHAR(1);
	
	SET Result = NULL;
	
	IF ((SELECT COUNT(*) FROM `character_data` WHERE `name` = mob_name) > 0) THEN
		SET Result = 'C';
	ELSEIF ((SELECT COUNT(*) FROM `bot_data` WHERE `name` = mob_name) > 0) THEN
		SET Result = 'B';
	END IF;
	
	RETURN Result;
END$$

-- (one code reference in /common/database.cpp)
CREATE FUNCTION `GetMobTypeById` (mob_id INTEGER UNSIGNED) RETURNS CHAR(1)
BEGIN
	DECLARE Result CHAR(1);
	
	SET Result = NULL;
	
	IF ((select `id` from `character_data` where `id` = mob_id) > 0) THEN
		SET Result = 'C';
	ELSEIF ((select `bot_id` from `bot_data` where `bot_id` = mob_id) > 0) THEN
		SET Result = 'B';
	END IF;
	
	RETURN Result;
END$$

-- (for reference only)
-- CREATE FUNCTION `GetMobTypeByName` (mob_name VARCHAR(64)) RETURNS CHAR(1)
-- BEGIN
--	DECLARE Result CHAR(1);
--	
--	SET Result = NULL;
--	
--	IF (select `id` from `character_data` where `name` = mob_name) > 0 THEN
--		SET Result = 'C';
--	ELSEIF (select `bot_id` from `bot_data` where `name` = mob_name) > 0 THEN
--		SET Result = 'B';
--	END IF;
--	
--	RETURN Result;
-- END $$

DELIMITER ;


-- Views
CREATE VIEW `vw_bot_character_mobs` AS
SELECT
_utf8'C' AS mob_type,
c.`id`,
c.`name`,
c.`class`,
c.`level`,
c.`last_login`,
c.`zone_id`
FROM `character_data` AS c
UNION ALL
SELECT _utf8'B' AS mob_type,
b.`bot_id` AS id,
b.`name`,
b.`class`,
b.`level`,
b.`last_spawn` AS last_login,
b.`zone_id`
FROM `bot_data` AS b;

CREATE VIEW `vw_bot_groups` AS
SELECT
g.`groups_index`,
g.`group_name`,
g.`group_leader_id`,
b.`name` AS group_leader_name,
b.`owner_id`,
c.`name` AS owner_name
FROM `bot_groups` AS g
JOIN `bot_data` AS b ON g.`group_leader_id` = b.`bot_id`
JOIN `character_data` AS c ON b.`owner_id` = c.`id`
ORDER BY b.`owner_id`, g.`group_name`;

CREATE VIEW `vw_groups` AS
SELECT
g.`groupid` AS group_id,
GetMobType(g.`name`) AS mob_type,
g.`name` AS name,
g.`charid` AS mob_id,
IFNULL(c.`level`, b.`level`) AS level
FROM `group_id` AS g
LEFT JOIN `character_data` AS c ON g.`name` = c.`name`
LEFT JOIN `bot_data` AS b ON g.`name` = b.`name`;

CREATE VIEW `vw_guild_members` AS
SELECT
'C' AS mob_type,
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
SELECT
'B' AS mob_type,
bm.`bot_id` AS char_id,
bm.`guild_id`,
bm.`rank`,
bm.`tribute_enable`,
bm.`total_tribute`,
bm.`last_tribute`,
bm.`banker`,
bm.`public_note`,
bm.`alt`
FROM `bot_guild_members` AS bm;


-- End of File
