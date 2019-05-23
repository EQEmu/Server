ALTER TABLE `tasks` ADD `reward_points` INT NOT NULL DEFAULT '0' AFTER `rewardmethod`;
ALTER TABLE `tasks` ADD `reward_type` INT NOT NULL DEFAULT '0' AFTER `reward_points`;
ALTER TABLE `tasks` ADD `replay_group` INT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `min_players` INT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `max_players` INT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `task_lock_step` INT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `instance_zone_id` INT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `zone_version` INT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `zone_in_zone_id` INT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `zone_in_x` FLOAT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `zone_in_y` FLOAT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `zone_in_object_id` TINYINT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `dest_x` FLOAT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `dest_y` FLOAT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `dest_z` FLOAT NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `dest_h` FLOAT NOT NULL DEFAULT '0';
CREATE TABLE `task_replay_groups` (
	`id` INT NOT NULL,
	`duration` INT NOT NULL,
	`name` VARCHAR(128) NOT NULL DEFAULT '',
	PRIMARY KEY(`id`)
);
CREATE TABLE `character_task_lockouts` (
	`charid` INT NOT NULL,
	`replay_group` INT NOT NULL,
	`original_id` INT NOT NULL,
	`timestamp` INT NOT NULL,
	PRIMARY KEY(`charid`, `replay_group`)
);
CREATE TABLE `shared_task_state` (
	`id` INT NOT NULL,
	`taskid` INT NOT NULL,
	`acceptedtime` INT NOT NULL,
	`locked` TINYINT NOT NULL DEFAULT '0',
	PRIMARY KEY(`id`)
);
CREATE TABLE `shared_task_activities` (
	`shared_id` INT NOT NULL,
	`activity_id` INT NOT NULL,
	`done_count` INT NOT NULL,
	`completed` TINYINT,
	PRIMARY KEY(`shared_id`, `activity_id`)
);
CREATE TABLE `shared_task_members` (
	`shared_id` INT NOT NULL,
	`charid` INT NOT NULL,
	`name` VARCHAR(64) NOT NULL,
	`leader` TINYINT,
	PRIMARY KEY(`charid`)
);
