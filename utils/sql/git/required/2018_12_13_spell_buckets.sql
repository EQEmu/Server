CREATE TABLE `spell_buckets` (
  `spellid` bigint(11) unsigned NOT NULL,
  `key` varchar(100) DEFAULT NULL,
  `value` text,
  PRIMARY KEY (`spellid`),
  KEY `key_index` (`key`) USING BTREE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8mb4;

INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Spells:EnableSpellBuckets', 'false', 'Enables spell buckets');
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Character:PerCharacterBucketMaxLevel', 'false', 'Enables data bucket-based max level.');