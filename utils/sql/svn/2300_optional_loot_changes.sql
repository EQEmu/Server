ALTER TABLE `lootdrop_entries`
MODIFY COLUMN `maxlevel`  tinyint(3) UNSIGNED NOT NULL DEFAULT 255 AFTER `minlevel`;
UPDATE `lootdrop_entries` SET `maxlevel` = 255 WHERE `maxlevel` = 127;