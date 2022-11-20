CREATE TABLE `bot_spell_settings` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `bot_id` int(11) NOT NULL DEFAULT 0,
  `spell_id` smallint(5) NOT NULL DEFAULT 0,
  `priority` smallint(5) NOT NULL DEFAULT 0,
  `min_level` smallint(5) unsigned NOT NULL DEFAULT 0,
  `max_level` smallint(5) unsigned NOT NULL DEFAULT 255,
  `min_hp` smallint(5) NOT NULL DEFAULT 0,
  `max_hp` smallint(5) NOT NULL DEFAULT 0,
  `is_enabled` tinyint(1) unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;