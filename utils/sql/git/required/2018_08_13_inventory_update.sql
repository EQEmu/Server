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
UPDATE `inventory` SET `slotid` = (`slotid` + 1) WHERE `slotid` >= 22 AND `slotid` <= 29; -- adjust general slots
-- current general bags remain the same
UPDATE `inventory` SET `slotid` = 22 WHERE `slotid` = 21; -- adjust ammo slot
UPDATE `inventory` SET `slotid` = 21 WHERE `slotid` = 9999; -- adjust powersource slot


-- update `character_corpse_items` slots
UPDATE `character_corpse_items` SET `equip_slot` = 33 WHERE `equip_slot` = 30; -- adjust cursor
UPDATE `character_corpse_items` SET `equip_slot` = (`equip_slot` + 20) WHERE `equip_slot` >= 331 AND `equip_slot` <= 340; -- adjust cursor bags
UPDATE `character_corpse_items` SET `equip_slot` = (`equip_slot` + 1) WHERE `equip_slot` >= 22 AND `equip_slot` <= 29; -- adjust general slots
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
