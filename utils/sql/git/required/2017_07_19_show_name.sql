ALTER TABLE `npc_types` ADD COLUMN `show_name` TINYINT(2) NOT NULL DEFAULT 1;
ALTER TABLE `npc_types` ADD COLUMN `untargetable` TINYINT(2) NOT NULL DEFAULT 0;
UPDATE `npc_types` SET `show_name` = 0, `untargetable` = 1 WHERE `bodytype` >= 66;
