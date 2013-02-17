DROP TABLE IF EXISTS `botactives`;
CREATE TABLE `botactives` (
  `ActiveBotId` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ActiveBotId`),
  KEY `FK_botactives_1` (`ActiveBotId`),
  CONSTRAINT `FK_botactives_1` FOREIGN KEY (`ActiveBotId`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;