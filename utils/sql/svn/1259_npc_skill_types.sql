ALTER TABLE `npc_types` ADD COLUMN `prim_melee_type` TINYINT(4) UNSIGNED NOT NULL DEFAULT 28 AFTER `d_meele_texture2`;
ALTER TABLE `npc_types` ADD COLUMN `sec_melee_type` TINYINT(4) UNSIGNED NOT NULL DEFAULT 28 AFTER `prim_melee_type`;
