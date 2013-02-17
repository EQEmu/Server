ALTER TABLE `npc_types` ADD `trap_template` INT UNSIGNED NOT NULL AFTER `adventure_template_id`;

CREATE TABLE `ldon_trap_templates` (
  `id` int(10) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL default '1',
  `spell_id` smallint(5) unsigned NOT NULL default '0',
  `skill` smallint(5) unsigned NOT NULL default '0',
  `locked` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `ldon_trap_entries` (
  `id` int(10) unsigned NOT NULL,
  `trap_id` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`,`trap_id`),
  KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;