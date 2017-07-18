CREATE TABLE `auras` (
	`type` INT(10) NOT NULL,
	`npc_type` INT(10) NOT NULL,
	`name` VARCHAR(64) NOT NULL,
	`spell_id` INT(10) NOT NULL,
	`distance` INT(10) NOT NULL DEFAULT 60,
	`aura_type` INT(10) NOT NULL DEFAULT 1,
	`spawn_type` INT(10) NOT NULL DEFAULT 0,
	`movement` INT(10) NOT NULL DEFAULT 0,
	`duration` INT(10) NOT NULL DEFAULT 5400,
	`icon` INT(10) NOT NULL DEFAULT -1,
	`cast_time` INT(10) NOT NULL DEFAULT 0,
	PRIMARY KEY(`type`)
)

CREATE TABLE `character_auras` (
	`id` INT(10) NOT NULL,
	`slot` TINYINT(10) NOT NULL,
	`spell_id` INT(10) NOT NULL,
	PRIMARY KEY (`id`, `slot`)
);

