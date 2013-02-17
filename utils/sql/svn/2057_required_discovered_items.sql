CREATE TABLE `discovered_items` (
  `item_id` int(11) unsigned NOT NULL DEFAULT '0',
  `char_name` varchar(64) NOT NULL DEFAULT '',
  `discovered_date` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_id`)
);
