DROP TABLE IF EXISTS `bot_owner_options`;

CREATE TABLE `bot_owner_options` (
	`owner_id` INT(11) UNSIGNED NOT NULL,
	`option_type` SMALLINT(3) UNSIGNED NOT NULL,
	`option_value` SMALLINT(3) UNSIGNED NULL DEFAULT '0',
	PRIMARY KEY (`owner_id`, `option_type`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;
