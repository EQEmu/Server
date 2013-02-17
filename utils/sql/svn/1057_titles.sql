CREATE TABLE `player_titlesets` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `char_id` int(11) unsigned NOT NULL,
  `title_set` int(11) unsigned NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `id` (`id`)
); 

alter table titles add column `title_set` int(11) NOT NULL default '0';