ALTER TABLE `account`
	ADD COLUMN `nodelay` SMALLINT(5) NOT NULL DEFAULT '-1' AFTER `suspend_reason`;