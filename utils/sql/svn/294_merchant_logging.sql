ALTER TABLE `eventlog` ADD `event_nid` INT NOT NULL DEFAULT '0';
INSERT INTO `rule_values` VALUES (0, 'EventLog:RecordSellToMerchant', 'false');
INSERT INTO `rule_values` VALUES (0, 'EventLog:RecordBuyFromMerchant', 'false');
