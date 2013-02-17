DROP TABLE IF EXISTS tblLoginServerAccounts;
CREATE TABLE tblLoginServerAccounts (
  LoginServerID SERIAL,
  AccountName text NOT NULL,
  AccountPassword text NOT NULL,
  AccountCreateDate date NOT NULL,
  AccountEmail text NOT NULL,
  LastLoginDate date NOT NULL,
  LastIPAddress text NOT NULL,
  PRIMARY KEY(LoginServerID, AccountName)
);

insert into tblLoginServerAccounts (AccountName, AccountPassword, AccountEmail, AccountCreateDate, LastLoginDate, LastIPAddress) values('Admin', '5baa61e4c9b93f3f0682250b6cf8331b7ee68fd8', 'admin@somewhere.com', current_date, current_date, '127.0.0.1');

DROP TABLE IF EXISTS tblServerListType;
CREATE TABLE tblServerListType (
	ServerListTypeID integer NOT NULL,
	CHECK (ServerListTypeID >= 0),
	ServerListTypeDescription text NOT NULL,
	PRIMARY KEY (ServerListTypeID)
);

INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (1, 'Legends');
INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (2, 'Preferred');
INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (3, 'Standard');

DROP TABLE IF EXISTS tblServerAdminRegistration;
CREATE TABLE tblServerAdminRegistration (
	ServerAdminID SERIAL,
	AccountName text NOT NULL,
	AccountPassword text NOT NULL,
	FirstName text NOT NULL,
	LastName text NOT NULL,
	Email text NOT NULL,
	RegistrationDate date NOT NULL,
	RegistrationIPAddr text NOT NULL,
	PRIMARY KEY (ServerAdminID, Email)
);

INSERT INTO tblServerAdminRegistration (AccountName, AccountPassword, FirstName, LastName, Email, RegistrationDate, RegistrationIPAddr) VALUES ('Admin', 'Password', 'Tom', 'Wilson', 'Tom.Wilson@gmail.com', current_date, '0.0.0.0');

DROP TABLE IF EXISTS tblWorldServerRegistration;
CREATE TABLE tblWorldServerRegistration (
  ServerID SERIAL,
  ServerLongName text NOT NULL,
  ServerTagDescription text NOT NULL,
  ServerShortName text NOT NULL,
  ServerListTypeID integer NOT NULL,
  ServerLastLoginDate date NULL,
  ServerLastIPAddr text NOT NULL,
  ServerAdminID integer NOT NULL,
  ServerTrusted integer NOT NULL,
  Note text NOT NULL,
  PRIMARY KEY (ServerID, ServerLongName)
);

INSERT INTO tblWorldServerRegistration (ServerLongName, ServerTagDescription, ServerShortName, ServerListTypeID, ServerLastLoginDate, ServerLastIPAddr, ServerAdminID, ServerTrusted, Note) VALUES ('My Test Server', 'A test server', 'MTST', 1, current_date, '0.0.0.0', 1, 0, 'This is a note for the test server');