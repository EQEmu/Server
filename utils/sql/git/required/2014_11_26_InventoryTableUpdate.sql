-- Inventory table update
ALTER TABLE `inventory`
	ADD COLUMN `ornamenticon` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `custom_data`,
	ADD COLUMN `ornamentidfile` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamenticon`;
