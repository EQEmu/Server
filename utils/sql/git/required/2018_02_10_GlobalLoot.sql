ALTER TABLE `npc_types` ADD `skip_global_loot` TINYINT DEFAULT '0';
ALTER TABLE `npc_types` ADD `rare_spawn` TINYINT DEFAULT '0';

CREATE TABLE global_loot (
	id INT NOT NULL AUTO_INCREMENT,
	description varchar(255),
	loottable_id INT NOT NULL,
	enabled TINYINT NOT NULL DEFAULT 1,
	min_level INT NOT NULL DEFAULT 0,
	max_level INT NOT NULL DEFAULT 0,
	rare TINYINT NULL,
	raid TINYINT NULL,
	race MEDIUMTEXT NULL,
	class MEDIUMTEXT NULL,
	bodytype MEDIUMTEXT NULL,
	zone MEDIUMTEXT NULL,
	PRIMARY KEY (id)
);

