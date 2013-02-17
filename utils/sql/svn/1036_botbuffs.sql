DROP TABLE IF EXISTS `botbuffs`;
CREATE TABLE `botbuffs` (
  `BotBuffId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BotId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpellId` int(10) unsigned NOT NULL DEFAULT '0',
  `CasterLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `DurationFormula` int(10) unsigned NOT NULL DEFAULT '0',
  `TicsRemaining` int(11) unsigned NOT NULL DEFAULT '0',
  `PoisonCounters` int(11) unsigned NOT NULL DEFAULT '0',
  `DiseaseCounters` int(11) unsigned NOT NULL DEFAULT '0',
  `CurseCounters` int(11) unsigned NOT NULL DEFAULT '0',
  `HitCount` int(10) unsigned NOT NULL DEFAULT '0',
  `MeleeRune` int(10) unsigned NOT NULL DEFAULT '0',
  `MagicRune` int(10) unsigned NOT NULL DEFAULT '0',
  `DeathSaveSuccessChance` int(10) unsigned NOT NULL DEFAULT '0',
  `CasterAARank` int(10) unsigned NOT NULL DEFAULT '0',
  `Persistent` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotBuffId`),
  KEY `FK_botbuff_1` (`BotId`),
  CONSTRAINT `FK_botbuff_1` FOREIGN KEY (`BotId`) REFERENCES `bots` (`BotID`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=latin1;