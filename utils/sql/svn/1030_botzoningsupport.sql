DROP TABLE IF EXISTS `botactives`;

ALTER TABLE `group_id` DROP PRIMARY KEY, ADD PRIMARY KEY  USING BTREE(`groupid`, `charid`, `name`);