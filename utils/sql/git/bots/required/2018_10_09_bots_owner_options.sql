DROP TABLE IF EXISTS `bot_owner_options`;

CREATE TABLE `bot_owner_options` (
	`owner_id` INT(11) UNSIGNED NOT NULL,
	`death_marquee` SMALLINT(3) UNSIGNED NULL DEFAULT '0',
	PRIMARY KEY (`owner_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=MyISAM;

INSERT INTO `bot_command_settings`(`bot_command`, `access`, `aliases`) VALUES ('owneroption', '0', 'oo');
