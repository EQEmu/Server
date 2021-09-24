INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('checksum_spells', '0', 'Check the spells_us.txt CRC32 checksum | 0 = Disable', '2021-09-23 14:16:27');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('checksum_eqgame', '0', 'Client eqgame.exe CRC32 Checksum | 0 = off', '2021-09-23 14:16:21');

ALTER TABLE `account`
	ADD COLUMN `checksum_eqgame` TEXT NULL AFTER `suspend_reason`,
	ADD COLUMN `checksum_spells` TEXT NULL AFTER `checksum_eqgame`;