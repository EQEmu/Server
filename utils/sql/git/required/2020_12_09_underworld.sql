ALTER TABLE `zone` ADD COLUMN `underworld_teleport_index` INT(4) NOT NULL DEFAULT '0';
UPDATE `zone` SET `underworld` = '-2030' WHERE `zoneidnumber` = '71';
UPDATE `zone` SET `underworld_teleport_index` = '11' WHERE `zoneidnumber` = '71';
UPDATE `zone` SET `underworld_teleport_index` = '-1' WHERE `zoneidnumber` = '75';
UPDATE `zone` SET `underworld_teleport_index` = '-1' WHERE `zoneidnumber` = '150';

