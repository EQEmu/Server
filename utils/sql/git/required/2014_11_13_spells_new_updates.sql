-- spells new table update
ALTER TABLE `spells_new` CHANGE `field124` `disallow_sit` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field125` `deities0` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field196` `sneaking` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field158` `effectdescnum2` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field165` `ldon_trap` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field205` `no_block` INT(11) NOT NULL DEFAULT '0';


