CREATE TABLE `splash_faction` (
	`id` INT(10) NOT NULL,
	`splash_id1` INT(10) NOT NULL DEFAULT 0,
	`splash_mod1` FLOAT NOT NULL DEFAULT 0,
	`splash_id2` INT(10) NOT NULL DEFAULT 0,
	`splash_mod2` FLOAT NOT NULL DEFAULT 0,
	`splash_id3` INT(10) NOT NULL DEFAULT 0,
	`splash_mod3` FLOAT NOT NULL DEFAULT 0,
	`splash_id4` INT(10) NOT NULL DEFAULT 0,
	`splash_mod4` FLOAT NOT NULL DEFAULT 0,
	`splash_id5` INT(10) NOT NULL DEFAULT 0,
	`splash_mod5` FLOAT NOT NULL DEFAULT 0,
	`splash_id6` INT(10) NOT NULL DEFAULT 0,
	`splash_mod6` FLOAT NOT NULL DEFAULT 0,
	`splash_id7` INT(10) NOT NULL DEFAULT 0,
	`splash_mod7` FLOAT NOT NULL DEFAULT 0,
	`splash_id8` INT(10) NOT NULL DEFAULT 0,
	`splash_mod8` FLOAT NOT NULL DEFAULT 0,
	`splash_id9` INT(10) NOT NULL DEFAULT 0,
	`splash_mod9` FLOAT NOT NULL DEFAULT 0,
	`splash_id10` INT(10) NOT NULL DEFAULT 0,
	`splash_mod10` FLOAT NOT NULL DEFAULT 0,
	PRIMARY KEY(`id`)
);

ALTER TABLE `npc_types` ADD `faction_amount` INT(10) DEFAULT '0';
ALTER TABLE `tasks` ADD `faction_amount` INT(10) DEFAULT '0';
