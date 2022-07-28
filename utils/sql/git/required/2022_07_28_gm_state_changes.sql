ALTER TABLE `account`
	ADD COLUMN `gminvul` TINYINT(4) NULL DEFAULT '0' AFTER `ip_exemption_multiplier`,
	ADD COLUMN `flymode` TINYINT(4) NULL DEFAULT '0' AFTER `gminvul`,
	ADD COLUMN `ignore_tells` TINYINT(4) NULL DEFAULT '0' AFTER `flymode`;
