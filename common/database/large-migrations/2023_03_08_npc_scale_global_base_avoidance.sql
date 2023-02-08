ALTER TABLE `npc_scale_global_base`
MODIFY COLUMN `hp` bigint(20) NOT NULL DEFAULT 0 AFTER `ac`,
MODIFY COLUMN `hp_regen_rate` bigint(20) NOT NULL DEFAULT 0 AFTER `max_dmg`,
ADD COLUMN `hp_regen_per_second` bigint(20) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
ADD COLUMN `avoidance` int(11) unsigned NOT NULL DEFAULT 0 AFTER `heal_scale`;
