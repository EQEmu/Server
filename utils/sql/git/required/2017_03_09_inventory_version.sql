DROP TABLE IF EXISTS `inventory_version`;

CREATE TABLE `inventory_version` (
	`version` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`step` INT(11) UNSIGNED NOT NULL DEFAULT '0'
)
COLLATE='latin1_swedish_ci'
ENGINE=MyISAM
;

INSERT INTO `inventory_version` VALUES (2, 0);
