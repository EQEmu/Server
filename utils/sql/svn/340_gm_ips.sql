INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`) VALUES (0, 'World:GMAccountIPList', 'false');

CREATE TABLE IF NOT EXISTS `gm_ips` (
  `name` varchar(64) NOT NULL,
  `account_id` int(11) NOT NULL,
  `ip_address` varchar(15) NOT NULL,
  UNIQUE KEY `account_id` (`account_id`,`ip_address`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;