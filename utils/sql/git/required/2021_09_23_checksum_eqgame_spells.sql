INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('checksum_crc1_eqgame', '0', 'Client CRC64 Checksum on: eqgame.exe', '2021-09-23 14:16:27');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('checksum_crc2_skillcaps', '0', 'Client CRC64 Checksum on: SkillCaps.txt', '2021-09-23 14:16:21');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('checksum_crc3_basedata', '0', 'Client CRC64 Checksum on: BaseData.txt','2021-09-23 14:16:21');

ALTER TABLE `account`
	ADD COLUMN `checksum_crc1_eqgame` TEXT NULL AFTER `suspend_reason`,
	ADD COLUMN `checksum_crc2_skillcaps` TEXT NULL AFTER `checksum_crc1_eqgame`,
	ADD COLUMN `checksum_crc3_basedata` TEXT NULL AFTER `checksum_crc2_skillcaps`;