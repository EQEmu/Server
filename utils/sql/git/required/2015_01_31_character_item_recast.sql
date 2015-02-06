CREATE TABLE `character_item_recast` (
    `id` int(11) UNSIGNED NOT NULL DEFAULT 0,
    `recast_type` smallint(11) UNSIGNED NOT NULL DEFAULT 0,
    `timestamp` int(11) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY(`id`, `recast_type`),
    KEY `id` (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = latin1;
