CREATE TABLE `dynamic_zones` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`instance_id` INT(10) NOT NULL DEFAULT 0,
	`type` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`compass_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`compass_x` FLOAT NOT NULL DEFAULT 0,
	`compass_y` FLOAT NOT NULL DEFAULT 0,
	`compass_z` FLOAT NOT NULL DEFAULT 0,
	`safe_return_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`safe_return_x` FLOAT NOT NULL DEFAULT 0,
	`safe_return_y` FLOAT NOT NULL DEFAULT 0,
	`safe_return_z` FLOAT NOT NULL DEFAULT 0,
	`safe_return_heading` FLOAT NOT NULL DEFAULT 0,
	`zone_in_x` FLOAT NOT NULL DEFAULT 0,
	`zone_in_y` FLOAT NOT NULL DEFAULT 0,
	`zone_in_z` FLOAT NOT NULL DEFAULT 0,
	`zone_in_heading` FLOAT NOT NULL DEFAULT 0,
	`has_zone_in` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `instance_id` (`instance_id`),
	CONSTRAINT `FK_dynamic_zones_instance_list` FOREIGN KEY (`instance_id`) REFERENCES `instance_list` (`id`) ON DELETE CASCADE
)
COLLATE='utf8mb4_general_ci'
ENGINE=InnoDB
;
