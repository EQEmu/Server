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
