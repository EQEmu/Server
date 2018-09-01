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
