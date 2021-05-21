CREATE TABLE `shared_tasks`
(
    `id`              bigint(20) NOT NULL AUTO_INCREMENT,
    `task_id`         int(11) DEFAULT NULL,
    `dynamic_zone_id` bigint(20) DEFAULT NULL,
    `accepted_time`   int(11) DEFAULT NULL,
    `completion_time` int(11) DEFAULT NULL,
    `is_locked`       tinyint(1) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_members`
(
    `id`             bigint(20) NOT NULL AUTO_INCREMENT,
    `shared_task_id` bigint(20) DEFAULT NULL,
    `character_id`   bigint(20) DEFAULT NULL,
    `is_leader`      tinyint(4) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_activity_state`
(
    `id`             bigint(20) NOT NULL AUTO_INCREMENT,
    `shared_task_id` bigint(20) DEFAULT NULL,
    `activity_id`    int(11) DEFAULT NULL,
    `done_count`     int(11) DEFAULT NULL,
    `updated_time`   int(11) DEFAULT NULL,
    `completed_time` int(11) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
