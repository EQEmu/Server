CREATE TABLE `bug_reports` (
	`id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`zone` VARCHAR(32) NOT NULL DEFAULT 'Unknown',
	`client_version_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`client_version_name` VARCHAR(24) NOT NULL DEFAULT 'Unknown',
	`account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`character_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`character_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`reporter_spoof` TINYINT(1) NOT NULL DEFAULT '1',
	`category_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`category_name` VARCHAR(64) NOT NULL DEFAULT 'Other',
	`reporter_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`ui_path` VARCHAR(128) NOT NULL DEFAULT 'Unknown',
	`pos_x` FLOAT NOT NULL DEFAULT '0',
	`pos_y` FLOAT NOT NULL DEFAULT '0',
	`pos_z` FLOAT NOT NULL DEFAULT '0',
	`heading` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`time_played` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`target_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`target_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`optional_info_mask` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`_can_duplicate` TINYINT(1) NOT NULL DEFAULT '0',
	`_crash_bug` TINYINT(1) NOT NULL DEFAULT '0',
	`_target_info` TINYINT(1) NOT NULL DEFAULT '0',
	`_character_flags` TINYINT(1) NOT NULL DEFAULT '0',
	`_unknown_value` TINYINT(1) NOT NULL DEFAULT '0',
	`bug_report` VARCHAR(1024) NOT NULL DEFAULT '',
	`system_info` VARCHAR(1024) NOT NULL DEFAULT '',
	`report_datetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
	`bug_status` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`last_review` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	`last_reviewer` VARCHAR(64) NOT NULL DEFAULT 'None',
	`reviewer_notes` VARCHAR(1024) NOT NULL DEFAULT '',
	PRIMARY KEY (`id`),
	UNIQUE INDEX `id` (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES
(1, 'Bugs:ReportingSystemActive', 'true', 'Activates bug reporting'),
(1, 'Bugs:UseOldReportingMethod', 'true', 'Forces the use of the old bug reporting system'),
(1, 'Bugs:DumpTargetEntity', 'false', 'Dumps the target entity, if one is provided');
