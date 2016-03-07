ALTER TABLE `character_bind` ADD `slot` int(4) AFTER `id`;
UPDATE `character_bind` SET `slot`='0' WHERE `is_home`=0;
UPDATE `character_bind` SET `slot`='4' WHERE `is_home`=1;
ALTER TABLE `character_bind` DROP PRIMARY KEY, ADD PRIMARY KEY(`id`, `slot`);
ALTER TABLE `character_bind` DROP COLUMN `is_home`;

