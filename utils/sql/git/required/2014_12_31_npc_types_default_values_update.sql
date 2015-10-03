UPDATE `npc_types` SET `bodytype` = 0 WHERE `bodytype` IS NULL;
ALTER TABLE `npc_types` MODIFY `bodytype` INT(11) NOT NULL DEFAULT '1';
UPDATE `npc_types` SET `d_melee_texture1` = 0 WHERE `d_melee_texture1` IS NULL;
ALTER TABLE `npc_types` MODIFY `d_melee_texture1` INT(11) NOT NULL DEFAULT '0';
UPDATE `npc_types` SET `d_melee_texture2` = 0 WHERE `d_melee_texture2` IS NULL;
ALTER TABLE `npc_types` MODIFY `d_melee_texture2` INT(11) NOT NULL DEFAULT '0';
