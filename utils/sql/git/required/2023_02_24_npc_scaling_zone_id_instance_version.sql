ALTER TABLE `npc_scale_global_base`
ADD COLUMN `zone_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `level`,
ADD COLUMN `instance_version` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `zone_id`,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`type`, `level`, `zone_id`, `instance_version`) USING BTREE;
