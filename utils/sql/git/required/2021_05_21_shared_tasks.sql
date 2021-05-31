CREATE TABLE `shared_tasks`
(
    `id`              bigint(20) NOT NULL AUTO_INCREMENT,
    `task_id`         int(11) DEFAULT NULL,
    `accepted_time`   int(11) DEFAULT NULL,
    `completion_time` int(11) DEFAULT NULL,
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
    `updated_time`   int(11) DEFAULT NULL,
    `completed_time` int(11) DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `activity_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
