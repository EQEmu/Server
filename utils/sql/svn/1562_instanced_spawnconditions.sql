CREATE TABLE `spawn_condition_values` (
  `id` int(10) unsigned NOT NULL,
  `value` tinyint(3) unsigned default NULL,
  `zone` varchar(64) NOT NULL,
  `instance_id` int(10) unsigned NOT NULL,
  UNIQUE KEY `instance` (`id`,`instance_id`,`zone`),
  KEY `zoneinstance` (`zone`,`instance_id`)
) ENGINE=InnoDB;