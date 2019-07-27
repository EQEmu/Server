CREATE TABLE `character_kick_events` (
	`Id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`Name` VARCHAR(64) NOT NULL,
	`Reason` TEXT NOT NULL,
	`Created` TIMESTAMP NOT NULL DEFAULT '',
	PRIMARY KEY (`Id`),
	INDEX `Name` (`Name`),
	INDEX `Created` (`Created`)
)
ENGINE=InnoDB
;
