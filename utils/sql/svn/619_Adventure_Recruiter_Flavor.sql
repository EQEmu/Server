CREATE TABLE `adventure_template_entry_flavor` (
  `id` int(10) unsigned NOT NULL,
  `text` varchar(512) character set utf8 collate utf8_bin NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `id_2` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;