/* Add the new Aug Slot 6 Fields to the items table */
ALTER TABLE `items` ADD `augslot6type` tinyint( 3 ) NOT NULL DEFAULT '0' AFTER `augslot5visible`;
ALTER TABLE `items` ADD `augslot6visible` tinyint( 3 ) NOT NULL DEFAULT '0' AFTER `augslot6type`;
ALTER TABLE `items` ADD `augslot6unk2` int( 11 ) NOT NULL DEFAULT '0' AFTER `augslot5unk2`;

/* Add the new Aug Slot 6 Field to the inventory table */
ALTER TABLE `inventory` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the sharedbank table */
ALTER TABLE `sharedbank` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the object_contents table */
ALTER TABLE `object_contents` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the sharedbank table */
ALTER TABLE `character_corpse_items` ADD `aug_6` int( 11 ) NOT NULL DEFAULT '0' AFTER `aug_5`;