CREATE TABLE `alternate_currency` (
	`id` INT(10) NOT NULL,
	`item_id` INT(10) NOT NULL,
	PRIMARY KEY (`id`)
);

CREATE TABLE `character_alt_currency` (
	`char_id` INT(10) UNSIGNED NOT NULL,
	`currency_id` INT(10) UNSIGNED NOT NULL,
	`amount` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`char_id`, `currency_id`)
)
ENGINE=InnoDB;

ALTER TABLE `merchantlist`  ADD COLUMN `faction_required` SMALLINT NOT NULL DEFAULT '-100' AFTER `item`,  ADD COLUMN `level_required` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `faction_required`,  ADD COLUMN `alt_currency_cost` SMALLINT UNSIGNED NOT NULL DEFAULT '0' AFTER `level_required`;
ALTER TABLE `npc_types`  ADD COLUMN `alt_currency_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `merchant_id`;
ALTER TABLE `lootdrop_entries`  CHANGE COLUMN `item_charges` `item_charges` SMALLINT(2) UNSIGNED NOT NULL DEFAULT '1' AFTER `item_id`;
ALTER TABLE `inventory`  CHANGE COLUMN `charges` `charges` SMALLINT(3) UNSIGNED NULL DEFAULT '0' AFTER `itemid`;
ALTER TABLE `sharedbank`  CHANGE COLUMN `charges` `charges` SMALLINT(3) UNSIGNED NULL DEFAULT '0' AFTER `itemid`;
ALTER TABLE `object`  CHANGE COLUMN `charges` `charges` SMALLINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `itemid`;
ALTER TABLE `object_contents`  CHANGE COLUMN `charges` `charges` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `itemid`;
