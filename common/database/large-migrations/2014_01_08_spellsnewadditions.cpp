std::string _2014_01_08_spellsnewadditions = R"(
ALTER TABLE `spells_new` CHANGE `field200` `suspendable` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field202` `songcap` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field215` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field216` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field217` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field218` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `maxtargets` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field220` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field221` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field222` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field223` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `persistdeath` INT(11) DEFAULT '0';

)";