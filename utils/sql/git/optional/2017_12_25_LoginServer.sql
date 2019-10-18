ALTER TABLE `tblloginserveraccounts`
	ADD COLUMN `AccountLoginserver` VARCHAR(64) NULL AFTER `LastIPAddress`;
	
ALTER TABLE `tblloginserveraccounts`
	CHANGE COLUMN `AccountLoginserver` `AccountLoginserver` VARCHAR(64) NOT NULL DEFAULT 'eqemu' AFTER `LoginServerID`,
	DROP PRIMARY KEY,
	ADD PRIMARY KEY (`LoginServerID`, `AccountLoginserver`);
	
ALTER TABLE `tblloginserveraccounts`
	ADD UNIQUE INDEX `AccountLoginserver_AccountName` (`AccountLoginserver`, `AccountName`);
	
ALTER TABLE `tblloginserveraccounts`
	ALTER `LoginServerID` DROP DEFAULT;
ALTER TABLE `tblloginserveraccounts`
	CHANGE COLUMN `LoginServerID` `LoginServerID` INT(10) UNSIGNED NOT NULL FIRST;
	
ALTER TABLE `tblloginserveraccounts`
	ALTER `AccountPassword` DROP DEFAULT;
ALTER TABLE `tblloginserveraccounts`
	CHANGE COLUMN `AccountPassword` `AccountPassword` TEXT NOT NULL AFTER `AccountName`;
	