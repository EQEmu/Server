RENAME TABLE `Banned_IPs` TO `Banned_IPs_`;

CREATE TABLE `banned_ips` (PRIMARY KEY (`ip_address`)) SELECT `ip_address`, `notes` FROM `Banned_IPs_`;

DROP TABLE IF EXISTS `Banned_IPs_`;
