CREATE TABLE bottimers (
  BotID int(10) unsigned NOT NULL default '0',
  TimerID int(10) unsigned NOT NULL default '0',
  Value int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (BotID),
  CONSTRAINT FK_bottimers_1 FOREIGN KEY (BotID) REFERENCES bots (BotID)
) 