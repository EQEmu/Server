DROP TABLE IF EXISTS `botpetinventory`;
DROP TABLE IF EXISTS `botpetbuffs`;
DROP TABLE IF EXISTS `botpets`;

CREATE TABLE IF NOT EXISTS `botpets` (
  `BotPetsId` integer unsigned NOT NULL AUTO_INCREMENT,
  `PetId` integer unsigned NOT NULL DEFAULT '0',
  `BotId` integer unsigned NOT NULL DEFAULT '0',
  `Name` varchar(64) NULL,
  `Mana` integer NOT NULL DEFAULT '0',
  `HitPoints` integer NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotPetsId`),
  KEY `FK_botpets_1` (`BotId`),
  CONSTRAINT `FK_botpets_1` FOREIGN KEY (`BotId`) REFERENCES `bots` (`BotID`),
  CONSTRAINT `U_botpets_1` UNIQUE (`BotId`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `botpetbuffs` (
  `BotPetBuffId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BotPetsId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpellId` int(10) unsigned NOT NULL DEFAULT '0',
  `CasterLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `Duration` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotPetBuffId`),
  KEY `FK_botpetbuffs_1` (`BotPetsId`),
  CONSTRAINT `FK_botpetbuffs_1` FOREIGN KEY (`BotPetsId`) REFERENCES `botpets` (`BotPetsID`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `botpetinventory` (
  `BotPetInventoryId` integer unsigned NOT NULL AUTO_INCREMENT,
  `BotPetsId` integer unsigned NOT NULL DEFAULT '0',
  `ItemId` integer unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotPetInventoryId`),
  KEY `FK_botpetinventory_1` (`BotPetsId`),
  CONSTRAINT `FK_botpetinventory_1` FOREIGN KEY (`BotPetsId`) REFERENCES `botpets` (`BotPetsID`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;