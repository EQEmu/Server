CREATE TABLE  botstances (
  BotID int(10) unsigned NOT NULL default '0',
  StanceID tinyint unsigned NOT NULL default '0',
  PRIMARY KEY  (BotID),
  CONSTRAINT FK_botstances_1 FOREIGN KEY (BotID) REFERENCES bots (BotID)
);