-- shared task tables
CREATE TABLE `shared_tasks`
(
    `id`              bigint(20) NOT NULL AUTO_INCREMENT,
    `task_id`         int(11) DEFAULT NULL,
    `accepted_time`   datetime DEFAULT NULL,
    `expire_time`     datetime DEFAULT NULL,
    `completion_time` datetime DEFAULT NULL,
    `is_locked`       tinyint(1) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_members`
(
    `shared_task_id` bigint(20) NOT NULL,
    `character_id`   bigint(20) NOT NULL,
    `is_leader`      tinyint(4) DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_activity_state`
(
    `shared_task_id` bigint(20) NOT NULL,
    `activity_id`    int(11) NOT NULL,
    `done_count`     int(11) DEFAULT NULL,
    `updated_time`   datetime DEFAULT NULL,
    `completed_time` datetime DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `activity_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_dynamic_zones`
(
    `shared_task_id`  bigint(20) NOT NULL,
    `dynamic_zone_id` int(10) unsigned NOT NULL,
    PRIMARY KEY (`shared_task_id`, `dynamic_zone_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- completed shared task tables - simply stores completed for reporting and logging

CREATE TABLE `completed_shared_tasks`
(
    `id`              bigint(20) NOT NULL,
    `task_id`         int(11) DEFAULT NULL,
    `accepted_time`   datetime DEFAULT NULL,
    `expire_time`     datetime DEFAULT NULL,
    `completion_time` datetime DEFAULT NULL,
    `is_locked`       tinyint(1) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `completed_shared_task_members`
(
    `shared_task_id` bigint(20) NOT NULL,
    `character_id`   bigint(20) NOT NULL,
    `is_leader`      tinyint(4) DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `completed_shared_task_activity_state`
(
    `shared_task_id` bigint(20) NOT NULL,
    `activity_id`    int(11) NOT NULL,
    `done_count`     int(11) DEFAULT NULL,
    `updated_time`   datetime DEFAULT NULL,
    `completed_time` datetime DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `activity_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- tasks

ALTER TABLE `tasks`
    ADD COLUMN `level_spread` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `maxlevel`,
  ADD COLUMN `min_players` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `level_spread`,
  ADD COLUMN `max_players` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `min_players`,
  ADD COLUMN `replay_timer_seconds` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `completion_emote`,
  ADD COLUMN `request_timer_seconds` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `replay_timer_seconds`;

-- character timers

CREATE TABLE `character_task_timers`
(
    `id`           int(10) unsigned NOT NULL AUTO_INCREMENT,
    `character_id` int(10) unsigned NOT NULL DEFAULT 0,
    `task_id`      int(10) unsigned NOT NULL DEFAULT 0,
    `timer_type`   int(11) NOT NULL DEFAULT 0,
    `expire_time`  datetime NOT NULL DEFAULT current_timestamp(),
    PRIMARY KEY (`id`),
    KEY            `character_id` (`character_id`),
    KEY            `task_id` (`task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `tasks`
    CHANGE COLUMN `completion_emote` `completion_emote` VARCHAR (512) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `faction_reward`;

ALTER TABLE `tasks`
    ADD COLUMN `reward_radiant_crystals` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `rewardmethod`,
  ADD COLUMN `reward_ebon_crystals` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `reward_radiant_crystals`;
