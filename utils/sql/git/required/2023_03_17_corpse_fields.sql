ALTER TABLE `character_corpse_items`
	ADD COLUMN `custom_data` TEXT NULL AFTER `attuned`,
	ADD COLUMN `ornamenticon` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `custom_data`,
	ADD COLUMN `ornamentidfile` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamenticon`,
	ADD COLUMN `ornament_hero_model` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamentidfile`;
