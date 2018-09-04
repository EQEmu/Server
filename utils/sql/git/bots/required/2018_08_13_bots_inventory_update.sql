-- update `bot_inventories` slots
UPDATE `bot_inventories` SET `slot_id` = 22 WHERE `slot_id` = 21; -- adjust ammo slot
UPDATE `bot_inventories` SET `slot_id` = 21 WHERE `slot_id` = 9999; -- adjust powersource slot

UPDATE `inventory_versions` SET `bot_step` = 1 WHERE `version` = 2;