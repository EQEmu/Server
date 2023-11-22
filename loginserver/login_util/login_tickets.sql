DROP TABLE IF EXISTS `login_tickets`;
CREATE TABLE `login_tickets` (
	`id` VARCHAR(128) NOT NULL,
	`login_server` TEXT NOT NULL,
	`username` TEXT NOT NULL,
	`account_id` INT(10) UNSIGNED NOT NULL,
	`ip_address` VARCHAR(45) NOT NULL,
	`expires` DATETIME NOT NULL,
	PRIMARY KEY (`id`) USING BTREE
)
ENGINE=InnoDB;
