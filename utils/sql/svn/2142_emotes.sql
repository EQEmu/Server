CREATE TABLE `npc_emotes` (
  `id` int(10) NOT NULL auto_increment,
  `emoteid` int(10) unsigned NOT NULL default '0',
  `event_` tinyint(3) NOT NULL default '0',
  `type` tinyint(3) NOT NULL default '0',
  `text` varchar(512) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `emoteid` (`emoteid`,`event_`)
) ENGINE=InnoDB AUTO_INCREMENT=579 DEFAULT CHARSET=latin1;

alter table npc_types add column `emoteid` int(10) NOT NULL default '0';

insert into commands values ('emoteview', 80, 'Lists all of a NPCs loaded emotes');
insert into commands values ('reloadview', 80, 'Reloads NPC emotes');
insert into commands values ('emotesearch', 80, 'Searches loaded NPC emotes');