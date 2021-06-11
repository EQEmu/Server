CREATE TABLE `dynamic_zone_members` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `dynamic_zone_id` int(10) unsigned NOT NULL DEFAULT 0,
  `character_id` int(10) unsigned NOT NULL DEFAULT 0,
  `is_current_member` tinyint(3) unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  UNIQUE KEY `dynamic_zone_id_character_id` (`dynamic_zone_id`,`character_id`),
  KEY `character_id` (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE `expedition_members`;
