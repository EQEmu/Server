DROP TABLE IF EXISTS tblLoginServerAccounts;
CREATE TABLE IF NOT EXISTS tblLoginServerAccounts (
  LoginServerID integer unsigned NOT NULL auto_increment,
  AccountName varchar(30) NOT NULL,
  AccountPassword varchar(50) NOT NULL,
  AccountCreateDate timestamp default CURRENT_TIMESTAMP NOT NULL,
  AccountEmail varchar(100) NOT NULL,
  LastLoginDate datetime NOT NULL,
  LastIPAddress varchar(15) NOT NULL,
  PRIMARY KEY (LoginServerID, AccountName)
) ENGINE=InnoDB;

insert into tblLoginServerAccounts (AccountName, AccountPassword, AccountEmail, LastLoginDate, LastIPAddress) values('Admin', sha('password'), 'admin@somewhere.com', now(), '127.0.0.1');

DROP TABLE IF EXISTS tblServerListType;
CREATE TABLE IF NOT EXISTS tblServerListType (
	ServerListTypeID integer unsigned NOT NULL,
	ServerListTypeDescription varchar(20) NOT NULL,
	PRIMARY KEY (ServerListTypeID)
) ENGINE=MyISAM;

INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (1, 'Legends');
INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (2, 'Preferred');
INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (3, 'Standard');

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

INSERT INTO tblServerAdminRegistration (AccountName, AccountPassword, FirstName, LastName, Email, RegistrationDate, RegistrationIPAddr) VALUES ('Admin', 'Password', 'Tom', 'Wilson', 'Tom.Wilson@gmail.com', now(), '0.0.0.0');

DROP TABLE IF EXISTS tblWorldServerRegistration;
CREATE TABLE IF NOT EXISTS tblWorldServerRegistration (
  ServerID integer unsigned NOT NULL auto_increment,
  ServerLongName varchar(100) NOT NULL,
  ServerTagDescription varchar(50) NOT NULL DEFAULT '',
  ServerShortName varchar(25) NOT NULL,
  ServerListTypeID integer NOT NULL,
  ServerLastLoginDate datetime NULL,
  ServerLastIPAddr varchar(15) NULL,
  ServerAdminID integer NOT NULL,
  ServerTrusted integer NOT NULL,
  Note varchar(300) NULL,
  PRIMARY KEY (ServerID, ServerLongName)
) ENGINE=InnoDB;


INSERT INTO tblWorldServerRegistration (ServerLongName, ServerTagDescription, ServerShortName, ServerListTypeID, ServerLastLoginDate, ServerLastIPAddr, ServerAdminID, ServerTrusted, Note) VALUES ('My Test Server', 'A test server', 'MTST', 1, now(), '0.0.0.0', 1, 0, 'This is a note for the test server');
