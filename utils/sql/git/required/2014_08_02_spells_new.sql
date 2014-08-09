-- spells new talbe update
ALTER TABLE `spells_new` CHANGE `NotOutofCombat` `InCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `NotInCombat` `OutofCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field201` `viral_range` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field218` `aemaxtargets` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE  `spells_new` ADD  `field225` int( 11 ) NOT NULL DEFAULT  '0' AFTER `persistdeath`;
ALTER TABLE  `spells_new` ADD  `field226` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field225`;
ALTER TABLE  `spells_new` ADD  `min_dist` float( 0 ) NOT NULL DEFAULT  '0' AFTER `field226`;
ALTER TABLE  `spells_new` ADD  `min_dist_mod` float( 0 ) NOT NULL DEFAULT  '0' AFTER `min_dist`;
ALTER TABLE  `spells_new` ADD  `max_dist` float( 0 ) NOT NULL DEFAULT  '0' AFTER `min_dist_mod`;
ALTER TABLE  `spells_new` ADD  `max_dist_mod` float( 0 ) NOT NULL DEFAULT  '0' AFTER `max_dist`;
ALTER TABLE  `spells_new` ADD  `min_range` int( 11 ) NOT NULL DEFAULT  '0' AFTER `max_dist_mod`;
ALTER TABLE  `spells_new` ADD  `field232` int( 11 ) NOT NULL DEFAULT  '0' AFTER `min_range`;
ALTER TABLE  `spells_new` ADD  `field233` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field232`;
ALTER TABLE  `spells_new` ADD  `field234` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field233`;
ALTER TABLE  `spells_new` ADD  `field235` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field234`;
ALTER TABLE  `spells_new` ADD  `field236` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field235`;

