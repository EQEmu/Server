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
  Note varchar(300) NULL,
  ServerTrusted int(11),
  PRIMARY KEY (ServerID, ServerLongName)
) ENGINE=InnoDB;
