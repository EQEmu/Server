CREATE TABLE `raid_leaders` (
	`gid` int(4) unsigned NOT NULL,
	`rid` int(4) unsigned NOT NULL,
	`marknpc` varchar(64) NOT NULL,
	`maintank` varchar(64) NOT NULL,
	`assist` varchar(64) NOT NULL,
	`puller` varchar(64) NOT NULL,
	`leadershipaa` tinyblob NOT NULL
);
