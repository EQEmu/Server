DROP TABLE IF EXISTS tblServerListType;
CREATE TABLE IF NOT EXISTS tblServerListType (
	ServerListTypeID integer unsigned NOT NULL,
	ServerListTypeDescription varchar(20) NOT NULL,
	PRIMARY KEY (ServerListTypeID)
) ENGINE=MyISAM;

INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (1, 'Legends');
INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (2, 'Preferred');
INSERT INTO tblServerListType (ServerListTypeID, ServerListTypeDescription) VALUES (3, 'Standard');