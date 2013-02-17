ALTER TABLE `adventure_template` ADD `assa_x` FLOAT DEFAULT '0' NOT NULL AFTER `type_count`;
ALTER TABLE `adventure_template` ADD `assa_y` FLOAT DEFAULT '0' NOT NULL AFTER `assa_x`;
ALTER TABLE `adventure_template` ADD `assa_z` FLOAT DEFAULT '0' NOT NULL AFTER `assa_y`;
ALTER TABLE `adventure_template` ADD `assa_h` FLOAT DEFAULT '0' NOT NULL AFTER `assa_z`;