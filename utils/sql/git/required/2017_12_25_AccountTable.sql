ALTER TABLE `account`
	DROP INDEX `name`,
	DROP INDEX `lsaccount_id`;
	
ALTER TABLE `account`
	ADD COLUMN `ls_id` VARCHAR(64) NULL DEFAULT 'eqemu' AFTER `status`;
	
ALTER TABLE `account`
	ADD UNIQUE INDEX `name_ls_id` (`name`, `ls_id`),
	ADD UNIQUE INDEX `ls_id_lsaccount_id` (`ls_id`, `lsaccount_id`);
	