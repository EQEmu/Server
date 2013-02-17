--Obsolete if you have the latest items db.

ALTER TABLE `items` ADD `attackbonus` INT(11)  DEFAULT "0" NOT NULL AFTER `astr`;
ALTER TABLE `items` CHANGE `unknown108` `augmentrestriction` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` CHANGE `unknown107` `banedmgamt2` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` ADD `charges` INT(11)  DEFAULT "0" NOT NULL AFTER `casttime2`;
ALTER TABLE `items` CHANGE `unknown076` `combatskill` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` CHANGE `unknown077` `combatskilldmg` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` ADD `damageshield` INT(11)  DEFAULT "0" NOT NULL AFTER `damage`;
ALTER TABLE `items` ADD `dotshielding` INT(11)  DEFAULT "0" NOT NULL AFTER `damageshield`;
ALTER TABLE `items` ADD `endur` INT(11)  DEFAULT "0" NOT NULL AFTER `damageshield`;
ALTER TABLE `items` ADD `hastepercent` INT(11)  DEFAULT "0" NOT NULL AFTER `gmflag`;
ALTER TABLE `items` ADD `hpregen` INT(11)  DEFAULT "0" NOT NULL AFTER `hp`;
ALTER TABLE `items` CHANGE `unknown099` `ldonsold` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` ADD `manaregen` INT(11)  DEFAULT "0" NOT NULL AFTER `mana`;
ALTER TABLE `items` CHANGE `unknown004` `merchantprice` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` CHANGE `unknown070` `proc_rate_mod` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` ADD `unknown003` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` ADD `unknown006` INT(11)  DEFAULT "0" NOT NULL;
ALTER TABLE `items` ADD `unknown007` INT(11)  DEFAULT "0" NOT NULL;
update items set filename='' where filename='0';
update items set charmfile='' where charmfile='0';