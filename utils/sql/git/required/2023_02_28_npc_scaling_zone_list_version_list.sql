ALTER TABLE `npc_scale_global_base`
    CHANGE COLUMN `zone_id` `zone_id_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER `level`,
    CHANGE COLUMN `instance_version` `instance_version_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER `zone_id_list`,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY (`type`, `level`, `zone_id_list`(255), `instance_version_list`(255)) USING BTREE;
