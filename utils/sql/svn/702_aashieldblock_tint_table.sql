INSERT INTO altadv_vars (skill_id, name, cost, max_level, hotkey_sid, hotkey_sid2, title_sid, desc_sid, type, spellid, prereq_skill, 
prereq_minpoints, spell_type, spell_refresh, classes, berserker, class_type, cost_inc, aa_expansion, special_category, sof_type, 
sof_cost_inc, sof_max_level, sof_next_skill, clientver) VALUES (1287, 'Shield Block', 3, 3, 4294967295, 4294967295, 1287, 
1287, 7, 0, 0, 0, 0, 0, 42, 0, 67, 3, 7, 4294967295, 2, 3, 3, 0, 1);

ALTER TABLE `npc_types` ADD COLUMN `armortint_id` INTEGER UNSIGNED NOT NULL DEFAULT 0 AFTER `drakkin_details`;

CREATE TABLE `npc_types_tint` (
  `id` int unsigned NOT NULL DEFAULT '0',
  `red1h` tinyint unsigned NOT NULL DEFAULT '0',
  `grn1h` tinyint unsigned NOT NULL DEFAULT '0',
  `blu1h` tinyint unsigned NOT NULL DEFAULT '0',
  `red2c` tinyint unsigned NOT NULL DEFAULT '0',
  `grn2c` tinyint unsigned NOT NULL DEFAULT '0',
  `blu2c` tinyint unsigned NOT NULL DEFAULT '0',
  `red3a` tinyint unsigned NOT NULL DEFAULT '0',
  `grn3a` tinyint unsigned NOT NULL DEFAULT '0',
  `blu3a` tinyint unsigned NOT NULL DEFAULT '0',
  `red4b` tinyint unsigned NOT NULL DEFAULT '0',
  `grn4b` tinyint unsigned NOT NULL DEFAULT '0',
  `blu4b` tinyint unsigned NOT NULL DEFAULT '0',
  `red5g` tinyint unsigned NOT NULL DEFAULT '0',
  `grn5g` tinyint unsigned NOT NULL DEFAULT '0',
  `blu5g` tinyint unsigned NOT NULL DEFAULT '0',
  `red6l` tinyint unsigned NOT NULL DEFAULT '0',
  `grn6l` tinyint unsigned NOT NULL DEFAULT '0',
  `blu6l` tinyint unsigned NOT NULL DEFAULT '0',
  `red7f` tinyint unsigned NOT NULL DEFAULT '0',
  `grn7f` tinyint unsigned NOT NULL DEFAULT '0',
  `blu7f` tinyint unsigned NOT NULL DEFAULT '0',
  `red8x` tinyint unsigned NOT NULL DEFAULT '0',
  `grn8x` tinyint unsigned NOT NULL DEFAULT '0',
  `blu8x` tinyint unsigned NOT NULL DEFAULT '0',
  `red9x` tinyint unsigned NOT NULL DEFAULT '0',
  `grn9x` tinyint unsigned NOT NULL DEFAULT '0',
  `blu9x` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;