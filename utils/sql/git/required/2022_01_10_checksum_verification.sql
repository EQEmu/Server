INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_eqgame', '0', 'Client CRC64 Checksum on: eqgame.exe', '2021-09-23 14:16:27');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_skillcaps', '0', 'Client CRC64 Checksum on: SkillCaps.txt', '2021-09-23 14:16:21');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_basedata', '0', 'Client CRC64 Checksum on: BaseData.txt','2021-09-23 14:16:21');

ALTER TABLE `account`
	ADD COLUMN `crc_eqgame` TEXT NULL AFTER `suspend_reason`,
	ADD COLUMN `crc_skillcaps` TEXT NULL AFTER `crc_eqgame`,
	ADD COLUMN `crc_basedata` TEXT NULL AFTER `crc_skillcaps`;

ALTER TABLE `account` CHANGE `suspendeduntil` `suspendeduntil` datetime  NULL  COMMENT '';
