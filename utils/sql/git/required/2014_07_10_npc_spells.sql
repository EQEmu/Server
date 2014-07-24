-- npc_types
ALTER TABLE  `npc_types` ADD  `ammo_idfile` varchar( 30 ) NOT NULL DEFAULT  'IT10' AFTER `d_meele_texture2`;
ALTER TABLE  `npc_types` ADD  `ranged_type` tinyint( 4 ) UNSIGNED NOT NULL DEFAULT  '7' AFTER `sec_melee_type`;
ALTER TABLE  `npc_types` ADD  `Avoidance` mediumint(9) UNSIGNED NOT NULL DEFAULT  '0' AFTER `Accuracy`;

-- npc spells
ALTER TABLE  `npc_spells` ADD  `range_proc` smallint(5) NOT NULL DEFAULT '-1';
ALTER TABLE  `npc_spells` ADD  `rproc_chance` smallint(5) NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `defensive_proc` smallint(5) NOT NULL DEFAULT '-1';
ALTER TABLE  `npc_spells` ADD  `dproc_chance` smallint(5) NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `fail_recast` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_no_sp_recast_min` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_b_self_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `engaged_b_other_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_d_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_no_sp_recast_min` int(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_d_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_no_sp_recast_min` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_b_chance` tinyint(11) unsigned NOT NULL DEFAULT '0';