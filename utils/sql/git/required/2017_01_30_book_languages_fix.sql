UPDATE `books` SET `language` = '0' WHERE `language` IS NULL;

ALTER TABLE `books` MODIFY COLUMN `language` INT NOT NULL DEFAULT '0';

