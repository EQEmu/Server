CREATE TABLE `reports` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name` varchar(64) default NULL,
  `reported` varchar(64) default NULL,
  `reported_text` text,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB;