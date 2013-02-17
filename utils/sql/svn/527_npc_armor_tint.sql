ALTER TABLE `npc_types` ADD COLUMN `armortint_red` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `drakkin_details`,
ADD COLUMN `armortint_green` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `armortint_red`,
ADD COLUMN `armortint_blue` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `armortint_green`;