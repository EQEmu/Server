CREATE TABLE `expeditions` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`uuid` VARCHAR(36) NOT NULL,
	`dynamic_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`expedition_name` VARCHAR(128) NOT NULL,
	`leader_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`min_players` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`max_players` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`add_replay_on_join` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
	`is_locked` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `dynamic_zone_id` (`dynamic_zone_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `expedition_lockouts` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`expedition_id` INT(10) UNSIGNED NOT NULL,
	`event_name` VARCHAR(256) NOT NULL,
	`expire_time` DATETIME NOT NULL DEFAULT current_timestamp(),
	`duration` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`from_expedition_uuid` VARCHAR(36) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `expedition_id_event_name` (`expedition_id`, `event_name`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `expedition_members` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`expedition_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`character_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`is_current_member` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `expedition_id_character_id` (`expedition_id`, `character_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `character_expedition_lockouts` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NOT NULL,
	`expedition_name` VARCHAR(128) NOT NULL,
	`event_name` VARCHAR(256) NOT NULL,
	`expire_time` DATETIME NOT NULL DEFAULT current_timestamp(),
	`duration` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`from_expedition_uuid` VARCHAR(36) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `character_id_expedition_name_event_name` (`character_id`, `expedition_name`, `event_name`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

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
	UNIQUE INDEX `instance_id` (`instance_id`)
)
COLLATE='utf8mb4_general_ci'
ENGINE=InnoDB
;
