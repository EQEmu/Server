ALTER TABLE `npc_types`  ADD COLUMN `underwater` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `unique_spawn_by_name`;
