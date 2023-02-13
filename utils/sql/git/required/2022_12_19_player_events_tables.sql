CREATE TABLE `player_event_log_settings`
(
    `id`                 bigint(20) NOT NULL,
    `event_name`         varchar(100) DEFAULT NULL,
    `event_enabled`      tinyint(1) DEFAULT NULL,
    `retention_days`     int(11) DEFAULT 0,
    `discord_webhook_id` int(11) DEFAULT 0,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `player_event_logs`
(
    `id`              bigint(20) NOT NULL AUTO_INCREMENT,
    `account_id`      bigint(20) DEFAULT NULL,
    `character_id`    bigint(20) DEFAULT NULL,
    `zone_id`         int(11) DEFAULT NULL,
    `instance_id`     int(11) DEFAULT NULL,
    `x`               float        DEFAULT NULL,
    `y`               float        DEFAULT NULL,
    `z`               float        DEFAULT NULL,
    `heading`         float        DEFAULT NULL,
    `event_type_id`   int(11) DEFAULT NULL,
    `event_type_name` varchar(255) DEFAULT NULL,
    `event_data`      longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL CHECK (json_valid(`event_data`)),
    `created_at`      datetime     DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY               `event_created_at` (`event_type_id`,`created_at`),
    KEY               `zone_id` (`zone_id`),
    KEY               `character_id` (`character_id`,`zone_id`) USING BTREE,
    KEY               `created_at` (`created_at`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;

DROP TABLE `hackers`;
DROP TABLE `eventlog`;
