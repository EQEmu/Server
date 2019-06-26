DROP TABLE IF EXISTS `inventory_versions`;
DROP TABLE IF EXISTS `inventory_snapshots`;


CREATE TABLE `inventory_versions` (
	`version` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`step` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`bot_step` INT(11) UNSIGNED NOT NULL DEFAULT '0'
)
COLLATE='latin1_swedish_ci'
ENGINE=MyISAM;

INSERT INTO `inventory_versions` VALUES (2, 0, 0);


CREATE TABLE `inventory_snapshots` (
	`time_index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`charid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`slotid` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`itemid` INT(11) UNSIGNED NULL DEFAULT '0',
	`charges` SMALLINT(3) UNSIGNED NULL DEFAULT '0',
	`color` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`augslot1` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot2` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot3` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot4` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot5` MEDIUMINT(7) UNSIGNED NULL DEFAULT '0',
	`augslot6` MEDIUMINT(7) NOT NULL DEFAULT '0',
	`instnodrop` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	`custom_data` TEXT NULL,
	`ornamenticon` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornamentidfile` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornament_hero_model` INT(11) NOT NULL DEFAULT '0',
	PRIMARY KEY (`time_index`, `charid`, `slotid`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;


CREATE TABLE `inventory_snapshots_v1_bak` (
	`time_index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`charid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`slotid` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`itemid` INT(11) UNSIGNED NULL DEFAULT '0',
	`charges` SMALLINT(3) UNSIGNED NULL DEFAULT '0',
	`color` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`augslot1` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot2` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot3` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot4` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot5` MEDIUMINT(7) UNSIGNED NULL DEFAULT '0',
	`augslot6` MEDIUMINT(7) NOT NULL DEFAULT '0',
	`instnodrop` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	`custom_data` TEXT NULL,
	`ornamenticon` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornamentidfile` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornament_hero_model` INT(11) NOT NULL DEFAULT '0',
	PRIMARY KEY (`time_index`, `charid`, `slotid`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;


-- create inventory v1 backup
SELECT @pre_timestamp := UNIX_TIMESTAMP(NOW());
INSERT INTO `inventory_snapshots_v1_bak`
  (`time_index`,`charid`,`slotid`,`itemid`,`charges`,`color`,`augslot1`,`augslot2`,`augslot3`,`augslot4`,
  `augslot5`,`augslot6`,`instnodrop`,`custom_data`,`ornamenticon`,`ornamentidfile`,`ornament_hero_model`)
SELECT
  @pre_timestamp, `charid`, `slotid`, `itemid`, `charges`, `color`, `augslot1`, `augslot2`, `augslot3`, `augslot4`,
  `augslot5`,`augslot6`,`instnodrop`,`custom_data`,`ornamenticon`,`ornamentidfile`,`ornament_hero_model`
FROM `inventory`;


-- update equipable slots in `items` table
SELECT 'pre-transform count..',
(SELECT COUNT(id) FROM `items` WHERE `slots` & (3 << 21)) total,
(SELECT COUNT(id) FROM `items` WHERE `slots` & (1 << 21)) bit21,
(SELECT COUNT(id) FROM `items` WHERE `slots` & (1 << 22)) bit22;

UPDATE `items` SET `slots` = (`slots` ^ (3 << 21)) WHERE (`slots` & (3 << 21)) IN ((1 << 21), (1 << 22)); -- transform

SELECT 'post-transform count..',
(SELECT COUNT(id) FROM `items` WHERE `slots` & (3 << 21)) total,
(SELECT COUNT(id) FROM `items` WHERE `slots` & (1 << 21)) bit21,
(SELECT COUNT(id) FROM `items` WHERE `slots` & (1 << 22)) bit22;


-- update `inventory` slots
UPDATE `inventory` SET `slotid` = 33 WHERE `slotid` = 30; -- adjust cursor
UPDATE `inventory` SET `slotid` = (`slotid` + 20) WHERE `slotid` >= 331 AND `slotid` <= 340; -- adjust cursor bags
UPDATE `inventory` SET `slotid` = 30 WHERE `slotid` = 29; -- adjust general8 slot
UPDATE `inventory` SET `slotid` = 29 WHERE `slotid` = 28; -- adjust general7 slot
UPDATE `inventory` SET `slotid` = 28 WHERE `slotid` = 27; -- adjust general6 slot
UPDATE `inventory` SET `slotid` = 27 WHERE `slotid` = 26; -- adjust general5 slot
UPDATE `inventory` SET `slotid` = 26 WHERE `slotid` = 25; -- adjust general4 slot
UPDATE `inventory` SET `slotid` = 25 WHERE `slotid` = 24; -- adjust general3 slot
UPDATE `inventory` SET `slotid` = 24 WHERE `slotid` = 23; -- adjust general2 slot
UPDATE `inventory` SET `slotid` = 23 WHERE `slotid` = 22; -- adjust general1 slot
-- current general bags remain the same
UPDATE `inventory` SET `slotid` = 22 WHERE `slotid` = 21; -- adjust ammo slot
UPDATE `inventory` SET `slotid` = 21 WHERE `slotid` = 9999; -- adjust powersource slot


-- update `character_corpse_items` slots
UPDATE `character_corpse_items` SET `equip_slot` = 33 WHERE `equip_slot` = 30; -- adjust cursor
UPDATE `character_corpse_items` SET `equip_slot` = (`equip_slot` + 20) WHERE `equip_slot` >= 331 AND `equip_slot` <= 340; -- adjust cursor bags
UPDATE `character_corpse_items` SET `equip_slot` = 30 WHERE `equip_slot` = 29; -- adjust general8 slot
UPDATE `character_corpse_items` SET `equip_slot` = 29 WHERE `equip_slot` = 28; -- adjust general7 slot
UPDATE `character_corpse_items` SET `equip_slot` = 28 WHERE `equip_slot` = 27; -- adjust general6 slot
UPDATE `character_corpse_items` SET `equip_slot` = 27 WHERE `equip_slot` = 26; -- adjust general5 slot
UPDATE `character_corpse_items` SET `equip_slot` = 26 WHERE `equip_slot` = 25; -- adjust general4 slot
UPDATE `character_corpse_items` SET `equip_slot` = 25 WHERE `equip_slot` = 24; -- adjust general3 slot
UPDATE `character_corpse_items` SET `equip_slot` = 24 WHERE `equip_slot` = 23; -- adjust general2 slot
UPDATE `character_corpse_items` SET `equip_slot` = 23 WHERE `equip_slot` = 22; -- adjust general1 slot
-- current general bags remain the same
UPDATE `character_corpse_items` SET `equip_slot` = 22 WHERE `equip_slot` = 21; -- adjust ammo slot
UPDATE `character_corpse_items` SET `equip_slot` = 21 WHERE `equip_slot` = 9999; -- adjust powersource slot


-- update `character_pet_inventory` slots
UPDATE `character_pet_inventory` SET `slot` = 22 WHERE `slot` = 21; -- adjust ammo slot

UPDATE `inventory_versions` SET `step` = 1 WHERE `version` = 2;


-- create initial inventory v2 snapshots
SELECT @post_timestamp := UNIX_TIMESTAMP(NOW());
INSERT INTO `inventory_snapshots`
  (`time_index`,`charid`,`slotid`,`itemid`,`charges`,`color`,`augslot1`,`augslot2`,`augslot3`,`augslot4`,
  `augslot5`,`augslot6`,`instnodrop`,`custom_data`,`ornamenticon`,`ornamentidfile`,`ornament_hero_model`)
SELECT
  @post_timestamp, `charid`, `slotid`, `itemid`, `charges`, `color`, `augslot1`, `augslot2`, `augslot3`, `augslot4`,
  `augslot5`,`augslot6`,`instnodrop`,`custom_data`,`ornamenticon`,`ornamentidfile`,`ornament_hero_model`
FROM `inventory`;
