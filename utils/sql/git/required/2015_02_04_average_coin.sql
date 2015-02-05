ALTER TABLE `loottable` CHANGE COLUMN `avgcoin` `avgcoin` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `maxcash`;
UPDATE `loottable` SET avgcoin = 0;
