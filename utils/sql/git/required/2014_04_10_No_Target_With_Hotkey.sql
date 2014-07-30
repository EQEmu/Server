-- npc_types
ALTER TABLE  `npc_types` ADD  `ammo_idfile` varchar( 30 ) NOT NULL DEFAULT  'IT10' AFTER `d_meele_texture2`;
ALTER TABLE  `npc_types` ADD  `ranged_type` tinyint( 4 ) UNSIGNED NOT NULL DEFAULT  '7' AFTER `sec_melee_type`;


