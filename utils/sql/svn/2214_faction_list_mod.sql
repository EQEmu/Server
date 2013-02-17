CREATE TABLE `faction_list_mod` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `faction_id` int(10) unsigned NOT NULL,
  `mod` smallint(6) NOT NULL,
  `mod_name` varchar(16) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `faction_id_mod_name` (`faction_id`,`mod_name`)
);