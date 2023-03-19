ALTER TABLE `npc_types` 
ADD COLUMN `keeps_sold_items` tinyint(1) UNSIGNED NOT NULL DEFAULT 1 AFTER `faction_amount`;