DROP TABLE IF EXISTS `character_inventory`;
CREATE TABLE `character_inventory` (
	`id` INT(10) UNSIGNED NOT NULL,
	`type` SMALLINT(6) NOT NULL,
	`slot` SMALLINT(6) NOT NULL,
	`bag_index` SMALLINT(6) NOT NULL,
	`aug_index` SMALLINT(6) NOT NULL,
	`item_id` INT(10) UNSIGNED NOT NULL,
	`charges` SMALLINT(6) NOT NULL,
	`color` INT(10) UNSIGNED NOT NULL,
	`attuned` TINYINT(3) UNSIGNED NOT NULL,
	`custom_data` TEXT NOT NULL,
	`ornament_icon` INT(10) UNSIGNED NOT NULL,
	`ornament_idfile` INT(10) UNSIGNED NOT NULL,
	`ornament_hero_model` INT(10) UNSIGNED NOT NULL,
	`tracking_id` BIGINT(20) UNSIGNED NOT NULL,
	PRIMARY KEY (`id`, `type`, `slot`, `bag_index`, `aug_index`),
	INDEX `tracking_id` (`tracking_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;
