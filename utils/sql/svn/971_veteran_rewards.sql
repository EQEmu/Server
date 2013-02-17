CREATE TABLE `veteran_reward_templates` (
  `claim_id` int(10) unsigned NOT NULL,
  `name` varchar(64) NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  `charges` smallint(5) unsigned NOT NULL,
  `reward_slot` tinyint(3) unsigned NOT NULL,
  UNIQUE KEY `claim_reward` (`claim_id`,`reward_slot`),
  KEY `claim_id` (`claim_id`)
) ENGINE=MyISAM;

CREATE TABLE `account_rewards` (
  `account_id` int(10) unsigned NOT NULL,
  `reward_id` int(10) unsigned NOT NULL,
  `amount` int(10) unsigned NOT NULL,
  UNIQUE KEY `account_reward` (`account_id`,`reward_id`),
  KEY `account_id` (`account_id`)
) ENGINE=InnoDB;