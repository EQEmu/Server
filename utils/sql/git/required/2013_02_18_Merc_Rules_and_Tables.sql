INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Mercs:ResurrectRadius', '50', 'Determines the distance from which a healer merc will attempt to resurrect a corpse');

DROP TABLE IF EXISTS mercsbuffs;
DROP TABLE IF EXISTS mercs;

CREATE TABLE  mercs (
	MercID 				int(10) unsigned NOT NULL AUTO_INCREMENT,
	OwnerCharacterID 	int(10) unsigned NOT NULL,
	Slot 	    		tinyint(1) unsigned NOT NULL DEFAULT '0',
	Name 				varchar(64) NOT NULL,
	TemplateID			int(10) unsigned NOT NULL DEFAULT '0',
	SuspendedTime		int(11) unsigned NOT NULL DEFAULT '0',
	IsSuspended			tinyint(1) unsigned NOT NULL default '0',
	TimerRemaining		int(11) unsigned NOT NULL DEFAULT '0',
	Gender 				tinyint unsigned NOT NULL DEFAULT '0',
	StanceID			tinyint unsigned NOT NULL DEFAULT '0',
	HP 					int(11) unsigned NOT NULL DEFAULT '0',
	Mana 				int(11) unsigned NOT NULL DEFAULT '0',
	Endurance			int(11) unsigned NOT NULL DEFAULT '0',
	Face 				int(10) unsigned NOT NULL DEFAULT '1',
	LuclinHairStyle 	int(10) unsigned NOT NULL DEFAULT '1',
	LuclinHairColor 	int(10) unsigned NOT NULL DEFAULT '1',
	LuclinEyeColor 		int(10) unsigned NOT NULL DEFAULT '1',
	LuclinEyeColor2 	int(10) unsigned NOT NULL DEFAULT '1',
	LuclinBeardColor 	int(10) unsigned NOT NULL DEFAULT '1',
	LuclinBeard 		int(10) unsigned NOT NULL DEFAULT '0',
	DrakkinHeritage 	int(10) unsigned NOT NULL DEFAULT '0',
	DrakkinTattoo 		int(10) unsigned NOT NULL DEFAULT '0',
	DrakkinDetails 		int(10) unsigned NOT NULL DEFAULT '0',
	PRIMARY KEY (MercID)
);

CREATE TABLE mercbuffs (
	MercBuffId        	int(10) unsigned NOT NULL auto_increment,
	MercId            	int(10) unsigned NOT NULL default '0',
	SpellId           	int(10) unsigned NOT NULL default '0',
	CasterLevel       	int(10) unsigned NOT NULL default '0',
	DurationFormula   	int(10) unsigned NOT NULL default '0',
	TicsRemaining     	int(11) unsigned NOT NULL default '0',
	PoisonCounters    	int(11) unsigned NOT NULL default '0',
	DiseaseCounters   	int(11) unsigned NOT NULL default '0',
	CurseCounters     	int(11) unsigned NOT NULL default '0',
	CorruptionCounters 	int(11) unsigned NOT NULL default '0',
	HitCount          	int(10) unsigned NOT NULL default '0',
	MeleeRune         	int(10) unsigned NOT NULL default '0',
	MagicRune         	int(10) unsigned NOT NULL default '0',
	DeathSaveSuccessChance int(10) unsigned NOT NULL default '0',
	CasterAARank      	int(10) unsigned NOT NULL default '0',
	Persistent        	tinyint(1) NOT NULL default '0',
	PRIMARY KEY  (MercBuffId),
	KEY FK_mercbuff_1 (MercId),
	CONSTRAINT FK_mercbuff_1 FOREIGN KEY (MercId) REFERENCES mercs (MercID)
);