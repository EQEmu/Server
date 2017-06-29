ALTER TABLE `rule_values`
MODIFY COLUMN `rule_value`  varchar(30) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `rule_name`;
