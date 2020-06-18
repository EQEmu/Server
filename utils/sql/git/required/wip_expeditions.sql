CREATE TABLE `expedition_details` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`instance_id` INT(10) NULL DEFAULT NULL,
	`expedition_name` VARCHAR(128) NOT NULL,
	`leader_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`min_players` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`max_players` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`add_replay_on_join` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
	`is_locked` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `instance_id` (`instance_id`),
	CONSTRAINT `FK_expedition_details_instance_list` FOREIGN KEY (`instance_id`) REFERENCES `instance_list` (`id`) ON DELETE SET NULL
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
	`is_inherited` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `expedition_id_event_name` (`expedition_id`, `event_name`),
	CONSTRAINT `FK_expedition_lockouts_expedition_details` FOREIGN KEY (`expedition_id`) REFERENCES `expedition_details` (`id`) ON DELETE CASCADE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `expedition_members` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`expedition_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`character_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`is_current_member` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `expedition_id_character_id` (`expedition_id`, `character_id`),
	CONSTRAINT `FK_expedition_members_expedition_details` FOREIGN KEY (`expedition_id`) REFERENCES `expedition_details` (`id`) ON DELETE CASCADE
)
COLLATE='utf8mb4_general_ci'
ENGINE=InnoDB
;

CREATE TABLE `expedition_character_lockouts` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NOT NULL,
	`expire_time` DATETIME NOT NULL DEFAULT current_timestamp(),
	`duration` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`expedition_name` VARCHAR(128) NOT NULL,
	`event_name` VARCHAR(256) NOT NULL,
	`is_pending` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `character_id_expedition_name_event_name` (`character_id`, `expedition_name`, `event_name`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
ROW_FORMAT=DYNAMIC
;
