CREATE TABLE `character_pvp_entries` (
	`id` INT(10) NOT NULL AUTO_INCREMENT,
	`killer_id` INT(10) NOT NULL,
	`killer_level` INT(10) NOT NULL,
	`victim_id` INT(10) NOT NULL,
	`victim_level` INT(10) NOT NULL,	
	`zone` INT(10) NOT NULL,
	`points` INT(10) NOT NULL,
	`timestamp` INT(10) NOT NULL,
	PRIMARY KEY (`id`)
);
