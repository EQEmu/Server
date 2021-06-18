-- ----------------------------
-- Table structure for server_first_levels
-- ----------------------------
CREATE TABLE `server_first_levels` (
	`char_name` VARCHAR(64) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci',
	`leveled_date` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`race` INT(11) NOT NULL DEFAULT '0',
	`class` INT(11) NOT NULL DEFAULT '0',
	`level` INT(11) NOT NULL DEFAULT '0',
	`account_status` INT(11) UNSIGNED NOT NULL DEFAULT '0'
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;