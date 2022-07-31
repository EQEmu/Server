ALTER TABLE `merchantlist_temp` ADD COLUMN `zone_id` INT(11) NOT NULL DEFAULT '0' AFTER `slot`;
ALTER TABLE `merchantlist_temp` ADD COLUMN `instance_id` INT(11) NOT NULL DEFAULT '0' AFTER `zone_id`;
ALTER TABLE `merchantlist_temp` DROP PRIMARY KEY, ADD PRIMARY KEY (`npcid`, `slot`, `zone_id`, `instance_id`);
