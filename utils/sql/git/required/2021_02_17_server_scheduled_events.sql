CREATE TABLE `server_scheduled_events`
(
    `id`              int(11) NOT NULL AUTO_INCREMENT,
    `description`     varchar(255) DEFAULT NULL,
    `event_type`      varchar(100) DEFAULT NULL,
    `event_data`      text         DEFAULT NULL,
    `minute_start`    int(11) DEFAULT 0,
    `hour_start`      int(11) DEFAULT 0,
    `day_start`       int(11) DEFAULT 0,
    `month_start`     int(11) DEFAULT 0,
    `year_start`      int(11) DEFAULT 0,
    `minute_end`      int(11) DEFAULT 0,
    `hour_end`        int(11) DEFAULT 0,
    `day_end`         int(11) DEFAULT 0,
    `month_end`       int(11) DEFAULT 0,
    `year_end`        int(11) DEFAULT 0,
    `cron_expression` varchar(100) DEFAULT NULL,
    `created_at`      datetime     DEFAULT NULL,
    `deleted_at`      datetime     DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
