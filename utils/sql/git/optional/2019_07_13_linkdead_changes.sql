UPDATE `rule_values` SET `rule_value`='90000' WHERE `rule_name`='Zone:ClientLinkdeadMS';
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'DisallowDuplicateAccountLogins', 'true', 'Requires account logins to be unique.');
