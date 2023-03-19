CREATE TABLE `dynamic_zone_templates` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `zone_id` int(11) NOT NULL DEFAULT 0,
  `zone_version` int(11) NOT NULL DEFAULT 0,
  `name` varchar(128) NOT NULL DEFAULT '',
  `min_players` int(11) NOT NULL DEFAULT 0,
  `max_players` int(11) NOT NULL DEFAULT 0,
  `duration_seconds` int(11) NOT NULL DEFAULT 0,
  `dz_switch_id` int(11) NOT NULL DEFAULT 0,
  `compass_zone_id` int(11) NOT NULL DEFAULT 0,
  `compass_x` float NOT NULL DEFAULT 0,
  `compass_y` float NOT NULL DEFAULT 0,
  `compass_z` float NOT NULL DEFAULT 0,
  `return_zone_id` int(11) NOT NULL DEFAULT 0,
  `return_x` float NOT NULL DEFAULT 0,
  `return_y` float NOT NULL DEFAULT 0,
  `return_z` float NOT NULL DEFAULT 0,
  `return_h` float NOT NULL DEFAULT 0,
  `override_zone_in` tinyint(4) NOT NULL DEFAULT 0,
  `zone_in_x` float NOT NULL DEFAULT 0,
  `zone_in_y` float NOT NULL DEFAULT 0,
  `zone_in_z` float NOT NULL DEFAULT 0,
  `zone_in_h` float NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `tasks`
  ADD COLUMN `dz_template_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `request_timer_seconds`;
