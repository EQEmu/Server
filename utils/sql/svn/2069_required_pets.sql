ALTER TABLE `pets` DROP PRIMARY KEY;
ALTER TABLE `pets` ADD COLUMN `petpower` int NOT NULL default '0' AFTER `type`, ADD COLUMN `petcontrol` tinyint NOT NULL default '0', ADD COLUMN `petnaming` tinyint NOT NULL default '0', ADD COLUMN `monsterflag` tinyint NOT NULL default '0', ADD COLUMN `equipmentset` int NOT NULL default -1, ADD PRIMARY KEY (`type`, `petpower`);

UPDATE `pets` SET `monsterflag` = 1 where `type` like 'MonsterSum%';
UPDATE `pets` SET `petcontrol` = 2 where `type` like 'SumAir%' or `type` like 'SumEarth%' or `type` like 'SumFire%' or `type` like 'SumWater%' or `type` = 'SumMageMultiElement' or `type` like 'BLPet%' or `type` like 'skel_%' or `type` like 'MonsterSum%' or `type` = 'DruidPet' or `type` like 'SpiritWolf%' ;
UPDATE `pets` SET `petcontrol` = 1 where `type` like 'Animation%';
UPDATE `pets` SET `petnaming` = 2 where `type` like 'BLPet%';
UPDATE `pets` SET `petnaming` = 3 where `type` like 'SumAir%' or `type` like 'SumEarth%' or `type` like 'SumFire%' or `type` like 'SumWater%' or `type` = 'SumMageMultiElement' or `type` like 'Animation%' or `type` like 'skel_%' or `type` like 'MonsterSum%' or `type` = 'DruidPet' or `type` like 'SpiritWolf%';
UPDATE `pets` SET `petnaming` = 1 where `type` rlike 'familiar';

/* Equipment sets can have a name to make it easier to manage them. */
CREATE TABLE `pets_equipmentset` (
	`set_id` int NOT NULL PRIMARY KEY,
	`setname` varchar(30) NOT NULL default '',
	`nested_set` int NOT NULL default -1
) ENGINE=MyISAM;

CREATE TABLE `pets_equipmentset_entries` (
	`set_id` int NOT NULL,
	`slot` int NOT NULL,
	`item_id` int NOT NULL,
	PRIMARY KEY (`set_id`, `slot`)
) ENGINE=MyISAM;


CREATE TABLE `character_pet_info` (
	`char_id` int NOT NULL,
	`pet` int NOT NULL,
	`petname` varchar(64) NOT NULL default '',
	`petpower` int NOT NULL default 0,
	`spell_id` int NOT NULL default 0,
	`hp` int NOT NULL default 0,
	`mana` int NOT NULL default 0,
	PRIMARY KEY (`char_id`, `pet`)
) ENGINE=InnoDB;

CREATE TABLE `character_pet_buffs` (
	`char_id` int NOT NULL,
	`pet` int NOT NULL,
	`slot` int NOT NULL,
	`spell_id` int NOT NULL,
	`caster_level` tinyint NOT NULL default 0,
	`castername` varchar(64) NOT NULL default '',
	`ticsremaining` int NOT NULL default 0,
	`counters` int NOT NULL default 0,
	`numhits` int NOT NULL default 0,
	`rune` int NOT NULL default 0,
	PRIMARY KEY (`char_id`, `pet`, `slot`)
) ENGINE=InnoDB;

CREATE TABLE `character_pet_inventory` (
	`char_id` int NOT NULL,
	`pet` int NOT NULL,
	`slot` int NOT NULL,
	`item_id` int NOT NULL,
	PRIMARY KEY (`char_id`, `pet`, `slot`)
) ENGINE=InnoDB;