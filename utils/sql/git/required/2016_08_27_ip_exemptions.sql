-- IP Exemptions table structure
DROP TABLE IF EXISTS `ip_exemptions`;
CREATE TABLE `ip_exemptions` (
  `exemption_id` int(11) NOT NULL AUTO_INCREMENT,
  `exemption_ip` varchar(255) DEFAULT NULL,
  `exemption_amount` int(11) DEFAULT NULL,
  PRIMARY KEY (`exemption_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

-- Rule Value Entry, Default to false
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES ('1', 'World:EnableIPExemptions', 'false', 'notation');

-- Logging Category Entry
INSERT INTO `logsys_categories` (`log_category_id`, `log_category_description`, `log_to_console`, `log_to_file`, `log_to_gmsay`) VALUES ('44', 'Client Login', '1', '1', '1');