DROP TABLE IF EXISTS tblServerAdminRegistration;
CREATE TABLE IF NOT EXISTS tblServerAdminRegistration (
	ServerAdminID integer unsigned NOT NULL auto_increment,
	AccountName varchar(30) NOT NULL,
	AccountPassword varchar(30) NOT NULL,
	FirstName varchar(40) NOT NULL,
	LastName varchar(50) NOT NULL,
	Email varchar(100) NULL,
	RegistrationDate datetime NOT NULL,
	RegistrationIPAddr varchar(15) NOT NULL,
	PRIMARY KEY (ServerAdminID, Email)
) ENGINE=MyISAM;