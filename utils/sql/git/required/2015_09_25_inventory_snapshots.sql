CREATE TABLE `inventory_snapshots` (
	`time_index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`charid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`slotid` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`itemid` INT(11) UNSIGNED NULL DEFAULT '0',
	`charges` SMALLINT(3) UNSIGNED NULL DEFAULT '0',
	`color` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`augslot1` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot2` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot3` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot4` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot5` MEDIUMINT(7) UNSIGNED NULL DEFAULT '0',
	`augslot6` MEDIUMINT(7) NOT NULL DEFAULT '0',
	`instnodrop` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	`custom_data` TEXT NULL,
	`ornamenticon` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornamentidfile` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornament_hero_model` INT(11) NOT NULL DEFAULT '0',
	PRIMARY KEY (`time_index`, `charid`, `slotid`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;

ALTER TABLE `character_data` ADD COLUMN `e_last_invsnapshot` INT(11) UNSIGNED NOT NULL DEFAULT '0';

INSERT INTO `rule_values` VALUES
(1, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(2, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(4, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(5, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(10, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(1, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(2, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(4, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(5, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(10, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(1, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(2, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(4, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(5, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(10, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(1, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries'),
(2, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries'),
(4, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries'),
(5, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries'),
(10, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries');
