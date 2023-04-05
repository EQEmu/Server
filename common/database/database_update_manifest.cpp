std::vector <ManifestEntry> manifest_entries = {
	ManifestEntry{
		.version = 9000,
		.description = "2013_02_18_merc_rules_and_tables.sql",
		.check = "SELECT * FROM `rule_values` WHERE `rule_name` LIKE '%Mercs:ResurrectRadius%'",
		.condition = "empty",
		.match = "",
		.sql = R"(
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
)",
	},
	ManifestEntry{
		.version = 9001,
		.description = "2013_02_25_impr_ht_lt.sql",
		.check = "SHOW TABLES LIKE 'merc_inventory'",
		.condition = "empty",
		.match = "",
		.sql = R"(
/* SK AA Touch of the Wicked should reduce reuse timers for */
/* Improved Harm Touch & Leech Touch as well as regular HT */
update aa_actions set redux_aa=596, redux_rate=17 where aaid=207;
update aa_actions set redux_aa=596, redux_rate=17 where aaid=208;

)",
	},
	ManifestEntry{
		.version = 9002,
		.description = "2013_03_1_merc_rules_and_equipment.sql",
		.check = "SHOW TABLES LIKE 'merc_inventory'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Mercs:ChargeMercPurchaseCost', 'false', 'Turns Mercenary purchase costs on or off.');
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Mercs:ChargeMercUpkeepCost', 'false', 'Turns Mercenary upkeep costs on or off.');

UPDATE merc_stats SET spellscale = 100, healscale = 100;

REPLACE INTO items (`id`, `minstatus`, `Name`, `aagi`, `ac`, `accuracy`, `acha`, `adex`, `aint`, `artifactflag`, `asta`, `astr`, `attack`, `augrestrict`, `augslot1type`, `augslot1visible`, `augslot2type`, `augslot2visible`, `augslot3type`, `augslot3visible`, `augslot4type`, `augslot4visible`, `augslot5type`, `augslot5visible`, `augtype`, `avoidance`, `awis`, `bagsize`, `bagslots`, `bagtype`, `bagwr`, `banedmgamt`, `banedmgraceamt`, `banedmgbody`, `banedmgrace`, `bardtype`, `bardvalue`, `book`, `casttime`, `casttime_`, `charmfile`, `charmfileid`, `classes`, `color`, `combateffects`, `extradmgskill`, `extradmgamt`, `price`, `cr`, `damage`, `damageshield`, `deity`, `delay`, `augdistiller`, `dotshielding`, `dr`, `clicktype`, `clicklevel2`, `elemdmgtype`, `elemdmgamt`, `endur`, `factionamt1`, `factionamt2`, `factionamt3`, `factionamt4`, `factionmod1`, `factionmod2`, `factionmod3`, `factionmod4`, `filename`, `focuseffect`, `fr`, `fvnodrop`, `haste`, `clicklevel`, `hp`, `regen`, `icon`, `idfile`, `itemclass`, `itemtype`, `ldonprice`, `ldontheme`, `ldonsold`, `light`, `lore`, `loregroup`, `magic`, `mana`, `manaregen`, `enduranceregen`, `material`, `maxcharges`, `mr`, `nodrop`, `norent`, `pendingloreflag`, `pr`, `procrate`, `races`, `range`, `reclevel`, `recskill`, `reqlevel`, `sellrate`, `shielding`, `size`, `skillmodtype`, `skillmodvalue`, `slots`, `clickeffect`, `spellshield`, `strikethrough`, `stunresist`, `summonedflag`, `tradeskills`, `favor`, `weight`, `UNK012`, `UNK013`, `benefitflag`, `UNK054`, `UNK059`, `booktype`, `recastdelay`, `recasttype`, `guildfavor`, `UNK123`, `UNK124`, `attuneable`, `nopet`, `updated`, `comment`, `UNK127`, `pointtype`, `potionbelt`, `potionbeltslots`, `stacksize`, `notransfer`, `stackable`, `UNK134`, `UNK137`, `proceffect`, `proctype`, `proclevel2`, `proclevel`, `UNK142`, `worneffect`, `worntype`, `wornlevel2`, `wornlevel`, `UNK147`, `focustype`, `focuslevel2`, `focuslevel`, `UNK152`, `scrolleffect`, `scrolltype`, `scrolllevel2`, `scrolllevel`, `UNK157`, `serialized`, `verified`, `serialization`, `source`, `UNK033`, `lorefile`, `UNK014`, `svcorruption`, `UNK038`, `UNK060`, `augslot1unk2`, `augslot2unk2`, `augslot3unk2`, `augslot4unk2`, `augslot5unk2`, `UNK120`, `UNK121`, `questitemflag`, `UNK132`, `clickunk5`, `clickunk6`, `clickunk7`, `procunk1`, `procunk2`, `procunk3`, `procunk4`, `procunk6`, `procunk7`, `wornunk1`, `wornunk2`, `wornunk3`, `wornunk4`, `wornunk5`, `wornunk6`, `wornunk7`, `focusunk1`, `focusunk2`, `focusunk3`, `focusunk4`, `focusunk5`, `focusunk6`, `focusunk7`, `scrollunk1`, `scrollunk2`, `scrollunk3`, `scrollunk4`, `scrollunk5`, `scrollunk6`, `scrollunk7`, `UNK193`, `purity`, `evolvinglevel`, `clickname`, `procname`, `wornname`, `focusname`, `scrollname`, `dsmitigation`, `heroic_str`, `heroic_int`, `heroic_wis`, `heroic_agi`, `heroic_dex`, `heroic_sta`, `heroic_cha`, `heroic_pr`, `heroic_dr`, `heroic_fr`, `heroic_cr`, `heroic_mr`, `heroic_svcorrup`, `healamt`, `spelldmg`, `clairvoyance`, `backstabdmg`, `created`, `elitematerial`, `ldonsellbackrate`, `scriptfileid`, `expendablearrow`, `powersourcecapacity`, `bardeffect`, `bardeffecttype`, `bardlevel2`, `bardlevel`, `bardunk1`, `bardunk2`, `bardunk3`, `bardunk4`, `bardunk5`, `bardname`, `bardunk7`, `UNK214`, `UNK219`, `UNK220`, `UNK221`, `UNK222`, `UNK223`, `UNK224`, `UNK225`, `UNK226`, `UNK227`, `UNK228`, `UNK229`, `UNK230`, `UNK231`, `UNK232`, `UNK233`, `UNK234`, `UNK235`, `UNK236`, `UNK237`, `UNK238`, `UNK239`, `UNK240`, `UNK241`, `UNK242`)
VALUES
	(51735, 0, 'MRC - CT Focus - Tier I - 5%'   , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17158, 0, 1, 0, 0, 0, 0, 1983, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51736, 0, 'MRC - CT Focus - Tier II - 10%' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17159, 0, 1, 0, 0, 0, 0, 1983, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51737, 0, 'MRC - CT Focus - Tier III - 15%', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17160, 0, 1, 0, 0, 0, 0, 1983, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51738, 0, 'MRC - CT Focus - Tier IV - 20%' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17161, 0, 1, 0, 0, 0, 0, 1983, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51739, 0, 'MRC - CT Focus - Tier V - 25%'  , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17162, 0, 1, 0, 0, 0, 0, 2001, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-09 18:53:11', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

	(51740, 0, 'MRC - ID Focus - Tier I - 10%'  , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17163, 0, 1, 0, 0, 0, 0, 1989, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51741, 0, 'MRC - ID Focus - Tier II - 20%' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17164, 0, 1, 0, 0, 0, 0, 1989, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51742, 0, 'MRC - ID Focus - Tier III - 30%', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17165, 0, 1, 0, 0, 0, 0, 1989, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51743, 0, 'MRC - ID Focus - Tier IV - 40%' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17166, 0, 1, 0, 0, 0, 0, 1989, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51744, 0, 'MRC - ID Focus - Tier V - 50%'  , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17167, 0, 1, 0, 0, 0, 0, 2002, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

	(51745, 0, 'MRC - IH Focus - Tier I - 15%'  , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17168, 0, 1, 0, 0, 0, 0, 1941, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51746, 0, 'MRC - IH Focus - Tier II - 30%' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17169, 0, 1, 0, 0, 0, 0, 1941, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51747, 0, 'MRC - IH Focus - Tier III - 45%', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17170, 0, 1, 0, 0, 0, 0, 1941, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51748, 0, 'MRC - IH Focus - Tier IV - 60%' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17171, 0, 1, 0, 0, 0, 0, 1941, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51749, 0, 'MRC - IH Focus - Tier V - 75%'  , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17172, 0, 1, 0, 0, 0, 0, 1994, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

	(51750, 0, 'MRC - MC Focus - Tier I - 5%'   , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17173, 0, 1, 0, 0, 0, 0, 1965, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51751, 0, 'MRC - MC Focus - Tier II - 10%' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17174, 0, 1, 0, 0, 0, 0, 1965, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51752, 0, 'MRC - MC Focus - Tier III - 15%', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17175, 0, 1, 0, 0, 0, 0, 1965, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51753, 0, 'MRC - MC Focus - Tier IV - 20%' , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17176, 0, 1, 0, 0, 0, 0, 1965, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(51754, 0, 'MRC - MC Focus - Tier V - 25%'  , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '0', 65535, 4278190080, '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 17177, 0, 1, 0, 0, 0, 0, 1998, 'IT63', 0, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 65535, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2011-10-03 10:49:31', '', 0, 0, 0, 0, 1, 1, 0, '', 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 6, 0, 0, 0, -1, 0, 0, 0, 0, '', '2009-05-03 03:13:17', '', '13THFLOOR', 0, '', 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, '0000000000000000000', 0, '', -1, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, '', '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '2008-11-23 02:58:34', 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, '', -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

ALTER TABLE mercbuffs RENAME TO  merc_buffs;

DROP TABLE IF EXISTS merc_inventory;

CREATE TABLE merc_inventory (
	merc_inventory_id      	int(10) unsigned NOT NULL auto_increment,
	merc_subtype_id       	int(10) unsigned NOT NULL default '0',
	item_id           	int(11) unsigned NOT NULL default '0',
	min_level       	int(10) unsigned NOT NULL default '0',
	max_level   		int(10) unsigned NOT NULL default '0',
	PRIMARY KEY  (merc_inventory_id),
	KEY FK_merc_inventory_1 (merc_subtype_id),
	CONSTRAINT FK_merc_inventory_1 FOREIGN KEY (merc_subtype_id) REFERENCES merc_subtypes (merc_subtype_id)
);

INSERT INTO merc_inventory (merc_subtype_id, item_id, min_level, max_level) VALUES
(6, 51735, 1, 85),
(7, 51736, 1, 85),
(8, 51737, 1, 85),
(9, 51738, 1, 85),
(10, 51739, 1, 85),
(6, 51740, 1, 85),
(7, 51741, 1, 85),
(8, 51742, 1, 85),
(9, 51743, 1, 85),
(10, 51744, 1, 85),
(6, 51745, 1, 85),
(7, 51746, 1, 85),
(8, 51747, 1, 85),
(9, 51748, 1, 85),
(10, 51749, 1, 85),
(6, 51750, 1, 85),
(7, 51751, 1, 85),
(8, 51752, 1, 85),
(9, 51753, 1, 85),
(10, 51754, 1, 85),
(16, 51735, 1, 85),
(17, 51736, 1, 85),
(18, 51737, 1, 85),
(19, 51738, 1, 85),
(20, 51739, 1, 85),
(10, 51739, 1, 85),
(16, 51740, 1, 85),
(17, 51741, 1, 85),
(18, 51742, 1, 85),
(19, 51743, 1, 85),
(20, 51744, 1, 85),
(16, 51745, 1, 85),
(17, 51746, 1, 85),
(18, 51747, 1, 85),
(19, 51748, 1, 85),
(20, 51749, 1, 85),
(16, 51750, 1, 85),
(17, 51751, 1, 85),
(18, 51752, 1, 85),
(19, 51753, 1, 85),
(20, 51754, 1, 85);
)",
	},
	ManifestEntry{
		.version = 9005,
		.description = "2013_04_08_salvage.sql",
		.check = "SHOW COLUMNS FROM `tradeskill_recipe_entries` LIKE 'salvagecount'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Add row to tre
ALTER TABLE `tradeskill_recipe_entries` ADD `salvagecount` tinyint(2) DEFAULT '0' NOT NULL AFTER `componentcount`;

-- Fix level req on Salvage
UPDATE `altadv_vars` SET `level_inc` = '5' WHERE `skill_id` = '997';

-- Set aa_effects for Salvage
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2374', '997', '1', '313', '5', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2375', '998', '1', '313', '15', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2376', '999', '1', '313', '25', '0');


)",
	},
	ManifestEntry{
		.version = 9006,
		.description = "2013_05_05_account_flags.sql",
		.check = "SHOW TABLES LIKE 'account_flags'",
		.condition = "empty",
		.match = "",
		.sql = R"(
--
-- Table structure for table `account_flags`
--

CREATE TABLE IF NOT EXISTS `account_flags` (
  `p_accid` int(10) unsigned NOT NULL,
  `p_flag` varchar(50) NOT NULL,
  `p_value` varchar(80) NOT NULL,
  PRIMARY KEY (`p_accid`,`p_flag`),
  KEY `p_accid` (`p_accid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9007,
		.description = "2013_05_05_item_tick.sql",
		.check = "SHOW TABLES LIKE 'item_tick'",
		.condition = "empty",
		.match = "",
		.sql = R"(
--
-- Table structure for table `item_tick`
--

CREATE TABLE IF NOT EXISTS `item_tick` (
  `it_itemid` int(11) NOT NULL,
  `it_chance` int(11) NOT NULL,
  `it_level` int(11) NOT NULL,
  `it_id` int(11) NOT NULL AUTO_INCREMENT,
  `it_qglobal` varchar(50) NOT NULL,
  `it_bagslot` tinyint(4) NOT NULL,
  PRIMARY KEY (`it_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9008,
		.description = "2013_07_11_npc_special_abilities.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'special_abilities'",
		.condition = "empty",
		.match = "",
		.sql =R"(
ALTER TABLE `npc_types`  ADD COLUMN `special_abilities` TEXT NULL AFTER `npcspecialattks`;
ALTER TABLE `npc_types` MODIFY COLUMN `special_abilities` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL;

UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "1,1^") WHERE npcspecialattks LIKE BINARY '%S%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "2,1^") WHERE npcspecialattks LIKE BINARY '%E%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "3,1^") WHERE npcspecialattks LIKE BINARY '%R%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "4,1^") WHERE npcspecialattks LIKE BINARY '%r%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "5,1^") WHERE npcspecialattks LIKE BINARY '%F%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "6,1^") WHERE npcspecialattks LIKE BINARY '%T%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "7,1^") WHERE npcspecialattks LIKE BINARY '%Q%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "8,1^") WHERE npcspecialattks LIKE BINARY '%L%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "9,1^") WHERE npcspecialattks LIKE BINARY '%b%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "10,1^") WHERE npcspecialattks LIKE BINARY '%m%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "11,1^") WHERE npcspecialattks LIKE BINARY '%Y%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "12,1^") WHERE npcspecialattks LIKE BINARY '%U%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "13,1^") WHERE npcspecialattks LIKE BINARY '%M%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "14,1^") WHERE npcspecialattks LIKE BINARY '%C%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "15,1^") WHERE npcspecialattks LIKE BINARY '%N%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "16,1^") WHERE npcspecialattks LIKE BINARY '%I%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "17,1^") WHERE npcspecialattks LIKE BINARY '%D%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "18,1^") WHERE npcspecialattks LIKE BINARY '%K%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "19,1^") WHERE npcspecialattks LIKE BINARY '%A%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "20,1^") WHERE npcspecialattks LIKE BINARY '%B%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "21,1^") WHERE npcspecialattks LIKE BINARY '%f%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "22,1^") WHERE npcspecialattks LIKE BINARY '%O%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "23,1^") WHERE npcspecialattks LIKE BINARY '%W%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "24,1^") WHERE npcspecialattks LIKE BINARY '%H%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "25,1^") WHERE npcspecialattks LIKE BINARY '%G%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "26,1^") WHERE npcspecialattks LIKE BINARY '%g%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "27,1^") WHERE npcspecialattks LIKE BINARY '%d%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "28,1^") WHERE npcspecialattks LIKE BINARY '%i%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "29,1^") WHERE npcspecialattks LIKE BINARY '%t%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "30,1^") WHERE npcspecialattks LIKE BINARY '%n%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "31,1^") WHERE npcspecialattks LIKE BINARY '%p%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "32,1^") WHERE npcspecialattks LIKE BINARY '%J%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "33,1^") WHERE npcspecialattks LIKE BINARY '%j%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "34,1^") WHERE npcspecialattks LIKE BINARY '%o%';
UPDATE npc_types SET special_abilities = CONCAT(special_abilities, "35,1^") WHERE npcspecialattks LIKE BINARY '%Z%';
UPDATE npc_types SET special_abilities = TRIM(TRAILING '^' FROM special_abilities);

ALTER TABLE `npc_types`  DROP COLUMN `npcspecialattks`;
)",
	},
	ManifestEntry{
		.version = 9009,
		.description = "2013_10_12_merc_special_abilities.sql",
		.check = "SHOW COLUMNS FROM `merc_stats` LIKE 'special_abilities'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merc_stats`  ADD COLUMN `special_abilities` TEXT NULL AFTER `specialattks`;
ALTER TABLE `merc_stats` MODIFY COLUMN `special_abilities`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL;

UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "1,1^") WHERE specialattks LIKE BINARY '%S%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "2,1^") WHERE specialattks LIKE BINARY '%E%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "3,1^") WHERE specialattks LIKE BINARY '%R%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "4,1^") WHERE specialattks LIKE BINARY '%r%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "5,1^") WHERE specialattks LIKE BINARY '%F%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "6,1^") WHERE specialattks LIKE BINARY '%T%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "7,1^") WHERE specialattks LIKE BINARY '%Q%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "8,1^") WHERE specialattks LIKE BINARY '%L%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "9,1^") WHERE specialattks LIKE BINARY '%b%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "10,1^") WHERE specialattks LIKE BINARY '%m%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "11,1^") WHERE specialattks LIKE BINARY '%Y%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "12,1^") WHERE specialattks LIKE BINARY '%U%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "13,1^") WHERE specialattks LIKE BINARY '%M%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "14,1^") WHERE specialattks LIKE BINARY '%C%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "15,1^") WHERE specialattks LIKE BINARY '%N%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "16,1^") WHERE specialattks LIKE BINARY '%I%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "17,1^") WHERE specialattks LIKE BINARY '%D%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "18,1^") WHERE specialattks LIKE BINARY '%K%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "19,1^") WHERE specialattks LIKE BINARY '%A%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "20,1^") WHERE specialattks LIKE BINARY '%B%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "21,1^") WHERE specialattks LIKE BINARY '%f%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "22,1^") WHERE specialattks LIKE BINARY '%O%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "23,1^") WHERE specialattks LIKE BINARY '%W%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "24,1^") WHERE specialattks LIKE BINARY '%H%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "25,1^") WHERE specialattks LIKE BINARY '%G%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "26,1^") WHERE specialattks LIKE BINARY '%g%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "27,1^") WHERE specialattks LIKE BINARY '%d%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "28,1^") WHERE specialattks LIKE BINARY '%i%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "29,1^") WHERE specialattks LIKE BINARY '%t%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "30,1^") WHERE specialattks LIKE BINARY '%n%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "31,1^") WHERE specialattks LIKE BINARY '%p%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "32,1^") WHERE specialattks LIKE BINARY '%J%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "33,1^") WHERE specialattks LIKE BINARY '%j%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "34,1^") WHERE specialattks LIKE BINARY '%o%';
UPDATE merc_stats SET special_abilities = CONCAT(special_abilities, "35,1^") WHERE specialattks LIKE BINARY '%Z%';
UPDATE merc_stats SET special_abilities = TRIM(TRAILING '^' FROM special_abilities);

ALTER TABLE `merc_stats`  DROP COLUMN `specialattks`;
)",
	},
	ManifestEntry{
		.version = 9011,
		.description = "2013_10_31_recipe_disabling.sql",
		.check = "SHOW COLUMNS FROM `tradeskill_recipe` LIKE 'enabled'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tradeskill_recipe` ADD `enabled` tinyint(1) NOT NULL DEFAULT '1';

)",
	},
	ManifestEntry{
		.version = 9014,
		.description = "2013_11_18_assistradius.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'assistradius'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE  `npc_types` ADD  `assistradius` INT( 10 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `aggroradius`;

)",
	},
	ManifestEntry{
		.version = 9015,
		.description = "2013_12_26_merchantlist_class_required.sql",
		.check = "SHOW COLUMNS FROM `merchantlist` LIKE 'classes_required'",
		.condition = "empty",
		.match = "",
		.sql = R"(
 ALTER TABLE `merchantlist` ADD COLUMN `classes_required` INT(11) NOT NULL DEFAULT '65535';


)",
	},
	ManifestEntry{
		.version = 9017,
		.description = "2014_01_08_spellsnewadditions.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'persistdeath'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field200` `suspendable` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field202` `songcap` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field215` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field216` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field217` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field218` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `maxtargets` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field220` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field221` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field222` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `field223` INT(11) DEFAULT '0';
ALTER TABLE `spells_new` ADD `persistdeath` INT(11) DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9018,
		.description = "2014_01_09_preservepetsize.sql",
		.check = "SHOW COLUMNS FROM `character_pet_info` LIKE 'size'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_pet_info` ADD `size` FLOAT NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9020,
		.description = "2014_01_20_not_extendable.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'not_extendable'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field197` `not_extendable` INT(11) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9022,
		.description = "2014_01_20_weather.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'rain_chance1'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table zone drop column `weather`;
alter table zone add column `rain_chance1` int(4) not null default 0;
alter table zone add column `rain_chance2` int(4) not null default 0;
alter table zone add column `rain_chance3` int(4) not null default 0;
alter table zone add column `rain_chance4` int(4) not null default 0;
alter table zone add column `rain_duration1` int(4) not null default 0;
alter table zone add column `rain_duration2` int(4) not null default 0;
alter table zone add column `rain_duration3` int(4) not null default 0;
alter table zone add column `rain_duration4` int(4) not null default 0;
alter table zone add column `snow_chance1` int(4) not null default 0;
alter table zone add column `snow_chance2` int(4) not null default 0;
alter table zone add column `snow_chance3` int(4) not null default 0;
alter table zone add column `snow_chance4` int(4) not null default 0;
alter table zone add column `snow_duration1` int(4) not null default 0;
alter table zone add column `snow_duration2` int(4) not null default 0;
alter table zone add column `snow_duration3` int(4) not null default 0;
alter table zone add column `snow_duration4` int(4) not null default 0;

UPDATE `zone` SET `snow_chance1`=25, `snow_chance2`=20, `snow_chance3`=10, `snow_chance4`=20, `snow_duration1`=10, `snow_duration2`=8, `snow_duration3`=5, `snow_duration4`=10 WHERE  `id`=160;
UPDATE `zone` SET `rain_chance1`=5, `rain_chance2`=5 WHERE  `id`=202;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=306;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=304;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=23;
UPDATE `zone` SET `snow_chance1`=50, `snow_chance2`=25, `snow_chance3`=10, `snow_chance4`=25, `snow_duration1`=24, `snow_duration2`=24, `snow_duration3`=24, `snow_duration4`=24 WHERE  `id`=112;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=303;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=50, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=302;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_duration1`=10, `rain_duration2`=10 WHERE  `id`=133;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_duration1`=10, `rain_duration2`=10 WHERE  `id`=132;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_duration1`=10, `rain_duration2`=10 WHERE  `id`=131;
UPDATE `zone` SET `rain_chance1`=5, `rain_chance2`=2 WHERE  `id`=257;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=104;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=5, `rain_chance4`=5, `rain_duration1`=2, `rain_duration2`=3, `rain_duration3`=1, `rain_duration4`=2 WHERE  `id`=439;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=365;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=4;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=204;
UPDATE `zone` SET `rain_chance1`=100, `rain_chance2`=100, `rain_chance3`=100, `rain_chance4`=100, `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=224;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=138;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=56;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=31;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=34;
UPDATE `zone` SET `rain_chance1`=100, `rain_chance2`=100, `rain_chance3`=100, `rain_chance4`=100, `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=419;
UPDATE `zone` SET `rain_chance1`=15, `rain_chance2`=15, `rain_chance3`=5, `rain_chance4`=5 WHERE  `id`=116;
UPDATE `zone` SET `rain_chance1`=15, `rain_chance2`=15, `rain_chance3`=5, `rain_chance4`=5 WHERE  `id`=115;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=188;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=189;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=410;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=75, `rain_chance3`=75, `rain_chance4`=25, `rain_duration1`=16, `rain_duration2`=12, `rain_duration3`=12, `rain_duration4`=16 WHERE  `id`=276;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1, `snow_duration2`=1, `snow_duration3`=1, `snow_duration4`=1 WHERE  `id`=430;
UPDATE `zone` SET `rain_chance2`=19, `rain_chance3`=14, `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_chance1`=20, `snow_chance4`=20, `snow_duration1`=2, `snow_duration4`=1 WHERE  `id`=370;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=194;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=108;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=110;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=51;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=196;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=5, `rain_chance4`=10 WHERE  `id`=316;
UPDATE `zone` SET `rain_chance1`=20, `rain_chance3`=10, `rain_chance4`=20, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=166;
UPDATE `zone` SET `rain_chance1`=20, `rain_chance3`=10, `rain_chance4`=20, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_duration1`=12 WHERE  `id`=165;
UPDATE `zone` SET `rain_chance1`=20, `rain_chance2`=20, `rain_chance3`=5, `rain_chance4`=10 WHERE  `id`=225;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=329;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=21;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=99;
UPDATE `zone` SET `rain_chance1`=30, `rain_chance2`=30, `rain_chance3`=20, `rain_chance4`=30, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=255;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=301;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=359;
UPDATE `zone` SET `rain_chance1`=20, `rain_chance2`=20, `rain_chance3`=15, `rain_chance4`=20, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=201;
UPDATE `zone` SET `rain_chance1`=35, `rain_chance2`=45, `rain_chance3`=15, `rain_chance4`=20, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=312;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=75, `rain_chance3`=50, `rain_chance4`=5, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=114;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=263;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=36;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=50, `rain_chance4`=25, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=182;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_duration1`=12 WHERE  `id`=142;
UPDATE `zone` SET `rain_chance1`=5, `rain_chance2`=3 WHERE  `id`=361;
UPDATE `zone` SET `rain_chance1`=8, `rain_chance2`=5 WHERE  `id`=259;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=357;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=156;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=149;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=50, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_chance1`=10, `snow_duration1`=10 WHERE  `id`=406;
UPDATE `zone` SET `rain_chance1`=30, `rain_chance2`=40, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=119;
UPDATE `zone` SET `rain_chance1`=3, `rain_chance2`=3, `rain_chance3`=3, `rain_chance4`=3, `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=32;
UPDATE `zone` SET `rain_chance1`=3, `rain_chance2`=3, `rain_chance3`=3, `rain_chance4`=3, `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=33;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=50, `rain_chance4`=25, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=374;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_duration1`=12 WHERE  `id`=412;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=331;
UPDATE `zone` SET `rain_chance1`=5, `rain_chance2`=5 WHERE  `id`=345;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_duration1`=12 WHERE  `id`=30;
UPDATE `zone` SET `rain_chance1`=35, `rain_chance2`=45, `rain_chance3`=15, `rain_chance4`=20, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=107;
UPDATE `zone` SET `rain_chance1`=70, `rain_chance2`=70, `rain_chance3`=70, `rain_chance4`=70, `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=46;
UPDATE `zone` SET `rain_duration1`=15, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_chance1`=25, `snow_chance2`=15, `snow_chance3`=5, `snow_chance4`=15, `snow_duration1`=24, `snow_duration2`=12, `snow_duration3`=6, `snow_duration4`=12 WHERE  `id`=95;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=190;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=10, `rain_chance4`=25, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_duration1`=12 WHERE  `id`=275;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=43;
UPDATE `zone` SET `rain_chance1`=35, `rain_chance2`=45, `rain_chance3`=15, `rain_chance4`=20, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=134;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1, `snow_duration2`=1, `snow_duration3`=1, `snow_duration4`=1 WHERE  `id`=97;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1, `snow_duration2`=1, `snow_duration3`=1, `snow_duration4`=1 WHERE  `id`=258;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=199;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=50, `rain_chance4`=25, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=431;
UPDATE `zone` SET `rain_chance1`=30, `rain_chance2`=40, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=200;
UPDATE `zone` SET `rain_chance4`=25 WHERE  `id`=113;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_chance1`=15, `snow_chance2`=15, `snow_chance3`=15, `snow_chance4`=15, `snow_duration1`=24, `snow_duration2`=6, `snow_duration3`=2, `snow_duration4`=10 WHERE  `id`=48;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_chance1`=50, `snow_chance2`=40, `snow_chance3`=35, `snow_chance4`=40, `snow_duration1`=15, `snow_duration2`=10, `snow_duration3`=4, `snow_duration4`=12 WHERE  `id`=436;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=243;
UPDATE `zone` SET `rain_chance1`=30, `rain_chance2`=30, `rain_chance3`=30, `rain_chance4`=30, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=428;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_chance1`=50, `snow_chance2`=15, `snow_chance3`=15, `snow_chance4`=15, `snow_duration1`=24, `snow_duration2`=6, `snow_duration3`=2, `snow_duration4`=10 WHERE  `id`=102;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_chance1`=15, `snow_chance2`=15, `snow_chance3`=15, `snow_chance4`=15, `snow_duration1`=24, `snow_duration2`=6, `snow_duration3`=2, `snow_duration4`=10 WHERE  `id`=174;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=136;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_chance1`=50, `snow_chance2`=15, `snow_chance3`=15, `snow_chance4`=15, `snow_duration1`=24, `snow_duration2`=6, `snow_duration3`=2, `snow_duration4`=10 WHERE  `id`=139;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=404;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=405;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=50, `rain_chance4`=25, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=143;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=394;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=416;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=253;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=252;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_duration1`=12 WHERE  `id`=362;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=140;
UPDATE `zone` SET `rain_chance1`=30, `rain_chance2`=30, `rain_chance3`=30, `rain_chance4`=30, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_chance1`=5, `snow_chance2`=5, `snow_chance3`=5, `snow_duration1`=1, `snow_duration2`=1, `snow_duration3`=1 WHERE  `id`=418;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=333;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=140;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=277;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1, `snow_duration2`=1, `snow_duration3`=1, `snow_duration4`=1 WHERE  `id`=103;
UPDATE `zone` SET `rain_chance1`=2, `rain_chance2`=2, `rain_chance3`=2, `rain_chance4`=2, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=332;
UPDATE `zone` SET `rain_chance1`=5, `rain_chance2`=5, `rain_chance3`=5, `rain_chance4`=5, `rain_duration1`=10, `rain_duration2`=10, `rain_duration3`=10, `rain_duration4`=10 WHERE  `id`=336;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=173;
UPDATE `zone` SET `rain_chance1`=2, `rain_chance2`=8, `rain_chance3`=10, `rain_chance4`=3, `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=141;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10, `snow_chance1`=25, `snow_duration1`=24 WHERE  `id`=389;
UPDATE `zone` SET `rain_duration1`=24, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24 WHERE  `id`=191;
UPDATE `zone` SET `rain_chance1`=25, `rain_chance2`=25, `rain_chance3`=50, `rain_chance4`=25, `rain_duration1`=1, `rain_duration2`=1, `rain_duration3`=1, `rain_duration4`=1 WHERE  `id`=223;
UPDATE `zone` SET `rain_duration1`=1, `rain_duration2`=2, `rain_duration3`=1, `rain_duration4`=1, `snow_duration1`=1 WHERE  `id`=49;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=281;
UPDATE `zone` SET `rain_chance1`=6, `rain_chance2`=6, `rain_chance3`=6, `rain_chance4`=6, `rain_duration1`=12, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=288;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=290;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=170;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=27;
UPDATE `zone` SET `rain_chance2`=100, `rain_chance3`=100, `rain_chance4`=50, `rain_duration2`=24, `rain_duration3`=24, `rain_duration4`=24, `snow_chance1`=100, `snow_chance4`=50, `snow_duration1`=24, `snow_duration4`=24 WHERE  `id`=289;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=285;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=171;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=282;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=283;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=24, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=280;
UPDATE `zone` SET `rain_chance1`=10, `rain_chance2`=10, `rain_chance3`=10, `rain_chance4`=10, `rain_duration1`=12, `rain_duration2`=6, `rain_duration3`=2, `rain_duration4`=10 WHERE  `id`=254;
)",
	},
	ManifestEntry{
		.version = 9025,
		.description = "2014_02_13_rename_instance_lockout_tables.sql",
		.check = "SHOW TABLES LIKE 'instance_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- rename the instance_lockout tables to instance_list. They have nothing to do with lockouts.
ALTER TABLE `instance_lockout` RENAME TO  `instance_list` ;
ALTER TABLE `instance_lockout_player` RENAME TO  `instance_list_player` ;
)",
	},
	ManifestEntry{
		.version = 9026,
		.description = "2014_02_13_spells_new_update.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'ConeStartAngle'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field161` `not_reflectable` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field151` `no_partial_resist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field189` `MinResist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field190` `MaxResist` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field194` `ConeStartAngle` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field195` `ConeStopAngle` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field208` `rank` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field159` `npc_no_los` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field213` `NotOutofCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field214` `NotInCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field168` `IsDiscipline` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field211` `CastRestriction` INT(11) NOT NULL DEFAULT '0';

UPDATE altadv_vars SET sof_next_id = 8261 WHERE skill_id = 8232;
UPDATE altadv_vars SET sof_next_id = 0 WHERE skill_id = 8261;
UPDATE altadv_vars SET sof_current_level = 3 WHERE skill_id = 8261;


)",
	},
	ManifestEntry{
		.version = 9027,
		.description = "2014_02_20_buff_update.sql",
		.check = "SHOW COLUMNS FROM `character_buffs` LIKE 'caston_y'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- UPDATE BUFF TABLES
ALTER TABLE `character_buffs` CHANGE `death_save_chance` `dot_rune` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` CHANGE `DeathSaveSuccessChance` `dot_rune` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` CHANGE `DeathSaveSuccessChance` `dot_rune` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` CHANGE `death_save_aa_chance` `caston_x` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` CHANGE `CasterAARank` `caston_x` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` CHANGE `CasterAARank` `caston_x` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `caston_y` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `caston_z` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `character_buffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `merc_buffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `botbuffs` ADD `ExtraDIChance` INT(10) NOT NULL DEFAULT '0';

ALTER TABLE `spells_new` CHANGE `not_reflectable` `reflectable` INT(11) NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9028,
		.description = "2014_02_26_roambox_update.sql",
		.check = "SHOW COLUMNS FROM `spawngroup` LIKE 'mindelay'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table `spawngroup` add column `mindelay` int(11) not null default 15000 AFTER delay;
alter table `spawngroup` change `delay` `delay` int(11) not null default 45000;
)",
	},
	ManifestEntry{
		.version = 9030,
		.description = "2014_04_04_physicalresist.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'PhR'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE  `npc_types` ADD  `PhR` smallint( 5 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `Corrup`;

-- Approximate baseline live npc values based on extensive parsing.
UPDATE npc_types SET PhR = 10 WHERE PhR = 0 AND level <= 50;
UPDATE npc_types SET PhR = (10 + (level - 50))  WHERE PhR = 0 AND (level > 50 AND level <= 60);
UPDATE npc_types SET PhR = (20 + ((level - 60)*4))  WHERE PhR = 0 AND level > 60;


)",
	},
	ManifestEntry{
		.version = 9031,
		.description = "2014_04_10_no_target_with_hotkey.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'no_target_hotkey'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE  `npc_types` ADD  `no_target_hotkey` tinyint( 1 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `healscale`;

)",
	},
	ManifestEntry{
		.version = 9032,
		.description = "2014_04_12_slowmitigation.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'slow_mitigation'",
		.condition = "contains",
		.match = "float",
		.sql = R"(
-- Convert all values from FLOAT to INT
UPDATE npc_types SET slow_mitigation = slow_mitigation * 100;

-- Change variable type from FLOAT TO INT
ALTER TABLE npc_types MODIFY slow_mitigation smallint(4) NOT NULL DEFAULT  '0';



)",
	},
	ManifestEntry{
		.version = 9034,
		.description = "2014_04_25_spawn_events.sql",
		.check = "SHOW COLUMNS FROM `spawn_events` LIKE 'strict'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table spawn_events add column `strict` tinyint(4) not null default 0;
)",
	},
	ManifestEntry{
		.version = 9035,
		.description = "2014_04_27_aispelleffects.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'npc_spells_effects_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Note: The data entered into the new table are only examples and can be deleted/modified as needed.

ALTER TABLE  `npc_types` ADD  `npc_spells_effects_id` int( 11 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `npc_spells_id`;

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `npc_spells_effects`
-- ----------------------------
DROP TABLE IF EXISTS `npc_spells_effects`;
CREATE TABLE `npc_spells_effects` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` tinytext,
  `parent_list` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1080 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of npc_spells_effects
-- ----------------------------
INSERT INTO `npc_spells_effects` VALUES ('1', 'Critical Melee [All Skills]', '0');
INSERT INTO `npc_spells_effects` VALUES ('2', 'Damage Shield', '0');
INSERT INTO `npc_spells_effects` VALUES ('3', 'Melee Haste', '0');
INSERT INTO `npc_spells_effects` VALUES ('4', 'Resist Spell Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('5', 'Resist Direct Dmg Spell Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('6', 'Reflect Spell Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('7', 'Spell Damage Shield', '0');
INSERT INTO `npc_spells_effects` VALUES ('8', 'Melee Mitigation [All]', '0');
INSERT INTO `npc_spells_effects` VALUES ('9', 'Avoid Melee', '0');
INSERT INTO `npc_spells_effects` VALUES ('10', 'Riposte Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('11', 'Dodge Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('12', 'Parry Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('13', 'Decrease Dmg Taken [2HS]', '0');
INSERT INTO `npc_spells_effects` VALUES ('14', 'Increase Dmg Taken [1HS]', '0');
INSERT INTO `npc_spells_effects` VALUES ('15', 'Block Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('16', 'Melee Lifetap', '0');
INSERT INTO `npc_spells_effects` VALUES ('17', 'Hit Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('18', 'Increase Dmg [1HS]', '0');
INSERT INTO `npc_spells_effects` VALUES ('19', 'Increase Archery Dmg', '0');
INSERT INTO `npc_spells_effects` VALUES ('20', 'Flurry Chance', '0');
INSERT INTO `npc_spells_effects` VALUES ('21', 'Add Damage [2HS]', '0');
INSERT INTO `npc_spells_effects` VALUES ('22', 'Divine Aura', '0');
INSERT INTO `npc_spells_effects` VALUES ('23', 'Cast CH on Kill', '0');
INSERT INTO `npc_spells_effects` VALUES ('24', 'Critical Heal', '0');
INSERT INTO `npc_spells_effects` VALUES ('25', 'Critical Direct Dmg', '0');
INSERT INTO `npc_spells_effects` VALUES ('26', 'Heal Rate', '0');
INSERT INTO `npc_spells_effects` VALUES ('27', 'Negate Damage Shield', '0');
INSERT INTO `npc_spells_effects` VALUES ('28', 'Increase Spell Vulnerability [All]', '0');
INSERT INTO `npc_spells_effects` VALUES ('29', 'Decrease Spell Vulnerability [FR]', '0');
INSERT INTO `npc_spells_effects` VALUES ('30', 'Movement Speed', '0');


SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `npc_spells_effects_entries`
-- ----------------------------
DROP TABLE IF EXISTS `npc_spells_effects_entries`;
CREATE TABLE `npc_spells_effects_entries` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `npc_spells_effects_id` int(11) NOT NULL DEFAULT '0',
  `spell_effect_id` smallint(5) NOT NULL DEFAULT '0',
  `minlevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `maxlevel` tinyint(3) unsigned NOT NULL DEFAULT '255',
  `se_base` int(11) NOT NULL DEFAULT '0',
  `se_limit` int(11) NOT NULL DEFAULT '0',
  `se_max` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `spellsid_spellid` (`npc_spells_effects_id`,`spell_effect_id`)
) ENGINE=InnoDB AUTO_INCREMENT=18374 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of npc_spells_effects_entries
-- ----------------------------
INSERT INTO `npc_spells_effects_entries` VALUES ('1', '1', '169', '0', '255', '10000', '-1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('2', '2', '59', '0', '255', '-60', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('3', '3', '11', '0', '255', '150', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('4', '4', '180', '0', '255', '50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('5', '5', '378', '0', '255', '85', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('6', '6', '158', '0', '255', '50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('7', '7', '157', '0', '255', '-300', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('8', '8', '168', '0', '255', '-50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('9', '9', '172', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('10', '10', '173', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('11', '11', '174', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('12', '12', '175', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('13', '13', '197', '0', '255', '-80', '3', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('14', '14', '197', '0', '255', '80', '1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('15', '15', '188', '0', '255', '10000', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('16', '16', '178', '0', '255', '90', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('17', '17', '184', '0', '255', '10000', '-1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('18', '18', '185', '0', '255', '100', '1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('19', '19', '301', '0', '255', '100', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('20', '20', '279', '0', '255', '50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('21', '21', '220', '0', '255', '2000', '1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('22', '22', '40', '0', '255', '1', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('23', '23', '360', '0', '255', '100', '13', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('24', '24', '274', '0', '255', '90', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('25', '25', '294', '0', '255', '100', '200', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('26', '26', '120', '0', '255', '50', '0', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('27', '27', '382', '0', '255', '0', '55', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('28', '28', '296', '0', '255', '1000', '-1', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('29', '29', '296', '0', '255', '-50', '2', '0');
INSERT INTO `npc_spells_effects_entries` VALUES ('30', '30', '3', '0', '255', '60', '0', '0');
)",
	},
	ManifestEntry{
		.version = 9036,
		.description = "2014_05_04_slowmitigationfix.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'slow_mitigation'",
		.condition = "contains",
		.match = "float",
		.sql = R"(
ALTER TABLE npc_types MODIFY slow_mitigation smallint(4) NOT NULL DEFAULT  '0';
)",
	},
	ManifestEntry{
		.version = 9040,
		.description = "2014_07_10_npc_spells.sql",
		.check = "SHOW COLUMNS FROM `npc_spells` LIKE 'engaged_no_sp_recast_min'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- npc_types
ALTER TABLE  `npc_types` ADD  `ammo_idfile` varchar( 30 ) NOT NULL DEFAULT  'IT10' AFTER `d_meele_texture2`;
ALTER TABLE  `npc_types` ADD  `ranged_type` tinyint( 4 ) UNSIGNED NOT NULL DEFAULT  '7' AFTER `sec_melee_type`;
ALTER TABLE  `npc_types` ADD  `Avoidance` mediumint(9) UNSIGNED NOT NULL DEFAULT  '0' AFTER `Accuracy`;

-- npc spells
ALTER TABLE  `npc_spells` ADD  `range_proc` smallint(5) NOT NULL DEFAULT '-1';
ALTER TABLE  `npc_spells` ADD  `rproc_chance` smallint(5) NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `defensive_proc` smallint(5) NOT NULL DEFAULT '-1';
ALTER TABLE  `npc_spells` ADD  `dproc_chance` smallint(5) NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `fail_recast` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_no_sp_recast_min` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_b_self_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `engaged_b_other_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD `engaged_d_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_no_sp_recast_min` int(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `pursue_d_chance` tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_no_sp_recast_min` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_no_sp_recast_max` int(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE  `npc_spells` ADD  `idle_b_chance` tinyint(11) unsigned NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9041,
		.description = "2014_08_02_spells_new.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'viral_range'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- spells new talbe update
ALTER TABLE `spells_new` CHANGE `NotOutofCombat` `InCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `NotInCombat` `OutofCombat` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field201` `viral_range` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field218` `aemaxtargets` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE  `spells_new` ADD  `field225` int( 11 ) NOT NULL DEFAULT  '0' AFTER `persistdeath`;
ALTER TABLE  `spells_new` ADD  `field226` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field225`;
ALTER TABLE  `spells_new` ADD  `min_dist` float( 0 ) NOT NULL DEFAULT  '0' AFTER `field226`;
ALTER TABLE  `spells_new` ADD  `min_dist_mod` float( 0 ) NOT NULL DEFAULT  '0' AFTER `min_dist`;
ALTER TABLE  `spells_new` ADD  `max_dist` float( 0 ) NOT NULL DEFAULT  '0' AFTER `min_dist_mod`;
ALTER TABLE  `spells_new` ADD  `max_dist_mod` float( 0 ) NOT NULL DEFAULT  '0' AFTER `max_dist`;
ALTER TABLE  `spells_new` ADD  `min_range` int( 11 ) NOT NULL DEFAULT  '0' AFTER `max_dist_mod`;
ALTER TABLE  `spells_new` ADD  `field232` int( 11 ) NOT NULL DEFAULT  '0' AFTER `min_range`;
ALTER TABLE  `spells_new` ADD  `field233` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field232`;
ALTER TABLE  `spells_new` ADD  `field234` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field233`;
ALTER TABLE  `spells_new` ADD  `field235` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field234`;
ALTER TABLE  `spells_new` ADD  `field236` int( 11 ) NOT NULL DEFAULT  '0' AFTER `field235`;
)",
	},
	ManifestEntry{
		.version = 9042,
		.description = "2014_08_12_npc_raid_targets.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'raid_target'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `raid_target` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0' AFTER `no_target_hotkey`;

)",
	},
	ManifestEntry{
		.version = 9043,
		.description = "2014_08_18_spells_new_update.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'viral_targets'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field191` `viral_targets` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field192` `viral_timer` INT(11) NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9044,
		.description = "2014_08_20_merchantlist_probability.sql",
		.check = "SHOW COLUMNS FROM `merchantlist` LIKE 'probability'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merchantlist` ADD `probability` INT(3) NOT NULL DEFAULT '100' AFTER `classes_required`;
)",
	},
	ManifestEntry{
		.version = 9045,
		.description = "2014_08_23_complete_queryserv_table_structures.sql",
		.check = "SHOW TABLES LIKE 'qs_player_aa_rate_hourly'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- QS Table Structures --

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for qs_merchant_transaction_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_merchant_transaction_record`;
CREATE TABLE `qs_merchant_transaction_record` (
  `transaction_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `zone_id` int(11) DEFAULT '0',
  `merchant_id` int(11) DEFAULT '0',
  `merchant_pp` int(11) DEFAULT '0',
  `merchant_gp` int(11) DEFAULT '0',
  `merchant_sp` int(11) DEFAULT '0',
  `merchant_cp` int(11) DEFAULT '0',
  `merchant_items` mediumint(7) DEFAULT '0',
  `char_id` int(11) DEFAULT '0',
  `char_pp` int(11) DEFAULT '0',
  `char_gp` int(11) DEFAULT '0',
  `char_sp` int(11) DEFAULT '0',
  `char_cp` int(11) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`transaction_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_merchant_transaction_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_merchant_transaction_record_entries`;
CREATE TABLE `qs_merchant_transaction_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_aa_rate_hourly
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_aa_rate_hourly`;
CREATE TABLE `qs_player_aa_rate_hourly` (
  `char_id` int(11) NOT NULL DEFAULT '0',
  `hour_time` int(11) NOT NULL,
  `aa_count` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`char_id`,`hour_time`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for qs_player_delete_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_delete_record`;
CREATE TABLE `qs_player_delete_record` (
  `delete_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char_id` int(11) DEFAULT '0',
  `stack_size` mediumint(7) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`delete_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_delete_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_delete_record_entries`;
CREATE TABLE `qs_player_delete_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_events
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_events`;
CREATE TABLE `qs_player_events` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) DEFAULT '0',
  `event` int(11) unsigned DEFAULT '0',
  `event_desc` varchar(255) DEFAULT NULL,
  `time` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for qs_player_handin_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_handin_record`;
CREATE TABLE `qs_player_handin_record` (
  `handin_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `quest_id` int(11) DEFAULT '0',
  `char_id` int(11) DEFAULT '0',
  `char_pp` int(11) DEFAULT '0',
  `char_gp` int(11) DEFAULT '0',
  `char_sp` int(11) DEFAULT '0',
  `char_cp` int(11) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  `npc_id` int(11) DEFAULT '0',
  `npc_pp` int(11) DEFAULT '0',
  `npc_gp` int(11) DEFAULT '0',
  `npc_sp` int(11) DEFAULT '0',
  `npc_cp` int(11) DEFAULT '0',
  `npc_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`handin_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_handin_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_handin_record_entries`;
CREATE TABLE `qs_player_handin_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `action_type` char(6) DEFAULT 'action',
  `char_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_move_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_move_record`;
CREATE TABLE `qs_player_move_record` (
  `move_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_slot` mediumint(7) DEFAULT '0',
  `stack_size` mediumint(7) DEFAULT '0',
  `char_items` mediumint(7) DEFAULT '0',
  `postaction` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`move_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_move_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_move_record_entries`;
CREATE TABLE `qs_player_move_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_npc_kill_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_npc_kill_record`;
CREATE TABLE `qs_player_npc_kill_record` (
  `fight_id` int(11) NOT NULL AUTO_INCREMENT,
  `npc_id` int(11) DEFAULT NULL,
  `type` int(11) DEFAULT NULL,
  `zone_id` int(11) DEFAULT NULL,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`fight_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_npc_kill_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_npc_kill_record_entries`;
CREATE TABLE `qs_player_npc_kill_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `char_id` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_speech
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_speech`;
CREATE TABLE `qs_player_speech` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `from` varchar(64) NOT NULL,
  `to` varchar(64) NOT NULL,
  `message` varchar(256) NOT NULL,
  `minstatus` smallint(5) NOT NULL,
  `guilddbid` int(11) NOT NULL,
  `type` tinyint(3) NOT NULL,
  `timerecorded` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_trade_record
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_trade_record`;
CREATE TABLE `qs_player_trade_record` (
  `trade_id` int(11) NOT NULL AUTO_INCREMENT,
  `time` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `char1_id` int(11) DEFAULT '0',
  `char1_pp` int(11) DEFAULT '0',
  `char1_gp` int(11) DEFAULT '0',
  `char1_sp` int(11) DEFAULT '0',
  `char1_cp` int(11) DEFAULT '0',
  `char1_items` mediumint(7) DEFAULT '0',
  `char2_id` int(11) DEFAULT '0',
  `char2_pp` int(11) DEFAULT '0',
  `char2_gp` int(11) DEFAULT '0',
  `char2_sp` int(11) DEFAULT '0',
  `char2_cp` int(11) DEFAULT '0',
  `char2_items` mediumint(7) DEFAULT '0',
  PRIMARY KEY (`trade_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for qs_player_trade_record_entries
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_trade_record_entries`;
CREATE TABLE `qs_player_trade_record_entries` (
  `event_id` int(11) DEFAULT '0',
  `from_id` int(11) DEFAULT '0',
  `from_slot` mediumint(7) DEFAULT '0',
  `to_id` int(11) DEFAULT '0',
  `to_slot` mediumint(7) DEFAULT '0',
  `item_id` int(11) DEFAULT '0',
  `charges` mediumint(7) DEFAULT '0',
  `aug_1` int(11) DEFAULT '0',
  `aug_2` int(11) DEFAULT '0',
  `aug_3` int(11) DEFAULT '0',
  `aug_4` int(11) DEFAULT '0',
  `aug_5` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

)",
	},
	ManifestEntry{
		.version = 9046,
		.description = "2014_08_23_player_events_and_player_aa_rate_hourly.sql",
		.check = "SHOW TABLES LIKE 'qs_player_events'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- ----------------------------
-- Table structure for qs_player_events
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_events`;
CREATE TABLE `qs_player_events` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) DEFAULT '0',
  `event` int(11) unsigned DEFAULT '0',
  `event_desc` varchar(255) DEFAULT NULL,
  `time` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for qs_player_aa_rate_hourly
-- ----------------------------
DROP TABLE IF EXISTS `qs_player_aa_rate_hourly`;
CREATE TABLE `qs_player_aa_rate_hourly` (
  `char_id` int(11) NOT NULL DEFAULT '0',
  `hour_time` int(11) NOT NULL,
  `aa_count` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`char_id`,`hour_time`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
)",
	},
	ManifestEntry{
		.version = 9048,
		.description = "2014_09_09_attack_delay.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'attack_delay'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `attack_delay` TINYINT(3) UNSIGNED DEFAULT '30' NOT NULL AFTER `attack_speed`;
UPDATE `npc_types` SET `attack_delay` = 36 + 36 * (`attack_speed` / 100);
UPDATE `npc_types` SET `attack_delay` = 30 WHERE `attack_speed` = 0;

)",
	},
	ManifestEntry{
		.version = 9050,
		.description = "2014_09_20_ban_messages.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'ban_reason'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account` ADD COLUMN `ban_reason` TEXT NULL DEFAULT NULL, ADD COLUMN `suspend_reason` TEXT NULL DEFAULT NULL AFTER `ban_reason`;

)",
	},
	ManifestEntry{
		.version = 9051,
		.description = "2014_10_11_raidmotd.sql",
		.check = "SHOW COLUMNS FROM `raid_details` LIKE 'motd'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `raid_details` ADD `motd` varchar(1024);

)",
	},
	ManifestEntry{
		.version = 9052,
		.description = "2014_10_13_raidleadership.sql",
		.check = "SHOW TABLES LIKE 'raid_leaders'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `raid_leaders` (
	`gid` int(4) unsigned NOT NULL,
	`rid` int(4) unsigned NOT NULL,
	`marknpc` varchar(64) NOT NULL,
	`maintank` varchar(64) NOT NULL,
	`assist` varchar(64) NOT NULL,
	`puller` varchar(64) NOT NULL,
	`leadershipaa` tinyblob NOT NULL
);

)",
	},
	ManifestEntry{
		.version = 9053,
		.description = "2014_10_18_group_mentor.sql",
		.check = "SHOW COLUMNS FROM `group_leaders` LIKE 'mentoree'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `group_leaders` ADD `mentoree` VARCHAR(64) NOT NULL;
ALTER TABLE `group_leaders` ADD `mentor_percent` INT(4) DEFAULT 0 NOT NULL;

)",
	},
	ManifestEntry{
		.version = 9054,
		.description = "2014_10_19_raid_group_mentor.sql",
		.check = "SHOW COLUMNS FROM `raid_leaders` LIKE 'mentoree'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `raid_leaders` ADD `mentoree` VARCHAR(64) NOT NULL;
ALTER TABLE `raid_leaders` ADD `mentor_percent` INT(4) DEFAULT 0 NOT NULL;

)",
	},
	ManifestEntry{
		.version = 9055,
		.description = "2014_10_30_special_abilities_null.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'special_abilities'",
		.condition = "contains",
		.match = "NO",
		.sql = R"(
ALTER TABLE `merc_stats` MODIFY COLUMN `special_abilities`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL;

ALTER TABLE `npc_types` MODIFY COLUMN `special_abilities`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL;


)",
	},
	ManifestEntry{
		.version = 9056,
		.description = "2014_11_08_raidmembers.sql",
		.check = "SHOW COLUMNS FROM `raid_members` LIKE 'groupid'",
		.condition = "missing",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `raid_members` CHANGE COLUMN `groupid` `groupid` INT(4) UNSIGNED NOT NULL DEFAULT '0' AFTER `charid`;
)",
	},
	ManifestEntry{
		.version = 9057,
		.description = "2014_11_13_spells_new_updates.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'disallow_sit'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- spells new table update
ALTER TABLE `spells_new` CHANGE `field124` `disallow_sit` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field125` `deities0` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field196` `sneaking` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field158` `effectdescnum2` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field165` `ldon_trap` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field205` `no_block` INT(11) NOT NULL DEFAULT '0';



)",
	},
	ManifestEntry{
		.version = 9058,
		.description = "2014_11_26_inventorytableupdate.sql",
		.check = "SHOW COLUMNS FROM `inventory` LIKE 'ornamenticon'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Inventory table update
ALTER TABLE `inventory`
	ADD COLUMN `ornamenticon` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `custom_data`,
	ADD COLUMN `ornamentidfile` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamenticon`;

)",
	},
	ManifestEntry{
		.version = 9059,
		.description = "2014_12_01_mercs_table_update.sql",
		.check = "SHOW COLUMNS FROM `mercs` LIKE 'MercSize'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `mercs` ADD `MercSize` float( 0 ) NOT NULL DEFAULT '5' AFTER `Gender`;

)",
	},
	ManifestEntry{
		.version = 9060,
		.description = "2014_12_09_items_table_update.sql",
		.check = "SHOW COLUMNS FROM `items` LIKE 'herosforgemodel'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `items` ADD `herosforgemodel` int( 11 ) NOT NULL DEFAULT '0' AFTER `material`;

)",
	},
	ManifestEntry{
		.version = 9061,
		.description = "2014_12_13_inventory_table_update.sql",
		.check = "SHOW COLUMNS FROM `inventory` LIKE 'ornament_hero_model'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `inventory` ADD `ornament_hero_model` int( 11 ) NOT NULL DEFAULT '0' AFTER `ornamentidfile`;
)",
	},
	ManifestEntry{
		.version = 9062,
		.description = "2014_12_15_multiple_table_updates.sql",
		.check = "SHOW COLUMNS FROM `items` LIKE 'augslot6type'",
		.condition = "empty",
		.match = "",
		.sql = R"(
/* Add the new Aug Slot 6 Fields to the items table */
ALTER TABLE `items` ADD `augslot6type` tinyint( 3 ) NOT NULL DEFAULT '0' AFTER `augslot5visible`;
ALTER TABLE `items` ADD `augslot6visible` tinyint( 3 ) NOT NULL DEFAULT '0' AFTER `augslot6type`;
ALTER TABLE `items` ADD `augslot6unk2` int( 11 ) NOT NULL DEFAULT '0' AFTER `augslot5unk2`;

/* Add the new Aug Slot 6 Field to the inventory table */
ALTER TABLE `inventory` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the sharedbank table */
ALTER TABLE `sharedbank` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the object_contents table */
ALTER TABLE `object_contents` ADD `augslot6` mediumint( 7 ) NOT NULL DEFAULT '0' AFTER `augslot5`;

/* Add the new Aug Slot 6 Field to the sharedbank table */
ALTER TABLE `character_corpse_items` ADD `aug_6` int( 11 ) NOT NULL DEFAULT '0' AFTER `aug_5`;
)",
	},
	ManifestEntry{
		.version = 9063,
		.description = "2014_12_24_npc_types_update.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'd_melee_texture1'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` CHANGE `d_meele_texture1` `d_melee_texture1` INT(11) DEFAULT NULL;
ALTER TABLE `npc_types` CHANGE `d_meele_texture2` `d_melee_texture2` INT(11) DEFAULT NULL;
)",
	},
	ManifestEntry{
		.version = 9064,
		.description = "2014_12_24_npc_types_table_update.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'herosforgemodel'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `herosforgemodel` int( 11 ) NOT NULL DEFAULT '0' AFTER `helmtexture`;
)",
	},
	ManifestEntry{
		.version = 9066,
		.description = "2014_12_31_npc_types_default_values_update.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'bodytype'",
		.condition = "contains",
		.match = "YES",
		.sql = R"(
UPDATE `npc_types` SET `bodytype` = 0 WHERE `bodytype` IS NULL;
ALTER TABLE `npc_types` MODIFY `bodytype` INT(11) NOT NULL DEFAULT '1';
UPDATE `npc_types` SET `d_melee_texture1` = 0 WHERE `d_melee_texture1` IS NULL;
ALTER TABLE `npc_types` MODIFY `d_melee_texture1` INT(11) NOT NULL DEFAULT '0';
UPDATE `npc_types` SET `d_melee_texture2` = 0 WHERE `d_melee_texture2` IS NULL;
ALTER TABLE `npc_types` MODIFY `d_melee_texture2` INT(11) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9067,
		.description = "2015_01_21_npc_types_update.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'light'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `light` tinyint( 2 ) NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9068,
		.description = "2015_01_15_logsys_categories_table.sql",
		.check = "SHOW TABLES LIKE 'logsys_categories'",
		.condition = "empty",
		.match = "",
		.sql = R"(
SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `logsys_categories`;
CREATE TABLE `logsys_categories` (
  `log_category_id` int(11) NOT NULL,
  `log_category_description` varchar(150) DEFAULT NULL,
  `log_to_console` smallint(11) DEFAULT '0',
  `log_to_file` smallint(11) DEFAULT '0',
  `log_to_gmsay` smallint(11) DEFAULT '0',
  PRIMARY KEY (`log_category_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
)",
	},
	ManifestEntry{
		.version = 9069,
		.description = "2015_01_25_logsys_mercenaries_category.sql",
		.check = "SELECT * FROM `logsys_categories` WHERE `log_category_description` LIKE 'Mercenaries'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `logsys_categories` (`log_category_id`,`log_category_description`,`log_to_console`,`log_to_file`,`log_to_gmsay`) VALUES ('37', 'Mercenaries', '0', '0', '0');
)",
	},
	ManifestEntry{
		.version = 9070,
		.description = "2015_01_28_quest_debug_log_category.sql",
		.check = "SELECT * FROM `logsys_categories` WHERE `log_category_description` LIKE 'Quest Debug'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `logsys_categories` (`log_category_id`, `log_category_description`, `log_to_gmsay`) VALUES ('38', 'Quest Debug', '1');

)",
	},
	ManifestEntry{
		.version = 9071,
		.description = "2015_01_29_merc_stats_table_update.sql",
		.check = "SHOW COLUMNS FROM `merc_stats` LIKE 'statscale'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merc_stats` ADD `statscale` int( 11 ) NOT NULL DEFAULT '100' AFTER `runspeed`;
)",
	},
	ManifestEntry{
		.version = 9072,
		.description = "2015_01_30_merc_attack_delay.sql",
		.check = "SHOW COLUMNS FROM `merc_stats` LIKE 'attack_delay'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merc_stats` ADD `attack_delay` TINYINT(3) UNSIGNED DEFAULT '30' NOT NULL AFTER `attack_speed`;
UPDATE `merc_stats` SET `attack_delay` = 36 + 36 * (`attack_speed` / 100);
UPDATE `merc_stats` SET `attack_delay` = 30 WHERE `attack_speed` = 0;

)",
	},
	ManifestEntry{
		.version = 9073,
		.description = "2015_01_31_character_item_recast.sql",
		.check = "SHOW TABLES LIKE 'character_item_recast'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_item_recast` (
    `id` int(11) UNSIGNED NOT NULL DEFAULT 0,
    `recast_type` smallint(11) UNSIGNED NOT NULL DEFAULT 0,
    `timestamp` int(11) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY(`id`, `recast_type`),
    KEY `id` (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = latin1;

)",
	},
	ManifestEntry{
		.version = 9074,
		.description = "2015_02_01_logsys_packet_logs.sql",
		.check = "SELECT * FROM `logsys_categories` WHERE `log_category_description` LIKE 'Packet: Server -> Client'",
		.condition = "empty",
		.match = "",
		.sql = R"(
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('39', 'Packet: Server -> Client');
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('5', 'Packet: Client -> Server');
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('40', 'Packet: Client -> Server Unhandled');
)",
	},
	ManifestEntry{
		.version = 9075,
		.description = "2015_02_02_logsys_packet_logs_with_dump.sql",
		.check = "SELECT * FROM `logsys_categories` WHERE `log_category_description` LIKE 'Packet: Server -> Client With Dump'",
		.condition = "empty",
		.match = "",
		.sql = R"(
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('41', 'Packet: Server -> Client With Dump');
REPLACE INTO `logsys_categories` (`log_category_id`, `log_category_description`) VALUES ('42', 'Packet: Client -> Server With Dump');

)",
	},
	ManifestEntry{
		.version = 9076,
		.description = "2015_02_04_average_coin.sql",
		.check = "SHOW COLUMNS FROM `loottable` WHERE Field = 'avgcoin'",
		.condition = "contains",
		.match = "smallint",
		.sql = R"(
ALTER TABLE `loottable` CHANGE COLUMN `avgcoin` `avgcoin` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `maxcash`;
UPDATE `loottable` SET avgcoin = 0;

)",
	},
	ManifestEntry{
		.version = 9077,
		.description = "2015_02_12_zone_gravity.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'gravity'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone`
ADD COLUMN `gravity`  float NOT NULL DEFAULT .4 AFTER `snow_duration4`;
)",
	},
	ManifestEntry{
		.version = 9078,
		.description = "2015_05_20_buffinstrumentmod.sql",
		.check = "SHOW COLUMNS FROM `character_buffs` LIKE 'instrument_mod'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_buffs` ADD COLUMN `instrument_mod` int(10) DEFAULT 10 NOT NULL;

)",
	},
	ManifestEntry{
		.version = 9079,
		.description = "2015_05_23_buffdurations.sql",
		.check = "SHOW COLUMNS FROM `character_buffs` LIKE 'ticsremaining'",
		.condition = "contains",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `character_buffs` CHANGE COLUMN `ticsremaining` `ticsremaining` INT(11) SIGNED NOT NULL;
ALTER TABLE `merc_buffs` CHANGE COLUMN `TicsRemaining` `TicsRemaining` INT(11) SIGNED NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9080,
		.description = "2015_05_23_petbuffinstrumentmod.sql",
		.check = "SHOW COLUMNS FROM `character_pet_buffs` LIKE 'instrument_mod'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_pet_buffs` ADD COLUMN `instrument_mod` tinyint UNSIGNED DEFAULT 10 NOT NULL;

)",
	},
	ManifestEntry{
		.version = 9081,
		.description = "2015_05_23_dbstr_us.sql",
		.check = "SHOW TABLES LIKE 'db_str'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `db_str` (
	`id` INT(10) NOT NULL,
	`type` INT(10) NOT NULL,
	`value` TEXT NOT NULL,
	PRIMARY KEY (`id`, `type`)
);

)",
	},
	ManifestEntry{
		.version = 9082,
		.description = "2015_05_25_npc_types_texture_fields.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'armtexture'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE npc_types
ADD COLUMN `armtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `raid_target`,
ADD COLUMN `bracertexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `armtexture`,
ADD COLUMN `handtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `bracertexture`,
ADD COLUMN `legtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `handtexture`,
ADD COLUMN `feettexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `legtexture`;
)",
	},
	ManifestEntry{
		.version = 9083,
		.description = "2015_06_07_aa_update.sql",
		.check = "SHOW COLUMNS FROM `character_alternate_abilities` LIKE 'charges'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE character_alternate_abilities ADD COLUMN charges SMALLINT(11) UNSIGNED NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9084,
		.description = "2015_06_30_runspeed_adjustments.sql",
		.check = "SELECT `runspeed` FROM `npc_types` WHERE `runspeed` > 3",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
/* This rescales the old peq runspeeds which were about 80 percent too high to new values */
/* This section should only ever be run once */
UPDATE npc_types SET npc_types.runspeed = 1.050 WHERE (npc_types.runspeed > 0 and npc_types.runspeed < 1.2);
UPDATE npc_types SET npc_types.runspeed = 1.325 WHERE (npc_types.runspeed > 1.19 and npc_types.runspeed < 1.75 and race != 73 and race != 72);
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE (npc_types.runspeed > 1.69 and npc_types.runspeed < 2.2);
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE (npc_types.runspeed > 2.19 and npc_types.runspeed < 3);
UPDATE npc_types SET npc_types.runspeed = 3 WHERE npc_types.runspeed > 3;
)",
	},
	ManifestEntry{
		.version = 9085,
		.description = "2015_07_01_marquee_rule.sql",
		.check = "SELECT * FROM `rule_values` WHERE `rule_name` LIKE '%Character:MarqueeHPUpdates%'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `rule_values` (`rule_name`, `rule_value`, `notes`) VALUES ('Character:MarqueeHPUpdates', 'false', 'Will show Health % in center of screen < 100%');
)",
	},
	ManifestEntry{
		.version = 9086,
		.description = "2015_07_02_aa_rework.sql",
		.check = "SHOW TABLES LIKE 'aa_ranks'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `aa_ability`;
CREATE TABLE IF NOT EXISTS `aa_ability` (
  `id` int(10) unsigned NOT NULL,
  `name` text NOT NULL,
  `category` int(10) NOT NULL DEFAULT '-1',
  `classes` int(10) NOT NULL DEFAULT '65535',
  `races` int(10) NOT NULL DEFAULT '65535',
  `drakkin_heritage` int(10) NOT NULL DEFAULT '127',
  `deities` int(10) NOT NULL DEFAULT '131071',
  `status` int(10) NOT NULL DEFAULT '0',
  `type` int(10) NOT NULL DEFAULT '0',
  `charges` int(11) NOT NULL DEFAULT '0',
  `grant_only` tinyint(4) NOT NULL DEFAULT '0',
  `first_rank_id` int(10) NOT NULL DEFAULT '-1',
  `enabled` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `aa_ranks`;
CREATE TABLE IF NOT EXISTS `aa_ranks` (
  `id` int(10) unsigned NOT NULL,
  `upper_hotkey_sid` int(10) NOT NULL DEFAULT '-1',
  `lower_hotkey_sid` int(10) NOT NULL DEFAULT '-1',
  `title_sid` int(10) NOT NULL DEFAULT '-1',
  `desc_sid` int(10) NOT NULL DEFAULT '-1',
  `cost` int(10) NOT NULL DEFAULT '1',
  `level_req` int(10) NOT NULL DEFAULT '51',
  `spell` int(10) NOT NULL DEFAULT '-1',
  `spell_type` int(10) NOT NULL DEFAULT '0',
  `recast_time` int(10) NOT NULL DEFAULT '0',
  `expansion` int(10) NOT NULL DEFAULT '0',
  `prev_id` int(10) NOT NULL DEFAULT '-1',
  `next_id` int(10) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Dumping structure for table eqdb.aa_rank_effects
DROP TABLE IF EXISTS `aa_rank_effects`;
CREATE TABLE IF NOT EXISTS `aa_rank_effects` (
  `rank_id` int(10) unsigned NOT NULL,
  `slot` int(10) unsigned NOT NULL DEFAULT '1',
  `effect_id` int(10) NOT NULL DEFAULT '0',
  `base1` int(10) NOT NULL DEFAULT '0',
  `base2` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`rank_id`,`slot`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `aa_rank_prereqs`;
CREATE TABLE IF NOT EXISTS `aa_rank_prereqs` (
  `rank_id` int(10) unsigned NOT NULL,
  `aa_id` int(10) NOT NULL,
  `points` int(10) NOT NULL,
  PRIMARY KEY (`rank_id`,`aa_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

RENAME TABLE `character_alternate_abilities` TO `character_alternate_abilities_old`;
DROP TABLE IF EXISTS `character_alternate_abilities`;
CREATE TABLE IF NOT EXISTS `character_alternate_abilities` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `aa_id` smallint(11) unsigned NOT NULL DEFAULT '0',
  `aa_value` smallint(11) unsigned NOT NULL DEFAULT '0',
  `charges` smallint(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`aa_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `character_data` ADD COLUMN `aa_points_spent_old` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `aa_points_spent`;
ALTER TABLE `character_data` ADD COLUMN `aa_points_old` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `aa_points`;

UPDATE `character_data` SET `aa_points_spent_old` = `aa_points_spent`, `aa_points_old` = `aa_points`;

 -- sanity checks since if someone never logged in after the db conversion there is junk data
 -- I don't have a good way of addressing this so I keep the old data in aa_points_spent_old and aa_points_old and character_alternate_abilities_old
 -- for anyone who wants to personally polish up their player data
UPDATE `character_data` SET `aa_points_spent` = 2700 WHERE `aa_points_spent` > 2700;
UPDATE `character_data` SET `aa_points` = 5000 WHERE `aa_points` > 5000;

 -- another sanity check, give people a few levels below 51 to keep their points
UPDATE `character_data` SET `aa_points_spent` = 0 WHERE `level` < 48;
UPDATE `character_data` SET `aa_points` = 0 WHERE `level` < 48;

 -- aa refund here
UPDATE `character_data` SET `aa_points` = `aa_points_spent` + `aa_points`;
UPDATE `character_data` SET `aa_points_spent` = 0;

)",
	},
	ManifestEntry{
		.version = 9087,
		.description = "2015_09_25_inventory_snapshots.sql",
		.check = "SHOW TABLES LIKE 'inventory_snapshots'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `inventory_snapshots` (
	`time_index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`charid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`slotid` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`itemid` INT(11) UNSIGNED NULL DEFAULT '0',
	`charges` SMALLINT(3) UNSIGNED NULL DEFAULT '0',
	`color` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`augslot1` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot2` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot3` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot4` MEDIUMINT(7) UNSIGNED NOT NULL DEFAULT '0',
	`augslot5` MEDIUMINT(7) UNSIGNED NULL DEFAULT '0',
	`augslot6` MEDIUMINT(7) NOT NULL DEFAULT '0',
	`instnodrop` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	`custom_data` TEXT NULL,
	`ornamenticon` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornamentidfile` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`ornament_hero_model` INT(11) NOT NULL DEFAULT '0',
	PRIMARY KEY (`time_index`, `charid`, `slotid`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB;

ALTER TABLE `character_data` ADD COLUMN `e_last_invsnapshot` INT(11) UNSIGNED NOT NULL DEFAULT '0';

INSERT INTO `rule_values` VALUES
(1, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(2, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(4, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(5, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(10, 'Character:ActiveInvSnapshots', 'false', 'Takes a periodic snapshot of inventory contents from online players'),
(1, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(2, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(4, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(5, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(10, 'Character:InvSnapshotMinIntervalM', '180', 'Minimum time (in minutes) between inventory snapshots'),
(1, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(2, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(4, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(5, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(10, 'Character:InvSnapshotMinRetryM', '30', 'Time (in minutes) to re-attempt an inventory snapshot after a failure'),
(1, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries'),
(2, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries'),
(4, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries'),
(5, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries'),
(10, 'Character:InvSnapshotHistoryD', '30', 'Time (in days) to keep snapshot entries');

)",
	},
	ManifestEntry{
		.version = 9088,
		.description = "2015_11_01_perl_event_export_settings.sql",
		.check = "SHOW TABLES LIKE 'perl_event_export_settings'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `perl_event_export_settings` (
  `event_id` int(11) NOT NULL,
  `event_description` varchar(150) DEFAULT NULL,
  `export_qglobals` smallint(11) DEFAULT '0',
  `export_mob` smallint(11) DEFAULT '0',
  `export_zone` smallint(11) DEFAULT '0',
  `export_item` smallint(11) DEFAULT '0',
  `export_event` smallint(11) DEFAULT '0',
  PRIMARY KEY (`event_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of perl_event_export_settings
-- ----------------------------
INSERT INTO `perl_event_export_settings` VALUES ('0', 'EVENT_SAY', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('1', 'EVENT_ITEM', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('2', 'EVENT_DEATH', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('3', 'EVENT_SPAWN', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('4', 'EVENT_ATTACK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('5', 'EVENT_COMBAT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('6', 'EVENT_AGGRO', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('7', 'EVENT_SLAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('8', 'EVENT_NPC_SLAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('9', 'EVENT_WAYPOINT_ARRIVE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('10', 'EVENT_WAYPOINT_DEPART', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('11', 'EVENT_TIMER', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('12', 'EVENT_SIGNAL', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('13', 'EVENT_HP', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('14', 'EVENT_ENTER', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('15', 'EVENT_EXIT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('16', 'EVENT_ENTERZONE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('17', 'EVENT_CLICKDOOR', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('18', 'EVENT_LOOT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('19', 'EVENT_ZONE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('20', 'EVENT_LEVEL_UP', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('21', 'EVENT_KILLED_MERIT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('22', 'EVENT_CAST_ON', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('23', 'EVENT_TASKACCEPTED', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('24', 'EVENT_TASK_STAGE_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('25', 'EVENT_TASK_UPDATE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('26', 'EVENT_TASK_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('27', 'EVENT_TASK_FAIL', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('28', 'EVENT_AGGRO_SAY', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('29', 'EVENT_PLAYER_PICKUP', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('30', 'EVENT_POPUPRESPONSE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('31', 'EVENT_ENVIRONMENTAL_DAMAGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('32', 'EVENT_PROXIMITY_SAY', '1', '1', '1', '1', '1');
INSERT INTO `perl_event_export_settings` VALUES ('33', 'EVENT_CAST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('34', 'EVENT_CAST_BEGIN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('35', 'EVENT_SCALE_CALC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('36', 'EVENT_ITEM_ENTER_ZONE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('37', 'EVENT_TARGET_CHANGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('38', 'EVENT_HATE_LIST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('39', 'EVENT_SPELL_EFFECT_CLIENT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('40', 'EVENT_SPELL_EFFECT_NPC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('41', 'EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('42', 'EVENT_SPELL_EFFECT_BUFF_TIC_NPC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('43', 'EVENT_SPELL_FADE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('44', 'EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('45', 'EVENT_COMBINE_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('46', 'EVENT_COMBINE_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('47', 'EVENT_ITEM_CLICK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('48', 'EVENT_ITEM_CLICK_CAST', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('49', 'EVENT_GROUP_CHANGE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('50', 'EVENT_FORAGE_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('51', 'EVENT_FORAGE_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('52', 'EVENT_FISH_START', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('53', 'EVENT_FISH_SUCCESS', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('54', 'EVENT_FISH_FAILURE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('55', 'EVENT_CLICK_OBJECT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('56', 'EVENT_DISCOVER_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('57', 'EVENT_DISCONNECT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('58', 'EVENT_CONNECT', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('59', 'EVENT_ITEM_TICK', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('60', 'EVENT_DUEL_WIN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('61', 'EVENT_DUEL_LOSE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('62', 'EVENT_ENCOUNTER_LOAD', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('63', 'EVENT_ENCOUNTER_UNLOAD', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('64', 'EVENT_SAY', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('65', 'EVENT_DROP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('66', 'EVENT_DESTROY_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('67', 'EVENT_FEIGN_DEATH', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('68', 'EVENT_WEAPON_PROC', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('69', 'EVENT_EQUIP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('70', 'EVENT_UNEQUIP_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('71', 'EVENT_AUGMENT_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('72', 'EVENT_UNAUGMENT_ITEM', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('73', 'EVENT_AUGMENT_INSERT', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('74', 'EVENT_AUGMENT_REMOVE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('75', 'EVENT_ENTER_AREA', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('76', 'EVENT_LEAVE_AREA', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('77', 'EVENT_RESPAWN', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('78', 'EVENT_DEATH_COMPLETE', '1', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('79', 'EVENT_UNHANDLED_OPCODE', '0', '1', '1', '0', '1');
INSERT INTO `perl_event_export_settings` VALUES ('80', 'EVENT_TICK', '0', '1', '1', '0', '1');
)",
	},
	ManifestEntry{
		.version = 9089,
		.description = "2015_11_02_ai_idle_no_spell_recast_default_changes.sql",
		.check = "SELECT * FROM `rule_values` WHERE `rule_name` LIKE '%Spells:AI_IdleNoSpellMinRecast%' AND `rule_value` = '500'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
UPDATE `rule_values` SET `rule_value` = '6000' WHERE `rule_value` = '500' AND `rule_name` = 'Spells:AI_IdleNoSpellMinRecast';
UPDATE `rule_values` SET `rule_value` = '60000' WHERE `rule_value` = '2000' AND `rule_name` = 'Spells:AI_IdleNoSpellMaxRecast';
)",
	},
	ManifestEntry{
		.version = 9090,
		.description = "2015_12_01_spell_scribe_restriction_rule.sql",
		.check = "SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'Character:RestrictSpellScribing'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `rule_values` VALUES ('0', 'Character:RestrictSpellScribing', 'false', 'Restricts spell scribing to allowable races/classes of spell scroll, if true');
INSERT INTO `rule_values` VALUES ('1', 'Character:RestrictSpellScribing', 'false', 'Restricts spell scribing to allowable races/classes of spell scroll, if true');
INSERT INTO `rule_values` VALUES ('2', 'Character:RestrictSpellScribing', 'false', 'Restricts spell scribing to allowable races/classes of spell scroll, if true');
INSERT INTO `rule_values` VALUES ('3', 'Character:RestrictSpellScribing', 'false', 'Restricts spell scribing to allowable races/classes of spell scroll, if true');
INSERT INTO `rule_values` VALUES ('4', 'Character:RestrictSpellScribing', 'false', 'Restricts spell scribing to allowable races/classes of spell scroll, if true');
INSERT INTO `rule_values` VALUES ('5', 'Character:RestrictSpellScribing', 'false', 'Restricts spell scribing to allowable races/classes of spell scroll, if true');
INSERT INTO `rule_values` VALUES ('10', 'Character:RestrictSpellScribing', 'false', 'Restricts spell scribing to allowable races/classes of spell scroll, if true');

)",
	},
	ManifestEntry{
		.version = 9091,
		.description = "2015_12_07_command_settings.sql",
		.check = "SHOW TABLES LIKE 'command_settings'",
		.condition = "empty",
		.match = "",
		.sql = R"(
RENAME TABLE `commands` to `commands_old`;

CREATE TABLE `command_settings` (
	`command` varchar(128) NOT NULL DEFAULT '',
	`access` int(11) NOT NULL DEFAULT '0',
	`aliases` varchar(256) NOT NULL DEFAULT '',
	PRIMARY KEY (`command`),
	UNIQUE KEY `UK_command_settings_1` (`command`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `command_settings` VALUES ('acceptrules', '90', ''),('advnpcspawn', '150', 'advnpc'),('aggro', '80', ''),('aggrozone', '200', ''),('ai', '100', ''),('appearance', '150', ''),('apply_shared_memory', '250', ''),('attack', '150', ''),('augmentitem', '250', 'aug'),('ban', '200', ''),('beard', '80', ''),('beardcolor', '80', ''),('bestz', '80', ''),('bind', '80', ''),('camerashake', '80', ''),('castspell', '90', 'cast'),('chat', '200', ''),('checklos', '50', 'los'),('clearinvsnapshots', '200', ''),('connectworldserver', '200', 'connectworld'),('corpse', '90', ''),('crashtest', '201', ''),('cvs', '80', ''),('damage', '150', ''),('date', '150', ''),('dbspawn2', '100', ''),('delacct', '200', ''),('deletegraveyard', '200', ''),('delpetition', '80', ''),('depop', '100', ''),('depopzone', '100', ''),('details', '80', ''),('disablerecipe', '80', ''),('disarmtrap', '80', ''),('distance', '80', ''),('doanim', '50', ''),('emote', '150', ''),('emotesearch', '80', ''),('emoteview', '80', ''),('enablerecipe', '80', ''),('equipitem', '50', ''),('face', '80', ''),('findnpctype', '90', 'fn'),('findspell', '90', 'fs|spfind'),('findzone', '1', 'fz'),('fixmob', '150', ''),('flag', '201', ''),('flagedit', '150', ''),('flags', '80', ''),('flymode', '80', ''),('fov', '80', ''),('freeze', '100', ''),('gassign', '150', ''),('gender', '90', ''),('getplayerburiedcorpsecount', '100', ''),('getvariable', '200', ''),('ginfo', '20', ''),('giveitem', '150', 'gi'),('givemoney', '150', ''),('globalview', '80', ''),('gm', '80', ''),('gmspeed', '80', ''),('goto', '80', ''),('grid', '150', ''),('guild', '80', 'guilds'),('guildapprove', '0', ''),('guildcreate', '0', ''),('guildlist', '0', ''),('hair', '80', ''),('haircolor', '80', ''),('haste', '100', ''),('hatelist', '80', ''),('heal', '100', ''),('helm', '80', ''),('help', '0', ''),('heritage', '80', ''),('heromodel', '200', 'hm'),('hideme', '80', 'gmhideme'),('hotfix', '250', ''),('hp', '90', ''),('incstat', '200', ''),('instance', '80', ''),('interrogateinv', '0', ''),('interrupt', '50', ''),('invsnapshot', '80', ''),('invul', '80', 'invulnerable'),('ipban', '201', ''),('iplookup', '200', ''),('iteminfo', '10', ''),('itemsearch', '90', 'fi|finditem|search'),('kick', '80', ''),('kill', '80', ''),('lastname', '80', ''),('level', '150', ''),('listnpcs', '90', ''),('listpetition', '80', ''),('load_shared_memory', '250', ''),('loc', '0', ''),('lock', '200', ''),('logs', '250', ''),('logtest', '250', ''),('makepet', '150', ''),('mana', '100', ''),('maxskills', '90', ''),('memspell', '100', ''),('merchant_close_shop', '100', 'close_shop'),('merchant_open_shop', '100', 'open_shop'),('modifynpcstat', '150', ''),('motd', '200', ''),('movechar', '80', ''),('myskills', '0', ''),('mysqltest', '250', ''),('mysql', '255', ''),('mystats', '50', ''),('name', '100', ''),('netstats', '200', ''),('npccast', '90', ''),('npcedit', '150', ''),('npcemote', '80', ''),('npcloot', '150', ''),('npcsay', '80', ''),('npcshout', '90', ''),('npcspawn', '100', ''),('npcspecialattk', '150', 'npcspecialatk|npcspecialattack'),('npcstats', '90', ''),('npctype_cache', '250', ''),('npctypespawn', '90', 'dbspawn'),('nukebuffs', '100', ''),('nukeitem', '150', ''),('object', '100', ''),('oocmute', '200', ''),('opcode', '250', ''),('path', '200', ''),('peekinv', '80', ''),('peqzone', '2', ''),('permaclass', '150', ''),('permagender', '150', ''),('permarace', '150', ''),('petitioninfo', '20', ''),('pf', '0', ''),('picklock', '0', ''),('pvp', '80', ''),('qglobal', '150', ''),('questerrors', '0', ''),('race', '90', ''),('raidloot', '0', ''),('randomfeatures', '90', ''),('refreshgroup', '0', ''),('reloadaa', '200', ''),('reloadallrules', '80', ''),('reloademote', '80', ''),('reloadlevelmods', '255', ''),('reloadperlexportsettings', '255', ''),('reloadqst', '80', 'reloadquest|rq'),('reloadrulesworld', '80', ''),('reloadstatic', '150', ''),('reloadtitles', '150', ''),('reloadworld', '255', ''),('reloadzps', '150', 'reloadzonepoints'),('repop', '90', ''),('repopclose', '100', ''),('resetaa', '100', ''),('resetaa_timer', '200', ''),('revoke', '80', ''),('rules', '200', ''),('save', '80', ''),('scribespell', '90', ''),('scribespells', '100', ''),('sendzonespawns', '200', ''),('sensetrap', '0', ''),('serverinfo', '201', ''),('serverrules', '90', ''),('setaapts', '100', 'setaapoints'),('setaaxp', '100', 'setaaexp'),('setadventurepoints', '200', ''),('setanim', '200', ''),('setcrystals', '100', ''),('setfaction', '170', ''),('setgraveyard', '200', ''),('setlanguage', '50', ''),('setlsinfo', '0', ''),('setpass', '150', ''),('setpvppoints', '100', ''),('setskill', '90', ''),('setskillall', '100', 'setallskill|setallskills'),('setstartzone', '80', ''),('setstat', '255', ''),('setxp', '100', 'setexp'),('showbonusstats', '50', ''),('showbuffs', '80', ''),('shownumhits', '0', ''),('showskills', '50', ''),('showspellslist', '100', ''),('showstats', '80', ''),('shutdown', '200', ''),('size', '90', ''),('spawn', '150', ''),('spawnfix', '80', ''),('spawnstatus', '150', ''),('spellinfo', '10', ''),('spoff', '0', ''),('spon', '0', ''),('stun', '100', ''),('summon', '80', ''),('summonburiedplayercorpse', '100', ''),('summonitem', '150', 'si'),('suspend', '100', ''),('task', '150', ''),('tattoo', '80', ''),('tempname', '100', ''),('texture', '150', ''),('time', '90', ''),('timers', '200', ''),('timezone', '90', ''),('title', '100', ''),('titlesuffix', '50', ''),('traindisc', '100', ''),('tune', '100', ''),('undyeme', '0', ''),('unfreeze', '100', ''),('unlock', '150', ''),('unscribespell', '90', ''),('unscribespells', '100', ''),('untraindisc', '180', ''),('untraindiscs', '180', ''),('uptime', '10', ''),('version', '0', ''),('viewnpctype', '100', ''),('viewpetition', '80', ''),('wc', '200', ''),('weather', '90', ''),('worldshutdown', '200', ''),('wp', '150', ''),('wpadd', '150', ''),('wpinfo', '150', ''),('xtargets', '250', ''),('zclip', '150', ''),('zcolor', '150', ''),('zheader', '150', ''),('zone', '80', ''),('zonebootup', '100', ''),('zoneinstance', '80', ''),('zonelock', '200', ''),('zoneshutdown', '200', ''),('zonespawn', '250', ''),('zonestatus', '150', ''),('zopp', '250', ''),('zsafecoords', '150', ''),('zsave', '200', ''),	('zsky', '150', ''),('zstats', '80', ''),('zunderworld', '80', ''),('zuwcoords', '80', '');

)",
	},
	ManifestEntry{
		.version = 9092,
		.description = "2015_12_17_eqtime.sql",
		.check = "SHOW TABLES LIKE 'eqtime'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `eqtime`;
CREATE TABLE `eqtime` (
  `minute` tinyint(4) not null default 0,
  `hour` tinyint(4) not null default 0,
  `day` tinyint(4) not null default 0,
  `month` tinyint(4) not null default 0,
  `year` int(4) not null default 0,
  `realtime` int(11) not null default 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

INSERT INTO eqtime values (0,1,28,12,3766,1444035661);
)",
	},
	ManifestEntry{
		.version = 9093,
		.description = "2015_12_21_items_updates_evoitem.sql",
		.check = "SHOW COLUMNS FROM `items` LIKE 'evoitem'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `items`
	ADD COLUMN `evoitem` INT(11) NOT NULL DEFAULT '0' AFTER `purity`,
	ADD COLUMN `evoid` INT(11) NOT NULL DEFAULT '0' AFTER `evoitem`,
	ADD COLUMN `evomax` INT(11) NOT NULL DEFAULT '0' AFTER `evolvinglevel`,
	CHANGE `UNK038` `skillmodmax` INT(11) NOT NULL DEFAULT '0',
	CHANGE `UNK222` `heirloom` INT(11) NOT NULL DEFAULT '0',
	CHANGE `UNK235` `placeable` INT(11) NOT NULL DEFAULT '0',
	CHANGE `UNK242` `epicitem` INT(11) NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9094,
		.description = "2015_12_29_quest_zone_events.sql",
		.check = "SELECT * FROM perl_event_export_settings WHERE event_description = 'EVENT_SPAWN_ZONE'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `perl_event_export_settings` (`event_id`, `event_description`, `export_qglobals`, `export_mob`, `export_zone`, `export_item`, `export_event`) VALUES (81, 'EVENT_SPAWN_ZONE', 0, 0, 0, 0, 1);
INSERT INTO `perl_event_export_settings` (`event_id`, `event_description`, `export_qglobals`, `export_mob`, `export_zone`, `export_item`, `export_event`) VALUES (82, 'EVENT_DEATH_ZONE', 0, 0, 0, 0, 1);
ALTER TABLE `rule_values`
MODIFY COLUMN `notes`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL AFTER `rule_value`;
)",
	},
	ManifestEntry{
		.version = 9095,
		.description = "2016_01_08_command_find_aliases.sql",
		.check = "SELECT * FROM `command_settings` WHERE `command` LIKE 'findaliases'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `command_settings` VALUES ('findaliases', 0, 'fa');

)",
	},
	ManifestEntry{
		.version = 9096,
		.description = "2016_03_05_secondary_recall.sql",
		.check = "SHOW COLUMNS FROM `character_bind` LIKE 'slot'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_bind` ADD `slot` int(4) AFTER `id`;
UPDATE `character_bind` SET `slot`='0' WHERE `is_home`=0;
UPDATE `character_bind` SET `slot`='4' WHERE `is_home`=1;
ALTER TABLE `character_bind` DROP PRIMARY KEY, ADD PRIMARY KEY(`id`, `slot`);
ALTER TABLE `character_bind` DROP COLUMN `is_home`;


)",
	},
	ManifestEntry{
		.version = 9097,
		.description = "2016_07_03_npc_class_as_last_name.sql",
		.check = "SELECT `rule_name` FROM `rule_values` WHERE `rule_name` LIKE 'NPC:UseClassAsLastName'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES
(1, 'NPC:UseClassAsLastName', 'true', 'Uses class archetype as LastName for npcs with none');

)",
	},
	ManifestEntry{
		.version = 9098,
		.description = "2016_08_26_object_size_tilt.sql",
		.check = "SHOW COLUMNS FROM `object` LIKE 'size'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `object`
	ADD COLUMN `size` FLOAT NOT NULL DEFAULT '100' AFTER `unknown84`,
	ADD COLUMN `tilt_x` FLOAT NOT NULL DEFAULT '0' AFTER `size`,
	ADD COLUMN `tilt_y` FLOAT NOT NULL DEFAULT '0' AFTER `tilt_x`;
)",
	},
	ManifestEntry{
		.version = 9099,
		.description = "2016_08_27_ip_exemptions.sql",
		.check = "SHOW TABLES LIKE 'ip_exemptions'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- IP Exemptions table structure
DROP TABLE IF EXISTS `ip_exemptions`;
CREATE TABLE `ip_exemptions` (
  `exemption_id` int(11) NOT NULL AUTO_INCREMENT,
  `exemption_ip` varchar(255) DEFAULT NULL,
  `exemption_amount` int(11) DEFAULT NULL,
  PRIMARY KEY (`exemption_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

-- Rule Value Entry, Default to false
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES ('1', 'World:EnableIPExemptions', 'false', 'notation');

-- Logging Category Entry
INSERT INTO `logsys_categories` (`log_category_id`, `log_category_description`, `log_to_console`, `log_to_file`, `log_to_gmsay`) VALUES ('44', 'Client Login', '1', '1', '1');
)",
	},
	ManifestEntry{
		.version = 9100,
		.description = "2016_08_27_object_display_name.sql",
		.check = "SHOW COLUMNS FROM `object` LIKE 'display_name'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `object` ADD COLUMN `display_name` VARCHAR(64);

)",
	},
	ManifestEntry{
		.version = 9101,
		.description = "2016_12_01_pcnpc_only.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'pcnpc_only_flag'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field183` `pcnpc_only_flag` INT(11) DEFAULT 0;
ALTER TABLE `spells_new` CHANGE `field184` `cast_not_standing` INT(11) DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9102,
		.description = "2017_01_10_book_languages.sql",
		.check = "SHOW COLUMNS FROM `books` LIKE 'language'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table books add language int not null default 0;

drop table if exists reading_is_fundamental;

create table reading_is_fundamental
(
filename varchar(32),
language int
);

insert into reading_is_fundamental (select items.filename, items.booktype from items where items.filename  != "" group by filename);

update books set books.language = (select language from reading_is_fundamental r where r.filename = books.name);

drop table reading_is_fundamental;

)",
	},
	ManifestEntry{
		.version = 9103,
		.description = "2017_01_30_book_languages_fix.sql",
		.check = "SELECT `language` from `books` WHERE `language` IS NULL",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
UPDATE `books` SET `language` = '0' WHERE `language` IS NULL;

ALTER TABLE `books` MODIFY COLUMN `language` INT NOT NULL DEFAULT '0';


)",
	},
	ManifestEntry{
		.version = 9104,
		.description = "2017_02_09_npc_spells_entries_type_update.sql",
		.check = "SHOW COLUMNS IN `npc_spells_entries` LIKE 'type'",
		.condition = "contains",
		.match = "smallint(5) unsigned",
		.sql = R"(
ALTER TABLE `npc_spells_entries` MODIFY COLUMN `type` INT(10) UNSIGNED NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9105,
		.description = "2017_02_15_bot_spells_entries.sql",
		.check = "SELECT `id` FROM `npc_spells_entries` WHERE `npc_spells_id` >= 701 AND `npc_spells_id` <= 712",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
-- Delete any existing `bots_spells_entries` table
DROP TABLE IF EXISTS `bots_spells_entries`;

-- Create new bot spells entries table (new table does not have spells_id_spellid constraint)
CREATE TABLE `bot_spells_entries` (
	`id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`npc_spells_id` INT(11) NOT NULL DEFAULT '0',
	`spellid` SMALLINT(5) NOT NULL DEFAULT '0',
	`type` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	`minlevel` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`maxlevel` TINYINT(3) UNSIGNED NOT NULL DEFAULT '255',
	`manacost` SMALLINT(5) NOT NULL DEFAULT '-1',
	`recast_delay` INT(11) NOT NULL DEFAULT '-1',
	`priority` SMALLINT(5) NOT NULL DEFAULT '0',
	`resist_adjust` INT(11) NULL DEFAULT NULL,
	PRIMARY KEY (`id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1
;

-- Copy bots spells into new table
INSERT INTO `bot_spells_entries` (`npc_spells_id`, `spellid`, `type`, `minlevel`, `maxlevel`, `manacost`, `recast_delay`, `priority`, `resist_adjust`)
SELECT `npc_spells_id`, `spellid`, `type`, `minlevel`, `maxlevel`, `manacost`, `recast_delay`, `priority`, `resist_adjust`
FROM `npc_spells_entries` WHERE `npc_spells_id` >= '701' AND `npc_spells_id` <= '712';

-- Delete bot spells from old table
DELETE FROM `npc_spells_entries` WHERE `npc_spells_id` >= '701' AND `npc_spells_id` <= '712';

-- Admins can remove this new table if they are 100% certain they will never use bots

)",
	},
	ManifestEntry{
		.version = 9106,
		.description = "2017_02_26_npc_spells_update_for_bots.sql",
		.check = "SELECT * FROM `npc_spells` WHERE `id` = '701' AND `name` = 'Cleric Bot'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
-- Re-ordered entries according to actual class values and added melee types (for future expansion)
DELETE FROM `npc_spells` WHERE `id` >= '701' AND `id` <= '712';

INSERT INTO `npc_spells` VALUES (3001, 'Warrior Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3002, 'Cleric Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3003, 'Paladin Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3004, 'Ranger Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3005, 'Shadowknight Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3006, 'Druid Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3007, 'Monk Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3008, 'Bard Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3009, 'Rogue Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3010, 'Shaman Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3011, 'Necromancer Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3012, 'Wizard Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3013, 'Magician Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3014, 'Enchanter Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3015, 'Beastlord Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_spells` VALUES (3016, 'Berserker Bot', 0, -1, 3, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

)",
	},
	ManifestEntry{
		.version = 9107,
		.description = "2017_03_09_inventory_version.sql",
		.check = "SHOW TABLES LIKE 'inventory_version'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `inventory_version`;

CREATE TABLE `inventory_version` (
	`version` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`step` INT(11) UNSIGNED NOT NULL DEFAULT '0'
)
COLLATE='latin1_swedish_ci'
ENGINE=MyISAM
;

INSERT INTO `inventory_version` VALUES (2, 0);

)",
	},
	ManifestEntry{
		.version = 9108,
		.description = "2017_04_07_ignore_despawn.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'ignore_despawn'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table npc_types add column `ignore_despawn` tinyint(2) not null default 0;
)",
	},
	ManifestEntry{
		.version = 9109,
		.description = "2017_04_08_doors_disable_timer.sql",
		.check = "SHOW COLUMNS FROM `doors` LIKE 'disable_timer'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `doors` ADD COLUMN `disable_timer` TINYINT(2) NOT NULL DEFAULT '0' AFTER `triggertype`;
)",
	},
	ManifestEntry{
		.version = 9110,
		.description = "2017_04_10_graveyard.sql",
		.check = "show index from graveyard WHERE key_name = 'zone_id_nonunique'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table graveyard drop index zone_id;
create index zone_id_nonunique on graveyard(zone_id);

)",
	},
	ManifestEntry{
		.version = 9111,
		.description = "2017_06_24_saylink_index.sql",
		.check = "SHOW INDEX FROM `saylink` WHERE `key_name` = 'phrase_index'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `saylink`
ADD INDEX `phrase_index` (`phrase`) USING BTREE ;
)",
	},
	ManifestEntry{
		.version = 9112,
		.description = "2017_06_24_rule_values_expand.sql",
		.check = "SHOW COLUMNS FROM rule_values WHERE Field = 'rule_value' and Type = 'varchar(30)'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `rule_values`
MODIFY COLUMN `rule_value`  varchar(30) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `rule_name`;

)",
	},
	ManifestEntry{
		.version = 9113,
		.description = "2017_07_19_show_name.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'show_name'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `show_name` TINYINT(2) NOT NULL DEFAULT 1;
ALTER TABLE `npc_types` ADD COLUMN `untargetable` TINYINT(2) NOT NULL DEFAULT 0;
UPDATE `npc_types` SET `show_name` = 0, `untargetable` = 1 WHERE `bodytype` >= 66;

)",
	},
	ManifestEntry{
		.version = 9114,
		.description = "2017_07_22_aura.sql",
		.check = "SHOW TABLES LIKE 'auras'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `auras` (
	`type` INT(10) NOT NULL,
	`npc_type` INT(10) NOT NULL,
	`name` VARCHAR(64) NOT NULL,
	`spell_id` INT(10) NOT NULL,
	`distance` INT(10) NOT NULL DEFAULT 60,
	`aura_type` INT(10) NOT NULL DEFAULT 1,
	`spawn_type` INT(10) NOT NULL DEFAULT 0,
	`movement` INT(10) NOT NULL DEFAULT 0,
	`duration` INT(10) NOT NULL DEFAULT 5400,
	`icon` INT(10) NOT NULL DEFAULT -1,
	`cast_time` INT(10) NOT NULL DEFAULT 0,
	PRIMARY KEY(`type`)
);

CREATE TABLE `character_auras` (
	`id` INT(10) NOT NULL,
	`slot` TINYINT(10) NOT NULL,
	`spell_id` INT(10) NOT NULL,
	PRIMARY KEY (`id`, `slot`)
);

)",
	},
	ManifestEntry{
		.version = 9115,
		.description = "2017_10_28_traps.sql",
		.check = "SHOW COLUMNS FROM `traps` LIKE 'triggered_number'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table `traps` add column `triggered_number` tinyint(4) not null default 0;
alter table `traps` add column `group` tinyint(4) not null default 0;
alter table `traps` add column `despawn_when_triggered` tinyint(4) not null default 0;
alter table `traps` add column `undetectable` tinyint(4) not null default 0;

)",
	},
	ManifestEntry{
		.version = 9116,
		.description = "2017_12_16_groundspawn_respawn_timer.sql",
		.check = "SHOW COLUMNS FROM `ground_spawns` WHERE Field = 'respawn_timer' AND Type = 'int(11) unsigned'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `ground_spawns` MODIFY `respawn_timer` int(11) unsigned NOT NULL default 300;
UPDATE `ground_spawns` SET `respawn_timer` = `respawn_timer` / 1000;

)",
	},
	ManifestEntry{
		.version = 9117,
		.description = "2018_02_01_npc_spells_min_max_hp.sql",
		.check = "SHOW COLUMNS FROM `npc_spells_entries` LIKE 'min_hp'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_spells_entries` ADD `min_hp` SMALLINT(5) DEFAULT '0';
ALTER TABLE `npc_spells_entries` ADD `max_hp` SMALLINT(5) DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9118,
		.description = "2018_02_04_charm_stats.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'charm_ac'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `charm_ac` SMALLINT(5) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_min_dmg` INT(10) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_max_dmg` INT(10) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_attack_delay` TINYINT(3) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_accuracy_rating` MEDIUMINT(9) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_avoidance_rating` MEDIUMINT(9) DEFAULT '0';
ALTER TABLE `npc_types` ADD `charm_atk` MEDIUMINT(9) DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9119,
		.description = "2018_02_10_globalloot.sql",
		.check = "SHOW TABLES LIKE 'global_loot'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD `skip_global_loot` TINYINT DEFAULT '0';
ALTER TABLE `npc_types` ADD `rare_spawn` TINYINT DEFAULT '0';

CREATE TABLE global_loot (
	id INT NOT NULL AUTO_INCREMENT,
	description varchar(255),
	loottable_id INT NOT NULL,
	enabled TINYINT NOT NULL DEFAULT 1,
	min_level INT NOT NULL DEFAULT 0,
	max_level INT NOT NULL DEFAULT 0,
	rare TINYINT NULL,
	raid TINYINT NULL,
	race MEDIUMTEXT NULL,
	class MEDIUMTEXT NULL,
	bodytype MEDIUMTEXT NULL,
	zone MEDIUMTEXT NULL,
	PRIMARY KEY (id)
);


)",
	},
	ManifestEntry{
		.version = 9120,
		.description = "2018_02_13_heading.sql",
		.check = "SELECT value FROM variables WHERE varname = 'fixed_heading'",
		.condition = "empty",
		.match = "",
		.sql = R"(
UPDATE spawn2 SET heading = heading * 8.0 / 4.0;
UPDATE grid_entries SET heading = heading * 8.0 / 4.0 WHERE heading <> -1;
INSERT INTO variables (varname, value, information) VALUES ('fixed_heading', 1, 'manifest heading fix hack'); -- hack

)",
	},
	ManifestEntry{
		.version = 9121,
		.description = "2018_02_18_bug_reports.sql",
		.check = "SHOW TABLES LIKE 'bug_reports'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `bug_reports` (
	`id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`zone` VARCHAR(32) NOT NULL DEFAULT 'Unknown',
	`client_version_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`client_version_name` VARCHAR(24) NOT NULL DEFAULT 'Unknown',
	`account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`character_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`character_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`reporter_spoof` TINYINT(1) NOT NULL DEFAULT '1',
	`category_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`category_name` VARCHAR(64) NOT NULL DEFAULT 'Other',
	`reporter_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`ui_path` VARCHAR(128) NOT NULL DEFAULT 'Unknown',
	`pos_x` FLOAT NOT NULL DEFAULT '0',
	`pos_y` FLOAT NOT NULL DEFAULT '0',
	`pos_z` FLOAT NOT NULL DEFAULT '0',
	`heading` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`time_played` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`target_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`target_name` VARCHAR(64) NOT NULL DEFAULT 'Unknown',
	`optional_info_mask` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`_can_duplicate` TINYINT(1) NOT NULL DEFAULT '0',
	`_crash_bug` TINYINT(1) NOT NULL DEFAULT '0',
	`_target_info` TINYINT(1) NOT NULL DEFAULT '0',
	`_character_flags` TINYINT(1) NOT NULL DEFAULT '0',
	`_unknown_value` TINYINT(1) NOT NULL DEFAULT '0',
	`bug_report` VARCHAR(1024) NOT NULL DEFAULT '',
	`system_info` VARCHAR(1024) NOT NULL DEFAULT '',
	`report_datetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
	`bug_status` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`last_review` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	`last_reviewer` VARCHAR(64) NOT NULL DEFAULT 'None',
	`reviewer_notes` VARCHAR(1024) NOT NULL DEFAULT '',
	PRIMARY KEY (`id`),
	UNIQUE INDEX `id` (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES
(1, 'Bugs:ReportingSystemActive', 'true', 'Activates bug reporting'),
(1, 'Bugs:UseOldReportingMethod', 'true', 'Forces the use of the old bug reporting system'),
(1, 'Bugs:DumpTargetEntity', 'false', 'Dumps the target entity, if one is provided');

)",
	},
	ManifestEntry{
		.version = 9122,
		.description = "2018_03_07_ucs_command.sql",
		.check = "SELECT * FROM `command_settings` WHERE `command` LIKE 'ucs'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `command_settings` VALUES ('ucs', '0', '');

)",
	},
	ManifestEntry{
		.version = 9123,
		.description = "2018_07_07_data_buckets.sql",
		.check = "SHOW TABLES LIKE 'data_buckets'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `data_buckets` (
  `id` bigint(11) unsigned NOT NULL AUTO_INCREMENT,
  `key` varchar(100) DEFAULT NULL,
  `value` text,
  `expires` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `key_index` (`key`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;
)",
	},
	ManifestEntry{
		.version = 9124,
		.description = "2018_07_09_tasks.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'type'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks` ADD `type` TINYINT NOT NULL DEFAULT '0' AFTER `id`;
ALTER TABLE `tasks` ADD `duration_code` TINYINT NOT NULL DEFAULT '0' AFTER `duration`;
UPDATE `tasks` SET `type` = '2'; -- we were treating them all as quests
ALTER TABLE `character_tasks` ADD `type` TINYINT NOT NULL DEFAULT '0' AFTER `slot`;
UPDATE `character_tasks` SET `type` = '2'; -- we were treating them all as quests
ALTER TABLE `activities` ADD `target_name` VARCHAR(64) NOT NULL DEFAULT '' AFTER `activitytype`;
ALTER TABLE `activities` ADD `item_list` VARCHAR(128) NOT NULL DEFAULT '' AFTER `target_name`;
ALTER TABLE `activities` ADD `skill_list` VARCHAR(64) NOT NULL DEFAULT '-1' AFTER `item_list`;
ALTER TABLE `activities` ADD `spell_list` VARCHAR(64) NOT NULL DEFAULT '0' AFTER `skill_list`;
ALTER TABLE `activities` ADD `description_override` VARCHAR(128) NOT NULL DEFAULT '' AFTER `spell_list`;
ALTER TABLE `activities` ADD `zones` VARCHAR(64) NOT NULL DEFAULT '' AFTER `zoneid`;
UPDATE `activities` SET `description_override` = `text3`;
UPDATE `activities` SET `target_name` = `text1`;
UPDATE `activities` SET `item_list` = `text2`;
UPDATE `activities` SET `zones` = `zoneid`; -- should be safe for us ...
ALTER TABLE `activities` DROP COLUMN `text1`;
ALTER TABLE `activities` DROP COLUMN `text2`;
ALTER TABLE `activities` DROP COLUMN `text3`;
ALTER TABLE `activities` DROP COLUMN `zoneid`;
ALTER TABLE `tasks` DROP COLUMN `startzone`;
ALTER TABLE `tasks` ADD `faction_reward` INT(10) NOT NULL DEFAULT '0';
RENAME TABLE `activities` TO `task_activities`;
)",
	},
	ManifestEntry{
		.version = 9125,
		.description = "2018_07_20_task_emote.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'completion_emote'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks` ADD `completion_emote` VARCHAR(128) NOT NULL DEFAULT '';

)",
	},
	ManifestEntry{
		.version = 9126,
		.description = "2018_09_07_fastregen.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'fast_regen_hp'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD `fast_regen_hp` INT NOT NULL DEFAULT '180';
ALTER TABLE `zone` ADD `fast_regen_mana` INT NOT NULL DEFAULT '180';
ALTER TABLE `zone` ADD `fast_regen_endurance` INT NOT NULL DEFAULT '180';

)",
	},
	ManifestEntry{
		.version = 9127,
		.description = "2018_09_07_npcmaxaggrodist.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'npc_max_aggro_dist'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD `npc_max_aggro_dist` INT NOT NULL DEFAULT '600';

)",
	},
	ManifestEntry{
		.version = 9128,
		.description = "2018_08_13_inventory_version_update.sql",
		.check = "SHOW TABLES LIKE 'inventory_version'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `inventory_version`;

)",
	},
	ManifestEntry{
		.version = 9130,
		.description = "2018_11_25_name_filter_update.sql",
		.check = "SHOW COLUMNS FROM `name_filter` LIKE 'id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `name_filter`
ADD COLUMN `id` INT(11) NULL AUTO_INCREMENT FIRST,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`) USING BTREE,
ADD INDEX `name_search_index`(`name`);

)",
	},
	ManifestEntry{
		.version = 9131,
		.description = "2018_12_13_spell_buckets.sql",
		.check = "SHOW TABLES LIKE 'spell_buckets'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `spell_buckets` (
  `spellid` bigint(11) unsigned NOT NULL,
  `key` varchar(100) DEFAULT NULL,
  `value` text,
  PRIMARY KEY (`spellid`),
  KEY `key_index` (`key`) USING BTREE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8mb4;

INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Spells:EnableSpellBuckets', 'false', 'Enables spell buckets');
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Character:PerCharacterBucketMaxLevel', 'false', 'Enables data bucket-based max level.');
)",
	},
	ManifestEntry{
		.version = 9132,
		.description = "2018_12_16_global_base_scaling.sql",
		.check = "SHOW TABLES LIKE 'npc_scale_global_base'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- INSERT #devtools / #dev command

INSERT INTO `command_settings` (`command`, `access`, `aliases`)
VALUES
	('devtools', 200, 'dev');

-- CREATE 'npc_scale_global_base'

CREATE TABLE `npc_scale_global_base` (
  `type` int(11) NOT NULL DEFAULT '0',
  `level` int(11) NOT NULL,
  `ac` int(11) DEFAULT NULL,
  `hp` int(11) DEFAULT NULL,
  `accuracy` int(11) DEFAULT NULL,
  `slow_mitigation` int(11) DEFAULT NULL,
  `attack` int(11) DEFAULT NULL,
  `strength` int(11) DEFAULT NULL,
  `stamina` int(11) DEFAULT NULL,
  `dexterity` int(11) DEFAULT NULL,
  `agility` int(11) DEFAULT NULL,
  `intelligence` int(11) DEFAULT NULL,
  `wisdom` int(11) DEFAULT NULL,
  `charisma` int(11) DEFAULT NULL,
  `magic_resist` int(11) DEFAULT NULL,
  `cold_resist` int(11) DEFAULT NULL,
  `fire_resist` int(11) DEFAULT NULL,
  `poison_resist` int(11) DEFAULT NULL,
  `disease_resist` int(11) DEFAULT NULL,
  `corruption_resist` int(11) DEFAULT NULL,
  `physical_resist` int(11) DEFAULT NULL,
  `min_dmg` int(11) DEFAULT NULL,
  `max_dmg` int(11) DEFAULT NULL,
  `hp_regen_rate` int(11) DEFAULT NULL,
  `attack_delay` int(11) DEFAULT NULL,
  `spell_scale` int(11) DEFAULT '100',
  `heal_scale` int(11) DEFAULT '100',
  `special_abilities` text,
  PRIMARY KEY (`type`,`level`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- INSERT 'npc_scale_global_base'

INSERT INTO `npc_scale_global_base` (`type`, `level`, `ac`, `hp`, `accuracy`, `slow_mitigation`, `attack`, `strength`, `stamina`, `dexterity`, `agility`, `intelligence`, `wisdom`, `charisma`, `magic_resist`, `cold_resist`, `fire_resist`, `poison_resist`, `disease_resist`, `corruption_resist`, `physical_resist`, `min_dmg`, `max_dmg`, `hp_regen_rate`, `attack_delay`, `spell_scale`, `heal_scale`, `special_abilities`)
VALUES
	(0,1,8,11,0,0,0,8,8,8,8,8,8,8,1,1,1,1,1,1,10,1,6,1,30,100,100,''),
	(0,2,11,27,0,0,0,11,11,11,11,11,11,11,1,1,1,1,1,2,10,1,8,2,30,100,100,''),
	(0,3,14,43,0,0,0,14,14,14,14,14,14,14,2,2,2,2,2,2,10,1,10,3,30,100,100,''),
	(0,4,16,59,0,0,0,17,17,17,17,17,17,17,2,2,2,2,2,3,10,1,12,4,30,100,100,''),
	(0,5,19,75,0,0,0,20,20,20,20,20,20,20,2,2,2,2,2,3,10,1,14,5,30,100,100,''),
	(0,6,22,100,0,0,0,23,23,23,23,23,23,23,2,2,2,2,2,4,10,1,16,6,30,100,100,''),
	(0,7,25,125,0,0,0,26,26,26,26,26,26,26,3,3,3,3,3,4,10,1,18,7,30,100,100,''),
	(0,8,28,150,0,0,0,29,29,29,29,29,29,29,3,3,3,3,3,5,10,1,20,8,30,100,100,''),
	(0,9,31,175,0,0,0,32,32,32,32,32,32,32,4,4,4,4,4,5,10,1,22,9,30,100,100,''),
	(0,10,34,200,0,0,0,35,35,35,35,35,35,35,4,4,4,4,4,6,10,1,24,10,30,100,100,''),
	(0,11,37,234,0,0,0,38,38,38,38,38,38,38,5,5,5,5,5,7,10,3,27,11,30,100,100,''),
	(0,12,40,268,0,0,0,42,42,42,42,42,42,42,5,5,5,5,5,8,10,4,30,12,30,100,100,''),
	(0,13,43,302,0,0,0,45,45,45,45,45,45,45,6,6,6,6,6,8,10,6,32,13,30,100,100,''),
	(0,14,46,336,0,0,0,48,48,48,48,48,48,48,6,6,6,6,6,9,10,7,35,14,30,100,100,''),
	(0,15,52,381,0,0,0,51,51,51,51,51,51,51,6,6,6,6,6,10,10,7,37,15,30,100,100,''),
	(0,16,59,426,0,0,0,54,54,54,54,54,54,54,7,7,7,7,7,10,10,8,39,16,30,100,100,''),
	(0,17,65,471,0,0,0,57,57,57,57,57,57,57,7,7,7,7,7,11,10,8,41,17,30,100,100,''),
	(0,18,72,516,0,0,0,60,60,60,60,60,60,60,7,7,7,7,7,11,10,9,42,18,30,100,100,''),
	(0,19,78,561,0,0,0,63,63,63,63,63,63,63,7,7,7,7,7,12,10,9,44,19,30,100,100,''),
	(0,20,85,606,0,0,0,66,66,66,66,66,66,66,8,8,8,8,8,12,10,10,46,20,30,100,100,''),
	(0,21,91,651,0,0,0,69,69,69,69,69,69,69,8,8,8,8,8,13,10,10,48,21,30,100,100,''),
	(0,22,95,712,0,0,0,72,72,72,72,72,72,72,8,8,8,8,8,14,10,10,50,22,30,100,100,''),
	(0,23,99,773,0,0,0,75,75,75,75,75,75,75,9,9,9,9,9,14,10,10,52,23,30,100,100,''),
	(0,24,103,834,0,0,0,78,78,78,78,78,78,78,9,9,9,9,9,15,10,11,55,24,30,100,100,''),
	(0,25,107,895,0,0,0,81,81,81,81,81,81,81,10,10,10,10,10,16,10,11,57,25,30,100,100,''),
	(0,26,111,956,0,0,0,85,85,85,85,85,85,85,10,10,10,10,10,16,10,11,59,26,30,100,100,''),
	(0,27,115,1017,0,0,0,88,88,88,88,88,88,88,11,11,11,11,11,17,10,11,61,27,30,100,100,''),
	(0,28,119,1078,0,0,0,91,91,91,91,91,91,91,11,11,11,11,11,18,10,12,64,28,30,100,100,''),
	(0,29,123,1139,0,0,0,94,94,94,94,94,94,94,12,12,12,12,12,18,10,12,66,29,30,100,100,''),
	(0,30,127,1200,0,0,0,97,97,97,97,97,97,97,12,12,12,12,12,19,10,12,68,30,30,100,100,''),
	(0,31,135,1580,4,0,4,104,104,104,104,104,104,104,13,13,13,13,13,20,10,14,74,31,30,100,100,''),
	(0,32,142,1960,8,0,8,110,110,110,110,110,110,110,14,14,14,14,14,22,10,16,79,32,30,100,100,''),
	(0,33,150,2340,12,0,12,117,117,117,117,117,117,117,15,15,15,15,15,23,10,18,85,33,29,100,100,''),
	(0,34,158,2720,16,0,16,123,123,123,123,123,123,123,16,16,16,16,16,25,10,20,90,34,28,100,100,''),
	(0,35,166,3100,20,0,20,130,130,130,130,130,130,130,17,17,17,17,17,26,10,22,96,35,27,100,100,''),
	(0,36,173,3480,24,0,24,137,137,137,137,137,137,137,17,17,17,17,17,27,10,24,102,36,25,100,100,''),
	(0,37,181,3860,28,0,28,143,143,143,143,143,143,143,18,18,18,18,18,29,10,26,107,37,24,100,100,''),
	(0,38,189,4240,32,0,32,150,150,150,150,150,150,150,19,19,19,19,19,30,10,28,113,38,23,100,100,''),
	(0,39,196,4620,36,0,36,156,156,156,156,156,156,156,20,20,20,20,20,32,10,30,118,39,22,100,100,''),
	(0,40,204,5000,40,0,40,163,163,163,163,163,163,163,21,21,21,21,21,33,10,32,124,40,21,100,100,''),
	(0,41,208,5300,42,0,42,166,166,166,166,166,166,166,22,22,22,22,22,34,10,33,127,41,21,100,100,''),
	(0,42,212,5600,44,0,44,169,169,169,169,169,169,169,22,22,22,22,22,35,10,34,130,42,21,100,100,''),
	(0,43,217,5900,46,0,46,173,173,173,173,173,173,173,23,23,23,23,23,35,10,34,133,43,21,100,100,''),
	(0,44,221,6200,48,0,48,176,176,176,176,176,176,176,23,23,23,23,23,36,10,35,136,44,21,100,100,''),
	(0,45,225,6500,50,0,50,179,179,179,179,179,179,179,24,24,24,24,24,37,10,36,139,45,21,100,100,''),
	(0,46,229,7200,50,0,50,182,182,182,182,182,182,182,24,24,24,24,24,38,10,44,152,46,21,100,100,''),
	(0,47,233,7900,50,0,50,185,185,185,185,185,185,185,25,25,25,25,25,39,10,51,165,47,21,100,100,''),
	(0,48,237,8600,50,0,50,188,188,188,188,188,188,188,25,25,25,25,25,39,10,59,178,48,21,100,100,''),
	(0,49,241,9300,50,0,50,191,191,191,191,191,191,191,26,26,26,26,26,40,10,66,191,49,21,100,100,''),
	(0,50,245,10000,50,0,50,194,194,194,194,194,194,194,26,26,26,26,26,41,10,74,204,50,21,100,100,''),
	(0,51,249,11700,53,0,53,197,197,197,197,197,197,197,27,27,27,27,27,42,11,78,231,51,20,100,100,''),
	(0,52,253,13400,56,0,56,200,200,200,200,200,200,200,27,27,27,27,27,43,12,81,258,52,20,100,100,''),
	(0,53,257,15100,59,0,59,203,203,203,203,203,203,203,28,28,28,28,28,43,13,85,284,53,20,100,100,''),
	(0,54,261,16800,62,10,62,206,206,206,206,206,206,206,28,28,28,28,28,44,14,89,311,54,20,100,100,''),
	(0,55,266,18500,65,10,65,210,210,210,210,210,210,210,29,29,29,29,29,45,15,93,338,55,20,100,100,''),
	(0,56,270,20200,68,10,68,213,213,213,213,213,213,213,29,29,29,29,29,46,16,96,365,56,20,100,100,''),
	(0,57,274,21900,71,10,71,216,216,216,216,216,216,216,30,30,30,30,30,47,17,100,392,57,19,100,100,''),
	(0,58,278,23600,74,10,74,219,219,219,219,219,219,219,30,30,30,30,30,47,18,104,418,58,19,100,100,'8,1'),
	(0,59,282,25300,77,10,77,222,222,222,222,222,222,222,31,31,31,31,31,48,19,107,445,59,19,100,100,'8,1'),
	(0,60,286,27000,80,20,80,225,225,225,225,225,225,225,31,31,31,31,31,49,20,111,472,60,19,100,100,'8,1^21,1'),
	(0,61,290,28909,85,20,84,228,228,228,228,228,228,228,32,32,32,32,32,50,24,128,536,61,19,100,100,'8,1^21,1'),
	(0,62,294,30818,91,20,87,231,231,231,231,231,231,231,32,32,32,32,32,51,28,145,599,62,18,100,100,'8,1^21,1'),
	(0,63,299,32727,96,20,91,234,234,234,234,234,234,234,33,33,33,33,33,51,32,162,663,63,18,100,100,'8,1^21,1'),
	(0,64,303,34636,102,20,95,237,237,237,237,237,237,237,33,33,33,33,33,52,36,179,727,64,18,100,100,'8,1^21,1'),
	(0,65,307,36545,107,25,98,240,240,240,240,240,240,240,34,34,34,34,34,53,40,196,790,65,18,100,100,'8,1^21,1'),
	(0,66,311,38455,113,25,102,244,244,244,244,244,244,244,34,34,34,34,34,54,44,213,854,66,18,100,100,'8,1^21,1'),
	(0,67,315,40364,118,25,105,247,247,247,247,247,247,247,35,35,35,35,35,55,48,230,917,67,17,100,100,'8,1^21,1'),
	(0,68,319,42273,124,25,109,250,250,250,250,250,250,250,35,35,35,35,35,56,52,247,981,68,17,100,100,'8,1^21,1'),
	(0,69,324,44182,129,25,113,253,253,253,253,253,253,253,36,36,36,36,36,56,56,264,1045,69,17,100,100,'8,1^21,1'),
	(0,70,328,46091,135,30,116,256,256,256,256,256,256,256,36,36,36,36,36,57,60,281,1108,70,17,100,100,'8,1^21,1'),
	(0,71,332,48000,140,30,120,259,259,259,259,259,259,259,37,37,37,37,37,58,64,298,1172,71,17,100,100,'8,1^21,1'),
	(0,72,336,49909,143,30,128,262,262,262,262,262,262,262,38,38,38,38,38,59,68,305,1193,72,17,100,100,'8,1^21,1'),
	(0,73,340,51818,145,30,135,265,265,265,265,265,265,265,39,39,39,39,39,60,72,312,1214,73,17,100,100,'8,1^21,1'),
	(0,74,344,53727,148,30,143,268,268,268,268,268,268,268,39,39,39,39,39,61,76,318,1235,74,17,100,100,'8,1^21,1'),
	(0,75,348,55636,150,30,150,271,271,271,271,271,271,271,40,40,40,40,40,62,80,325,1256,75,17,100,100,'8,1^21,1'),
	(0,76,352,75000,160,30,160,274,274,274,274,274,274,274,41,41,41,41,41,63,84,400,1600,76,17,100,100,'8,1^21,1'),
	(0,77,356,90000,170,30,170,277,277,277,277,277,277,277,42,42,42,42,42,64,88,500,2050,77,17,100,100,'8,1^21,1'),
	(0,78,360,113000,180,30,180,280,280,280,280,280,280,280,43,43,43,43,43,65,92,594,2323,120,17,100,100,'8,1^21,1'),
	(0,79,364,130000,190,30,190,283,283,283,283,283,283,283,44,44,44,44,44,66,96,650,2500,130,17,100,100,'8,1^21,1'),
	(0,80,368,140000,200,30,200,286,286,286,286,286,286,286,45,45,45,45,45,67,100,720,2799,140,16,100,100,'8,1^21,1'),
	(0,81,372,240000,300,30,300,289,289,289,289,289,289,289,46,46,46,46,46,68,104,800,3599,240,16,100,100,'8,1^21,1'),
	(0,82,376,340000,400,30,400,292,292,292,292,292,292,292,47,47,47,47,47,69,108,900,4599,340,16,100,100,'8,1^21,1'),
	(0,83,380,440000,410,30,410,295,295,295,295,295,295,295,48,48,48,48,48,70,112,1275,4904,440,16,100,100,'8,1^21,1'),
	(0,84,384,445000,420,30,420,298,298,298,298,298,298,298,49,49,49,49,49,71,116,1300,5100,445,16,100,100,'8,1^21,1'),
	(0,85,388,450000,430,30,430,301,301,301,301,301,301,301,50,50,50,50,50,72,120,1359,5292,450,16,100,100,'8,1^21,1'),
	(0,86,392,455000,440,30,440,304,304,304,304,304,304,304,51,51,51,51,51,73,124,1475,5578,455,16,100,100,'8,1^21,1'),
	(0,87,396,460000,450,30,450,307,307,307,307,307,307,307,52,52,52,52,52,74,128,1510,5918,460,16,100,100,'8,1^21,1'),
	(0,88,400,465000,460,30,460,310,310,310,310,310,310,310,53,53,53,53,53,75,132,1610,6200,465,16,100,100,'8,1^21,1'),
	(0,89,404,470000,470,30,470,313,313,313,313,313,313,313,54,54,54,54,54,76,136,1650,6275,470,16,100,100,'8,1^21,1'),
	(0,90,408,475000,480,30,480,316,316,316,316,316,316,316,55,55,55,55,55,77,140,1700,6350,475,16,100,100,'8,1^21,1'),
	(1,1,10,13,0,0,0,10,10,10,10,10,10,10,1,1,1,1,1,1,12,1,7,1,30,100,100,'13,1^14,1^21,1'),
	(1,2,13,32,0,0,0,13,13,13,13,13,13,13,1,1,1,1,1,2,12,1,10,2,30,100,100,'13,1^14,1^21,1'),
	(1,3,17,52,0,0,0,17,17,17,17,17,17,17,2,2,2,2,2,2,12,1,12,4,30,100,100,'13,1^14,1^21,1'),
	(1,4,19,71,0,0,0,20,20,20,20,20,20,20,2,2,2,2,2,4,12,1,14,5,30,100,100,'13,1^14,1^21,1'),
	(1,5,23,90,0,0,0,24,24,24,24,24,24,24,2,2,2,2,2,4,12,1,17,6,30,100,100,'13,1^14,1^21,1'),
	(1,6,26,120,0,0,0,28,28,28,28,28,28,28,2,2,2,2,2,5,12,1,19,7,30,100,100,'13,1^14,1^21,1'),
	(1,7,30,150,0,0,0,31,31,31,31,31,31,31,4,4,4,4,4,5,12,1,22,8,30,100,100,'13,1^14,1^21,1'),
	(1,8,34,180,0,0,0,35,35,35,35,35,35,35,4,4,4,4,4,6,12,1,24,10,30,100,100,'13,1^14,1^21,1'),
	(1,9,37,210,0,0,0,38,38,38,38,38,38,38,5,5,5,5,5,6,12,1,26,11,30,100,100,'13,1^14,1^21,1'),
	(1,10,41,240,0,0,0,42,42,42,42,42,42,42,5,5,5,5,5,7,12,1,29,12,30,100,100,'13,1^14,1^21,1'),
	(1,11,44,281,0,0,0,46,46,46,46,46,46,46,6,6,6,6,6,8,12,4,32,13,30,100,100,'13,1^14,1^21,1'),
	(1,12,48,322,0,0,0,50,50,50,50,50,50,50,6,6,6,6,6,10,12,5,36,14,30,100,100,'13,1^14,1^21,1'),
	(1,13,52,362,0,0,0,54,54,54,54,54,54,54,7,7,7,7,7,10,12,7,38,16,30,100,100,'13,1^14,1^21,1'),
	(1,14,55,403,0,0,0,58,58,58,58,58,58,58,7,7,7,7,7,11,12,8,42,17,30,100,100,'13,1^14,1^21,1'),
	(1,15,62,457,0,0,0,61,61,61,61,61,61,61,7,7,7,7,7,12,12,8,44,18,30,100,100,'13,1^14,1^21,1'),
	(1,16,71,511,0,0,0,65,65,65,65,65,65,65,8,8,8,8,8,12,12,10,47,19,30,100,100,'13,1^14,1^21,1'),
	(1,17,78,565,0,0,0,68,68,68,68,68,68,68,8,8,8,8,8,13,12,10,49,20,30,100,100,'13,1^14,1^21,1'),
	(1,18,86,619,0,0,0,72,72,72,72,72,72,72,8,8,8,8,8,13,12,11,50,22,30,100,100,'13,1^14,1^21,1'),
	(1,19,94,673,0,0,0,76,76,76,76,76,76,76,8,8,8,8,8,14,12,11,53,23,30,100,100,'13,1^14,1^21,1'),
	(1,20,102,727,0,0,0,79,79,79,79,79,79,79,10,10,10,10,10,14,12,12,55,24,30,100,100,'13,1^14,1^21,1'),
	(1,21,109,781,0,0,0,83,83,83,83,83,83,83,10,10,10,10,10,16,12,12,58,25,30,100,100,'13,1^14,1^21,1'),
	(1,22,114,854,0,0,0,86,86,86,86,86,86,86,10,10,10,10,10,17,12,12,60,26,30,100,100,'13,1^14,1^21,1'),
	(1,23,119,928,0,0,0,90,90,90,90,90,90,90,11,11,11,11,11,17,12,12,62,28,30,100,100,'13,1^14,1^21,1'),
	(1,24,124,1001,0,0,0,94,94,94,94,94,94,94,11,11,11,11,11,18,12,13,66,29,30,100,100,'13,1^14,1^21,1'),
	(1,25,128,1074,0,0,0,97,97,97,97,97,97,97,12,12,12,12,12,19,12,13,68,30,30,100,100,'13,1^14,1^21,1'),
	(1,26,133,1147,0,0,0,102,102,102,102,102,102,102,12,12,12,12,12,19,12,13,71,31,30,100,100,'13,1^14,1^21,1'),
	(1,27,138,1220,0,0,0,106,106,106,106,106,106,106,13,13,13,13,13,20,12,13,73,32,30,100,100,'13,1^14,1^21,1'),
	(1,28,143,1294,0,0,0,109,109,109,109,109,109,109,13,13,13,13,13,22,12,14,77,34,30,100,100,'13,1^14,1^21,1'),
	(1,29,148,1367,0,0,0,113,113,113,113,113,113,113,14,14,14,14,14,22,12,14,79,35,30,100,100,'13,1^14,1^21,1'),
	(1,30,152,1440,0,0,0,116,116,116,116,116,116,116,14,14,14,14,14,23,12,14,82,36,30,100,100,'13,1^14,1^21,1'),
	(1,31,162,1896,5,0,5,125,125,125,125,125,125,125,16,16,16,16,16,24,12,17,89,37,30,100,100,'13,1^14,1^21,1'),
	(1,32,170,2352,10,0,10,132,132,132,132,132,132,132,17,17,17,17,17,26,12,19,95,38,30,100,100,'13,1^14,1^21,1'),
	(1,33,180,2808,14,0,14,140,140,140,140,140,140,140,18,18,18,18,18,28,12,22,102,40,29,100,100,'13,1^14,1^21,1'),
	(1,34,190,3264,19,0,19,148,148,148,148,148,148,148,19,19,19,19,19,30,12,24,108,41,28,100,100,'13,1^14,1^21,1'),
	(1,35,199,3720,24,0,24,156,156,156,156,156,156,156,20,20,20,20,20,31,12,26,115,42,27,100,100,'13,1^14,1^21,1'),
	(1,36,208,4176,29,0,29,164,164,164,164,164,164,164,20,20,20,20,20,32,12,29,122,43,25,100,100,'13,1^14,1^21,1'),
	(1,37,217,4632,34,0,34,172,172,172,172,172,172,172,22,22,22,22,22,35,12,31,128,44,24,100,100,'13,1^14,1^21,1'),
	(1,38,227,5088,38,0,38,180,180,180,180,180,180,180,23,23,23,23,23,36,12,34,136,46,23,100,100,'13,1^14,1^21,1'),
	(1,39,235,5544,43,0,43,187,187,187,187,187,187,187,24,24,24,24,24,38,12,36,142,47,22,100,100,'13,1^14,1^21,1'),
	(1,40,245,6000,48,0,48,196,196,196,196,196,196,196,25,25,25,25,25,40,12,38,149,48,21,100,100,'13,1^14,1^21,1'),
	(1,41,250,6360,50,0,50,199,199,199,199,199,199,199,26,26,26,26,26,41,12,40,152,49,21,100,100,'13,1^14,1^21,1'),
	(1,42,254,6720,53,0,53,203,203,203,203,203,203,203,26,26,26,26,26,42,12,41,156,50,21,100,100,'13,1^14,1^21,1'),
	(1,43,260,7080,55,0,55,208,208,208,208,208,208,208,28,28,28,28,28,42,12,41,160,52,21,100,100,'13,1^14,1^21,1'),
	(1,44,265,7440,58,0,58,211,211,211,211,211,211,211,28,28,28,28,28,43,12,42,163,53,21,100,100,'13,1^14,1^21,1'),
	(1,45,270,7800,60,0,60,215,215,215,215,215,215,215,29,29,29,29,29,44,12,43,167,54,21,100,100,'13,1^14,1^21,1'),
	(1,46,275,8640,60,0,60,218,218,218,218,218,218,218,29,29,29,29,29,46,12,53,182,55,21,100,100,'13,1^14,1^21,1'),
	(1,47,280,9480,60,0,60,222,222,222,222,222,222,222,30,30,30,30,30,47,12,61,198,56,21,100,100,'13,1^14,1^21,1'),
	(1,48,284,10320,60,0,60,226,226,226,226,226,226,226,30,30,30,30,30,47,12,71,214,58,21,100,100,'13,1^14,1^21,1'),
	(1,49,289,11160,60,0,60,229,229,229,229,229,229,229,31,31,31,31,31,48,12,79,229,59,21,100,100,'13,1^14,1^21,1'),
	(1,50,294,12000,60,0,60,233,233,233,233,233,233,233,31,31,31,31,31,49,12,89,245,60,21,100,100,'13,1^14,1^21,1'),
	(1,51,299,14040,64,0,64,236,236,236,236,236,236,236,32,32,32,32,32,50,13,94,277,61,20,100,100,'13,1^14,1^21,1'),
	(1,52,304,16080,67,0,67,240,240,240,240,240,240,240,32,32,32,32,32,52,14,97,310,62,20,100,100,'13,1^14,1^21,1'),
	(1,53,308,18120,71,0,71,244,244,244,244,244,244,244,34,34,34,34,34,52,16,102,341,64,20,100,100,'13,1^14,1^21,1'),
	(1,54,313,20160,74,0,74,247,247,247,247,247,247,247,34,34,34,34,34,53,17,107,373,65,20,100,100,'13,1^14,1^21,1'),
	(1,55,319,22200,78,0,78,252,252,252,252,252,252,252,35,35,35,35,35,54,18,112,406,66,20,100,100,'13,1^14,1^21,1'),
	(1,56,324,24240,82,0,82,256,256,256,256,256,256,256,35,35,35,35,35,55,19,115,438,67,20,100,100,'13,1^14,1^21,1'),
	(1,57,329,26280,85,0,85,259,259,259,259,259,259,259,36,36,36,36,36,56,20,120,470,68,19,100,100,'13,1^14,1^21,1'),
	(1,58,334,28320,89,0,89,263,263,263,263,263,263,263,36,36,36,36,36,56,22,125,502,70,19,100,100,'13,1^14,1^21,1'),
	(1,59,338,30360,92,0,92,266,266,266,266,266,266,266,37,37,37,37,37,58,23,128,534,71,19,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,60,343,32400,96,30,96,270,270,270,270,270,270,270,37,37,37,37,37,59,24,133,566,72,19,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,61,348,34691,102,30,101,274,274,274,274,274,274,274,38,38,38,38,38,60,29,154,643,73,19,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,62,353,36982,109,30,104,277,277,277,277,277,277,277,38,38,38,38,38,61,34,174,719,74,18,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,63,359,39272,115,30,109,281,281,281,281,281,281,281,40,40,40,40,40,61,38,194,796,76,18,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,64,364,41563,122,30,114,284,284,284,284,284,284,284,40,40,40,40,40,62,43,215,872,77,18,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,65,368,43854,128,35,118,288,288,288,288,288,288,288,41,41,41,41,41,64,48,235,948,78,18,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,66,373,46146,136,35,122,293,293,293,293,293,293,293,41,41,41,41,41,65,53,256,1025,79,18,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,67,378,48437,142,35,126,296,296,296,296,296,296,296,42,42,42,42,42,66,58,276,1100,80,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,68,383,50728,149,35,131,300,300,300,300,300,300,300,42,42,42,42,42,67,62,296,1177,82,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,69,389,53018,155,35,136,304,304,304,304,304,304,304,43,43,43,43,43,67,67,317,1254,83,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,70,394,55309,162,40,139,307,307,307,307,307,307,307,43,43,43,43,43,68,72,337,1330,84,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,71,398,57600,168,40,144,311,311,311,311,311,311,311,44,44,44,44,44,70,77,358,1406,85,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,72,403,57600,172,40,154,314,314,314,314,314,314,314,46,46,46,46,46,71,82,366,1432,86,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,73,408,57600,174,40,162,318,318,318,318,318,318,318,47,47,47,47,47,72,86,374,1457,88,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,74,413,57600,178,40,172,322,322,322,322,322,322,322,47,47,47,47,47,73,91,382,1482,89,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,75,418,57600,180,40,180,325,325,325,325,325,325,325,48,48,48,48,48,74,96,390,1507,90,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,76,423,90000,192,45,192,329,329,329,329,329,329,329,49,49,49,49,49,76,101,480,1920,91,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,77,428,108000,204,45,204,332,332,332,332,332,332,332,50,50,50,50,50,77,106,600,2460,92,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,78,433,135600,216,45,216,336,336,336,336,336,336,336,52,52,52,52,52,78,110,713,2788,144,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,79,438,156000,228,45,228,340,340,340,340,340,340,340,53,53,53,53,53,79,115,780,3000,156,17,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,80,443,168000,240,45,240,343,343,343,343,343,343,343,54,54,54,54,54,80,120,864,3359,168,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,81,448,288000,360,45,360,347,347,347,347,347,347,347,55,55,55,55,55,82,125,960,4319,288,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,82,453,408000,480,45,480,350,350,350,350,350,350,350,56,56,56,56,56,83,130,1080,5519,408,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,83,458,528000,492,45,492,354,354,354,354,354,354,354,58,58,58,58,58,84,134,1530,5885,528,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,84,463,534000,504,45,504,358,358,358,358,358,358,358,59,59,59,59,59,85,139,1560,6120,534,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,85,468,540000,516,45,516,361,361,361,361,361,361,361,60,60,60,60,60,86,144,1631,6350,540,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,86,473,546000,528,45,528,365,365,365,365,365,365,365,61,61,61,61,61,88,149,1770,6694,546,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,87,478,552000,540,45,540,368,368,368,368,368,368,368,62,62,62,62,62,89,154,1812,7102,552,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,88,483,558000,552,45,552,372,372,372,372,372,372,372,64,64,64,64,64,90,158,1932,7440,558,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,89,488,564000,564,45,564,376,376,376,376,376,376,376,65,65,65,65,65,91,163,1980,7530,564,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(1,90,490,570000,576,45,576,379,379,379,379,379,379,379,66,66,66,66,66,92,168,2040,7620,570,16,100,100,'1,1^8,1^13,1^14,1^21,1'),
	(2,1,12,17,0,0,0,12,12,12,12,12,12,12,2,2,2,2,2,2,15,2,9,2,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,2,17,41,0,0,0,17,17,17,17,17,17,17,4,4,4,4,4,4,15,2,12,3,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,3,21,65,0,0,0,21,21,21,21,21,21,21,6,6,6,6,6,6,15,2,15,5,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,4,24,89,0,0,0,26,26,26,26,26,26,26,8,8,8,8,8,8,15,2,18,6,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,5,29,113,0,0,0,30,30,30,30,30,30,30,10,10,10,10,10,10,15,2,21,8,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,6,33,150,0,0,0,35,35,35,35,35,35,35,12,12,12,12,12,12,15,2,24,9,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,7,38,188,0,0,0,39,39,39,39,39,39,39,14,14,14,14,14,14,15,2,27,11,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,8,42,225,0,0,0,44,44,44,44,44,44,44,16,16,16,16,16,16,15,2,30,12,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,9,47,263,0,0,0,48,48,48,48,48,48,48,18,18,18,18,18,18,15,2,33,14,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,10,51,300,0,0,0,53,53,53,53,53,53,53,20,20,20,20,20,20,15,2,36,15,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,11,56,351,0,0,0,57,57,57,57,57,57,57,22,22,22,22,22,22,15,5,41,17,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,12,60,402,0,0,0,63,63,63,63,63,63,63,24,24,24,24,24,24,15,6,45,18,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,13,65,453,0,0,0,68,68,68,68,68,68,68,26,26,26,26,26,26,15,9,48,20,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,14,69,504,0,0,0,72,72,72,72,72,72,72,28,28,28,28,28,28,15,11,53,21,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,15,78,572,0,0,0,77,77,77,77,77,77,77,30,30,30,30,30,30,15,11,56,23,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,16,89,639,0,0,0,81,81,81,81,81,81,81,32,32,32,32,32,32,15,12,59,24,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,17,98,707,0,0,0,86,86,86,86,86,86,86,34,34,34,34,34,34,15,12,62,26,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,18,108,774,0,0,0,90,90,90,90,90,90,90,36,36,36,36,36,36,15,14,63,27,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,19,117,842,0,0,0,95,95,95,95,95,95,95,38,38,38,38,38,38,15,14,66,29,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,20,128,909,0,0,0,99,99,99,99,99,99,99,40,40,40,40,40,40,15,15,69,30,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,21,137,977,0,0,0,104,104,104,104,104,104,104,42,42,42,42,42,42,15,15,72,32,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,22,143,1068,0,0,0,108,108,108,108,108,108,108,44,44,44,44,44,44,15,15,75,33,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,23,149,1160,0,0,0,113,113,113,113,113,113,113,46,46,46,46,46,46,15,15,78,35,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,24,155,1251,0,0,0,117,117,117,117,117,117,117,48,48,48,48,48,48,15,17,83,36,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,25,161,1343,0,0,0,122,122,122,122,122,122,122,50,50,50,50,50,50,15,17,86,38,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,26,167,1434,0,0,0,128,128,128,128,128,128,128,52,52,52,52,52,52,15,17,89,39,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,27,173,1526,0,0,0,132,132,132,132,132,132,132,54,54,54,54,54,54,15,17,92,41,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,28,179,1617,0,0,0,137,137,137,137,137,137,137,56,56,56,56,56,56,15,18,96,42,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,29,185,1709,0,0,0,141,141,141,141,141,141,141,58,58,58,58,58,58,15,18,99,44,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,30,191,1800,0,0,0,146,146,146,146,146,146,146,60,60,60,60,60,60,15,18,102,45,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,31,203,2370,6,0,6,156,156,156,156,156,156,156,62,62,62,62,62,62,15,21,111,47,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,32,213,2940,12,0,12,165,165,165,165,165,165,165,64,64,64,64,64,64,15,24,119,48,30,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,33,225,3510,18,0,18,176,176,176,176,176,176,176,66,66,66,66,66,66,15,27,128,50,29,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,34,237,4080,24,0,24,185,185,185,185,185,185,185,68,68,68,68,68,68,15,30,135,51,28,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,35,249,4650,30,0,30,195,195,195,195,195,195,195,70,70,70,70,70,70,15,33,144,53,27,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,36,260,5220,36,0,36,206,206,206,206,206,206,206,72,72,72,72,72,72,15,36,153,54,25,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,37,272,5790,42,0,42,215,215,215,215,215,215,215,74,74,74,74,74,74,15,39,161,56,24,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,38,284,6360,48,0,48,225,225,225,225,225,225,225,76,76,76,76,76,76,15,42,170,57,23,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,39,294,6930,54,0,54,234,234,234,234,234,234,234,78,78,78,78,78,78,15,45,177,59,22,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,40,306,7500,60,0,60,245,245,245,245,245,245,245,80,80,80,80,80,80,15,48,186,60,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,41,312,7950,63,0,63,249,249,249,249,249,249,249,82,82,82,82,82,82,15,50,191,62,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,42,318,8400,66,0,66,254,254,254,254,254,254,254,84,84,84,84,84,84,15,51,195,63,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,43,326,11000,69,0,69,260,260,260,260,260,260,260,86,86,86,86,86,86,15,51,200,65,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,44,332,25000,72,0,72,264,264,264,264,264,264,264,88,88,88,88,88,88,15,53,204,66,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,45,338,35000,75,0,75,269,269,269,269,269,269,269,90,90,90,90,90,90,15,54,209,68,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,46,344,45000,75,0,75,273,273,273,273,273,273,273,92,92,92,92,92,92,15,66,228,69,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,47,350,55000,75,0,75,278,278,278,278,278,278,278,94,94,94,94,94,94,15,77,248,71,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,48,356,70000,75,0,75,282,282,282,282,282,282,282,96,96,96,96,96,96,15,89,267,72,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,49,362,85000,75,0,75,287,287,287,287,287,287,287,98,98,98,98,98,98,15,99,287,74,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,50,368,100000,75,0,75,291,291,291,291,291,291,291,100,100,100,100,100,100,15,111,306,75,21,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,51,374,130000,80,0,80,296,296,296,296,296,296,296,102,102,102,102,102,102,17,117,347,77,20,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,52,380,140000,84,0,84,300,300,300,300,300,300,300,104,104,104,104,104,104,18,122,387,78,20,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,53,386,150000,89,0,89,305,305,305,305,305,305,305,106,106,106,106,106,106,20,128,426,80,19,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,54,392,160000,93,0,93,309,309,309,309,309,309,309,108,108,108,108,108,108,21,134,467,81,19,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,55,399,170000,98,0,98,315,315,315,315,315,315,315,110,110,110,110,110,110,23,140,507,83,18,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,56,405,180000,102,0,102,320,320,320,320,320,320,320,112,112,112,112,112,112,24,144,548,84,17,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,57,411,190000,107,0,107,324,324,324,324,324,324,324,114,114,114,114,114,114,26,150,588,86,17,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,58,417,200000,111,0,111,329,329,329,329,329,329,329,116,116,116,116,116,116,27,156,627,87,16,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,59,434,400750,130,0,125,253,253,253,253,253,253,253,118,118,118,118,118,118,19,170,700,141,16,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,60,476,450813,140,0,129,258,258,258,258,258,258,258,120,120,120,120,120,120,22,185,740,194,15,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,61,517,500875,145,0,130,263,263,263,263,263,263,263,122,122,122,122,122,122,26,195,780,246,15,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,62,559,550938,150,0,140,268,268,268,268,268,268,268,124,124,124,124,124,124,29,210,800,299,15,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,63,600,601000,155,0,160,273,273,273,273,273,273,273,126,126,126,126,126,126,32,220,825,351,15,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,64,563,734167,160,0,170,277,277,277,277,277,277,277,128,128,128,128,128,128,50,241,850,401,15,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,65,525,867333,165,0,180,281,281,281,281,281,281,281,130,130,130,130,130,130,67,262,875,450,14,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,66,488,1000500,170,0,190,285,285,285,285,285,285,285,132,132,132,132,132,132,85,283,904,500,14,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,67,498,1013841,175,10,200,292,292,292,292,292,292,292,134,134,134,134,134,134,81,312,1071,600,14,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,68,508,1027182,180,20,225,300,300,300,300,300,300,300,136,136,136,136,136,136,77,341,1238,700,14,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,69,519,1040522,185,30,239,307,307,307,307,307,307,307,138,138,138,138,138,138,72,369,1404,800,14,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,70,529,1053863,190,40,245,315,315,315,315,315,315,315,140,140,140,140,140,140,68,398,1571,900,14,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,71,539,1067204,200,50,255,322,322,322,322,322,322,322,142,142,142,142,142,142,64,427,1738,1000,14,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,72,547,1262903,220,56,265,327,327,327,327,327,327,327,144,144,144,144,144,144,68,520,1979,1125,14,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,73,556,1458602,253,63,285,332,332,332,332,332,332,332,146,146,146,146,146,146,72,614,2219,1250,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,74,564,1654301,306,69,300,337,337,337,337,337,337,337,148,148,148,148,148,148,76,707,2460,1375,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,75,572,1850000,330,75,310,342,342,342,342,342,342,342,150,150,150,150,150,150,80,725,2700,1500,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,76,579,1900000,348,77,320,346,346,346,346,346,346,346,152,152,152,152,152,152,84,750,2960,1600,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,77,586,1950000,355,79,330,350,350,350,350,350,350,350,154,154,154,154,154,154,88,775,3000,1700,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,78,594,2000000,365,81,340,354,354,354,354,354,354,354,156,156,156,156,156,156,92,800,3100,1800,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,79,601,2050000,375,83,350,358,358,358,358,358,358,358,158,158,158,158,158,158,96,825,3200,1900,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,80,608,2100000,380,85,360,362,362,362,362,362,362,362,160,160,160,160,160,160,100,850,3300,2000,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,81,615,2480000,385,83,370,366,366,366,366,366,366,366,162,162,162,162,162,162,104,875,3350,3000,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,82,622,2860000,390,81,380,370,370,370,370,370,370,370,164,164,164,164,164,164,108,900,3400,4000,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,83,629,3240000,395,79,390,375,375,375,375,375,375,375,166,166,166,166,166,166,112,925,3450,5000,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,84,636,3620000,400,77,400,379,379,379,379,379,379,379,168,168,168,168,168,168,116,940,3500,6000,13,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,85,643,4000000,405,75,410,383,383,383,383,383,383,383,170,170,170,170,170,170,120,960,3550,7000,12,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,86,650,4800000,410,77,420,387,387,387,387,387,387,387,172,172,172,172,172,172,124,980,3600,8000,12,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,87,657,5600000,415,79,430,391,391,391,391,391,391,391,174,174,174,174,174,174,128,1000,3650,9000,12,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,88,665,6400000,420,81,440,395,395,395,395,395,395,395,176,176,176,176,176,176,132,1010,3700,10000,12,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,89,672,7200000,420,83,445,399,399,399,399,399,399,399,178,178,178,178,178,178,136,1018,3800,11000,12,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2,90,679,8000000,420,85,450,403,403,403,403,403,403,403,180,180,180,180,180,180,140,1050,3900,12000,12,100,100,'1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1');

)",
	},
	ManifestEntry{
		.version = 9133,
		.description = "2018_11_25_stuckbehavior.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'stuck_behavior'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `stuck_behavior` TINYINT(4) NOT NULL DEFAULT '0' AFTER `rare_spawn`;
UPDATE `npc_types` SET `stuck_behavior`=2 WHERE `underwater`=1;

)",
	},
	ManifestEntry{
		.version = 9134,
		.description = "2019_01_04_update_global_base_scaling.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9134",
		.condition = "empty",
		.match = "",
		.sql = R"(
REPLACE INTO `npc_scale_global_base` (`type`, `level`, `ac`, `hp`, `accuracy`, `slow_mitigation`, `attack`, `strength`, `stamina`, `dexterity`, `agility`, `intelligence`, `wisdom`, `charisma`, `magic_resist`, `cold_resist`, `fire_resist`, `poison_resist`, `disease_resist`, `corruption_resist`, `physical_resist`, `min_dmg`, `max_dmg`, `hp_regen_rate`, `attack_delay`, `spell_scale`, `heal_scale`, `special_abilities`)
VALUES
	(0, 1, 8, 11, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 1, 1, 1, 1, 1, 1, 10, 1, 6, 1, 30, 100, 100, ''),
	(0, 2, 11, 27, 0, 0, 0, 11, 11, 11, 11, 11, 11, 11, 1, 1, 1, 1, 1, 2, 10, 1, 8, 1, 30, 100, 100, ''),
	(0, 3, 14, 43, 0, 0, 0, 14, 14, 14, 14, 14, 14, 14, 2, 2, 2, 2, 2, 2, 10, 1, 10, 1, 30, 100, 100, ''),
	(0, 4, 16, 59, 0, 0, 0, 17, 17, 17, 17, 17, 17, 17, 2, 2, 2, 2, 2, 3, 10, 1, 12, 1, 30, 100, 100, ''),
	(0, 5, 19, 75, 0, 0, 0, 20, 20, 20, 20, 20, 20, 20, 2, 2, 2, 2, 2, 3, 10, 1, 14, 1, 30, 100, 100, ''),
	(0, 6, 22, 100, 0, 0, 0, 23, 23, 23, 23, 23, 23, 23, 2, 2, 2, 2, 2, 4, 10, 1, 16, 1, 30, 100, 100, ''),
	(0, 7, 25, 125, 0, 0, 0, 26, 26, 26, 26, 26, 26, 26, 3, 3, 3, 3, 3, 4, 10, 1, 18, 2, 30, 100, 100, ''),
	(0, 8, 28, 150, 0, 0, 0, 29, 29, 29, 29, 29, 29, 29, 3, 3, 3, 3, 3, 5, 10, 1, 20, 2, 30, 100, 100, ''),
	(0, 9, 31, 175, 0, 0, 0, 32, 32, 32, 32, 32, 32, 32, 4, 4, 4, 4, 4, 5, 10, 1, 22, 2, 30, 100, 100, ''),
	(0, 10, 34, 200, 0, 0, 0, 35, 35, 35, 35, 35, 35, 35, 4, 4, 4, 4, 4, 6, 10, 1, 24, 2, 30, 100, 100, ''),
	(0, 11, 37, 234, 0, 0, 0, 38, 38, 38, 38, 38, 38, 38, 5, 5, 5, 5, 5, 7, 10, 3, 27, 2, 30, 100, 100, ''),
	(0, 12, 40, 268, 0, 0, 0, 42, 42, 42, 42, 42, 42, 42, 5, 5, 5, 5, 5, 8, 10, 4, 30, 3, 30, 100, 100, ''),
	(0, 13, 43, 302, 0, 0, 0, 45, 45, 45, 45, 45, 45, 45, 6, 6, 6, 6, 6, 8, 10, 6, 32, 3, 30, 100, 100, ''),
	(0, 14, 46, 336, 0, 0, 0, 48, 48, 48, 48, 48, 48, 48, 6, 6, 6, 6, 6, 9, 10, 7, 35, 3, 30, 100, 100, ''),
	(0, 15, 52, 381, 0, 0, 0, 51, 51, 51, 51, 51, 51, 51, 6, 6, 6, 6, 6, 10, 10, 7, 37, 4, 30, 100, 100, ''),
	(0, 16, 59, 426, 0, 0, 0, 54, 54, 54, 54, 54, 54, 54, 7, 7, 7, 7, 7, 10, 10, 8, 39, 4, 30, 100, 100, ''),
	(0, 17, 65, 471, 0, 0, 0, 57, 57, 57, 57, 57, 57, 57, 7, 7, 7, 7, 7, 11, 10, 8, 41, 5, 30, 100, 100, ''),
	(0, 18, 72, 516, 0, 0, 0, 60, 60, 60, 60, 60, 60, 60, 7, 7, 7, 7, 7, 11, 10, 9, 42, 5, 30, 100, 100, ''),
	(0, 19, 78, 561, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63, 7, 7, 7, 7, 7, 12, 10, 9, 44, 6, 30, 100, 100, ''),
	(0, 20, 85, 606, 0, 0, 0, 66, 66, 66, 66, 66, 66, 66, 8, 8, 8, 8, 8, 12, 10, 10, 46, 6, 30, 100, 100, ''),
	(0, 21, 91, 651, 0, 0, 0, 69, 69, 69, 69, 69, 69, 69, 8, 8, 8, 8, 8, 13, 10, 10, 48, 7, 30, 100, 100, ''),
	(0, 22, 95, 712, 0, 0, 0, 72, 72, 72, 72, 72, 72, 72, 8, 8, 8, 8, 8, 14, 10, 10, 50, 7, 30, 100, 100, ''),
	(0, 23, 99, 800, 0, 0, 0, 75, 75, 75, 75, 75, 75, 75, 9, 9, 9, 9, 9, 14, 10, 10, 52, 8, 30, 100, 100, ''),
	(0, 24, 103, 845, 0, 0, 0, 78, 78, 78, 78, 78, 78, 78, 9, 9, 9, 9, 9, 15, 10, 11, 55, 8, 30, 100, 100, ''),
	(0, 25, 107, 895, 0, 0, 0, 81, 81, 81, 81, 81, 81, 81, 10, 10, 10, 10, 10, 16, 10, 11, 57, 9, 30, 100, 100, ''),
	(0, 26, 111, 956, 0, 0, 0, 85, 85, 85, 85, 85, 85, 85, 10, 10, 10, 10, 10, 16, 10, 11, 59, 10, 30, 100, 100, ''),
	(0, 27, 115, 1100, 0, 0, 0, 88, 88, 88, 88, 88, 88, 88, 11, 11, 11, 11, 11, 17, 10, 11, 61, 10, 30, 100, 100, ''),
	(0, 28, 119, 1140, 0, 0, 0, 91, 91, 91, 91, 91, 91, 91, 11, 11, 11, 11, 11, 18, 10, 12, 64, 11, 30, 100, 100, ''),
	(0, 29, 123, 1240, 0, 0, 0, 94, 94, 94, 94, 94, 94, 94, 12, 12, 12, 12, 12, 18, 10, 12, 66, 12, 30, 100, 100, ''),
	(0, 30, 127, 1350, 0, 0, 0, 97, 97, 97, 97, 97, 97, 97, 12, 12, 12, 12, 12, 19, 10, 12, 68, 13, 30, 100, 100, ''),
	(0, 31, 135, 1450, 4, 0, 4, 104, 104, 104, 104, 104, 104, 104, 13, 13, 13, 13, 13, 20, 10, 14, 74, 14, 30, 100, 100, ''),
	(0, 32, 142, 1550, 8, 0, 8, 110, 110, 110, 110, 110, 110, 110, 14, 14, 14, 14, 14, 22, 10, 16, 79, 15, 30, 100, 100, ''),
	(0, 33, 150, 1650, 12, 0, 12, 117, 117, 117, 117, 117, 117, 117, 15, 15, 15, 15, 15, 23, 10, 18, 85, 16, 29, 100, 100, ''),
	(0, 34, 158, 1750, 16, 0, 16, 123, 123, 123, 123, 123, 123, 123, 16, 16, 16, 16, 16, 25, 10, 20, 90, 17, 28, 100, 100, ''),
	(0, 35, 166, 1850, 20, 0, 20, 130, 130, 130, 130, 130, 130, 130, 17, 17, 17, 17, 17, 26, 10, 22, 96, 18, 27, 100, 100, ''),
	(0, 36, 173, 1950, 24, 0, 24, 137, 137, 137, 137, 137, 137, 137, 17, 17, 17, 17, 17, 27, 10, 24, 102, 19, 25, 100, 100, ''),
	(0, 37, 181, 2100, 28, 0, 28, 143, 143, 143, 143, 143, 143, 143, 18, 18, 18, 18, 18, 29, 10, 26, 107, 21, 24, 100, 100, ''),
	(0, 38, 189, 2350, 32, 0, 32, 150, 150, 150, 150, 150, 150, 150, 19, 19, 19, 19, 19, 30, 10, 28, 113, 23, 23, 100, 100, ''),
	(0, 39, 196, 2650, 36, 0, 36, 156, 156, 156, 156, 156, 156, 156, 20, 20, 20, 20, 20, 32, 10, 30, 118, 26, 22, 100, 100, ''),
	(0, 40, 204, 2900, 40, 0, 40, 163, 163, 163, 163, 163, 163, 163, 21, 21, 21, 21, 21, 33, 10, 32, 124, 29, 21, 100, 100, ''),
	(0, 41, 208, 3250, 42, 0, 42, 166, 166, 166, 166, 166, 166, 166, 22, 22, 22, 22, 22, 34, 10, 33, 127, 32, 21, 100, 100, ''),
	(0, 42, 212, 3750, 44, 0, 44, 169, 169, 169, 169, 169, 169, 169, 22, 22, 22, 22, 22, 35, 10, 34, 130, 37, 21, 100, 100, ''),
	(0, 43, 217, 4250, 46, 0, 46, 173, 173, 173, 173, 173, 173, 173, 23, 23, 23, 23, 23, 35, 10, 34, 133, 42, 21, 100, 100, ''),
	(0, 44, 221, 5000, 48, 0, 48, 176, 176, 176, 176, 176, 176, 176, 23, 23, 23, 23, 23, 36, 10, 35, 136, 50, 21, 100, 100, ''),
	(0, 45, 225, 5600, 50, 0, 50, 179, 179, 179, 179, 179, 179, 179, 24, 24, 24, 24, 24, 37, 10, 36, 139, 56, 21, 100, 100, ''),
	(0, 46, 229, 6000, 50, 0, 50, 182, 182, 182, 182, 182, 182, 182, 24, 24, 24, 24, 24, 38, 10, 44, 152, 60, 21, 100, 100, ''),
	(0, 47, 233, 6500, 50, 0, 50, 185, 185, 185, 185, 185, 185, 185, 25, 25, 25, 25, 25, 39, 10, 51, 165, 65, 21, 100, 100, ''),
	(0, 48, 237, 7500, 50, 0, 50, 188, 188, 188, 188, 188, 188, 188, 25, 25, 25, 25, 25, 39, 10, 59, 178, 75, 21, 100, 100, ''),
	(0, 49, 241, 8500, 50, 0, 50, 191, 191, 191, 191, 191, 191, 191, 26, 26, 26, 26, 26, 40, 10, 66, 191, 85, 21, 100, 100, ''),
	(0, 50, 245, 10000, 50, 0, 50, 194, 194, 194, 194, 194, 194, 194, 26, 26, 26, 26, 26, 41, 10, 74, 204, 100, 21, 100, 100, ''),
	(0, 51, 249, 11700, 53, 0, 53, 197, 197, 197, 197, 197, 197, 197, 27, 27, 27, 27, 27, 42, 11, 78, 231, 117, 20, 100, 100, ''),
	(0, 52, 253, 13400, 56, 0, 56, 200, 200, 200, 200, 200, 200, 200, 27, 27, 27, 27, 27, 43, 12, 81, 258, 134, 20, 100, 100, ''),
	(0, 53, 257, 15100, 59, 0, 59, 203, 203, 203, 203, 203, 203, 203, 28, 28, 28, 28, 28, 43, 13, 85, 284, 151, 20, 100, 100, ''),
	(0, 54, 261, 16800, 62, 10, 62, 206, 206, 206, 206, 206, 206, 206, 28, 28, 28, 28, 28, 44, 14, 89, 311, 168, 20, 100, 100, ''),
	(0, 55, 266, 18500, 65, 10, 65, 210, 210, 210, 210, 210, 210, 210, 29, 29, 29, 29, 29, 45, 15, 93, 338, 185, 20, 100, 100, ''),
	(0, 56, 270, 20200, 68, 10, 68, 213, 213, 213, 213, 213, 213, 213, 29, 29, 29, 29, 29, 46, 16, 96, 365, 202, 20, 100, 100, ''),
	(0, 57, 274, 21900, 71, 10, 71, 216, 216, 216, 216, 216, 216, 216, 30, 30, 30, 30, 30, 47, 17, 100, 392, 219, 19, 100, 100, ''),
	(0, 58, 278, 23600, 74, 10, 74, 219, 219, 219, 219, 219, 219, 219, 30, 30, 30, 30, 30, 47, 18, 104, 418, 236, 19, 100, 100, '8,1'),
	(0, 59, 282, 25300, 77, 10, 77, 222, 222, 222, 222, 222, 222, 222, 31, 31, 31, 31, 31, 48, 19, 107, 445, 253, 19, 100, 100, '8,1'),
	(0, 60, 286, 27000, 80, 20, 80, 225, 225, 225, 225, 225, 225, 225, 31, 31, 31, 31, 31, 49, 20, 111, 472, 270, 19, 100, 100, '8,1^21,1'),
	(0, 61, 290, 28909, 85, 20, 84, 228, 228, 228, 228, 228, 228, 228, 32, 32, 32, 32, 32, 50, 24, 128, 536, 289, 19, 100, 100, '8,1^21,1'),
	(0, 62, 294, 30818, 91, 20, 87, 231, 231, 231, 231, 231, 231, 231, 32, 32, 32, 32, 32, 51, 28, 145, 599, 308, 18, 100, 100, '8,1^21,1'),
	(0, 63, 299, 32727, 96, 20, 91, 234, 234, 234, 234, 234, 234, 234, 33, 33, 33, 33, 33, 51, 32, 162, 663, 327, 18, 100, 100, '8,1^21,1'),
	(0, 64, 303, 34636, 102, 20, 95, 237, 237, 237, 237, 237, 237, 237, 33, 33, 33, 33, 33, 52, 36, 179, 727, 346, 18, 100, 100, '8,1^21,1'),
	(0, 65, 307, 36545, 107, 25, 98, 240, 240, 240, 240, 240, 240, 240, 34, 34, 34, 34, 34, 53, 40, 196, 790, 365, 18, 100, 100, '8,1^21,1'),
	(0, 66, 311, 38455, 113, 25, 102, 244, 244, 244, 244, 244, 244, 244, 34, 34, 34, 34, 34, 54, 44, 213, 854, 384, 18, 100, 100, '8,1^21,1'),
	(0, 67, 315, 40364, 118, 25, 105, 247, 247, 247, 247, 247, 247, 247, 35, 35, 35, 35, 35, 55, 48, 230, 917, 403, 17, 100, 100, '8,1^21,1'),
	(0, 68, 319, 42273, 124, 25, 109, 250, 250, 250, 250, 250, 250, 250, 35, 35, 35, 35, 35, 56, 52, 247, 981, 422, 17, 100, 100, '8,1^21,1'),
	(0, 69, 324, 44182, 129, 25, 113, 253, 253, 253, 253, 253, 253, 253, 36, 36, 36, 36, 36, 56, 56, 264, 1045, 441, 17, 100, 100, '8,1^21,1'),
	(0, 70, 328, 46091, 135, 30, 116, 256, 256, 256, 256, 256, 256, 256, 36, 36, 36, 36, 36, 57, 60, 281, 1108, 460, 17, 100, 100, '8,1^21,1'),
	(0, 71, 332, 48000, 140, 30, 120, 259, 259, 259, 259, 259, 259, 259, 37, 37, 37, 37, 37, 58, 64, 298, 1172, 480, 17, 100, 100, '8,1^21,1'),
	(0, 72, 336, 49909, 143, 30, 128, 262, 262, 262, 262, 262, 262, 262, 38, 38, 38, 38, 38, 59, 68, 305, 1193, 499, 17, 100, 100, '8,1^21,1'),
	(0, 73, 340, 51818, 145, 30, 135, 265, 265, 265, 265, 265, 265, 265, 39, 39, 39, 39, 39, 60, 72, 312, 1214, 518, 17, 100, 100, '8,1^21,1'),
	(0, 74, 344, 53727, 148, 30, 143, 268, 268, 268, 268, 268, 268, 268, 39, 39, 39, 39, 39, 61, 76, 318, 1235, 537, 17, 100, 100, '8,1^21,1'),
	(0, 75, 348, 55636, 150, 30, 150, 271, 271, 271, 271, 271, 271, 271, 40, 40, 40, 40, 40, 62, 80, 325, 1256, 556, 17, 100, 100, '8,1^21,1'),
	(0, 76, 352, 75000, 160, 30, 160, 274, 274, 274, 274, 274, 274, 274, 41, 41, 41, 41, 41, 63, 84, 400, 1600, 750, 17, 100, 100, '8,1^21,1'),
	(0, 77, 356, 90000, 170, 30, 170, 277, 277, 277, 277, 277, 277, 277, 42, 42, 42, 42, 42, 64, 88, 500, 2050, 900, 17, 100, 100, '8,1^21,1'),
	(0, 78, 360, 113000, 180, 30, 180, 280, 280, 280, 280, 280, 280, 280, 43, 43, 43, 43, 43, 65, 92, 594, 2323, 1130, 17, 100, 100, '8,1^21,1'),
	(0, 79, 364, 130000, 190, 30, 190, 283, 283, 283, 283, 283, 283, 283, 44, 44, 44, 44, 44, 66, 96, 650, 2500, 1300, 17, 100, 100, '8,1^21,1'),
	(0, 80, 368, 140000, 200, 30, 200, 286, 286, 286, 286, 286, 286, 286, 45, 45, 45, 45, 45, 67, 100, 720, 2799, 1140, 16, 100, 100, '8,1^21,1'),
	(0, 81, 372, 240000, 300, 30, 300, 289, 289, 289, 289, 289, 289, 289, 46, 46, 46, 46, 46, 68, 104, 800, 3599, 2400, 16, 100, 100, '8,1^21,1'),
	(0, 82, 376, 340000, 400, 30, 400, 292, 292, 292, 292, 292, 292, 292, 47, 47, 47, 47, 47, 69, 108, 900, 4599, 3400, 16, 100, 100, '8,1^21,1'),
	(0, 83, 380, 440000, 410, 30, 410, 295, 295, 295, 295, 295, 295, 295, 48, 48, 48, 48, 48, 70, 112, 1275, 4904, 4400, 16, 100, 100, '8,1^21,1'),
	(0, 84, 384, 445000, 420, 30, 420, 298, 298, 298, 298, 298, 298, 298, 49, 49, 49, 49, 49, 71, 116, 1300, 5100, 4450, 16, 100, 100, '8,1^21,1'),
	(0, 85, 388, 450000, 430, 30, 430, 301, 301, 301, 301, 301, 301, 301, 50, 50, 50, 50, 50, 72, 120, 1359, 5292, 4500, 16, 100, 100, '8,1^21,1'),
	(0, 86, 392, 455000, 440, 30, 440, 304, 304, 304, 304, 304, 304, 304, 51, 51, 51, 51, 51, 73, 124, 1475, 5578, 4550, 16, 100, 100, '8,1^21,1'),
	(0, 87, 396, 460000, 450, 30, 450, 307, 307, 307, 307, 307, 307, 307, 52, 52, 52, 52, 52, 74, 128, 1510, 5918, 4600, 16, 100, 100, '8,1^21,1'),
	(0, 88, 400, 465000, 460, 30, 460, 310, 310, 310, 310, 310, 310, 310, 53, 53, 53, 53, 53, 75, 132, 1610, 6200, 4650, 16, 100, 100, '8,1^21,1'),
	(0, 89, 404, 470000, 470, 30, 470, 313, 313, 313, 313, 313, 313, 313, 54, 54, 54, 54, 54, 76, 136, 1650, 6275, 4700, 16, 100, 100, '8,1^21,1'),
	(0, 90, 408, 475000, 480, 30, 480, 316, 316, 316, 316, 316, 316, 316, 55, 55, 55, 55, 55, 77, 140, 1700, 6350, 4750, 16, 100, 100, '8,1^21,1'),
	(1, 1, 10, 13, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 1, 1, 1, 1, 1, 1, 12, 1, 7, 0, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 2, 13, 32, 0, 0, 0, 13, 13, 13, 13, 13, 13, 13, 1, 1, 1, 1, 1, 2, 12, 1, 10, 0, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 3, 17, 52, 0, 0, 0, 17, 17, 17, 17, 17, 17, 17, 2, 2, 2, 2, 2, 2, 12, 1, 12, 1, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 4, 19, 71, 0, 0, 0, 20, 20, 20, 20, 20, 20, 20, 2, 2, 2, 2, 2, 4, 12, 1, 14, 1, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 5, 23, 90, 0, 0, 0, 24, 24, 24, 24, 24, 24, 24, 2, 2, 2, 2, 2, 4, 12, 1, 17, 1, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 6, 26, 120, 0, 0, 0, 28, 28, 28, 28, 28, 28, 28, 2, 2, 2, 2, 2, 5, 12, 1, 19, 1, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 7, 30, 150, 0, 0, 0, 31, 31, 31, 31, 31, 31, 31, 4, 4, 4, 4, 4, 5, 12, 1, 22, 2, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 8, 34, 180, 0, 0, 0, 35, 35, 35, 35, 35, 35, 35, 4, 4, 4, 4, 4, 6, 12, 1, 24, 2, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 9, 37, 210, 0, 0, 0, 38, 38, 38, 38, 38, 38, 38, 5, 5, 5, 5, 5, 6, 12, 1, 26, 2, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 10, 41, 240, 0, 0, 0, 42, 42, 42, 42, 42, 42, 42, 5, 5, 5, 5, 5, 7, 12, 1, 29, 2, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 11, 44, 281, 0, 0, 0, 46, 46, 46, 46, 46, 46, 46, 6, 6, 6, 6, 6, 8, 12, 4, 32, 3, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 12, 48, 322, 0, 0, 0, 50, 50, 50, 50, 50, 50, 50, 6, 6, 6, 6, 6, 10, 12, 5, 36, 3, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 13, 52, 362, 0, 0, 0, 54, 54, 54, 54, 54, 54, 54, 7, 7, 7, 7, 7, 10, 12, 7, 38, 4, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 14, 55, 403, 0, 0, 0, 58, 58, 58, 58, 58, 58, 58, 7, 7, 7, 7, 7, 11, 12, 8, 42, 4, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 15, 62, 457, 0, 0, 0, 61, 61, 61, 61, 61, 61, 61, 7, 7, 7, 7, 7, 12, 12, 8, 44, 5, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 16, 71, 511, 0, 0, 0, 65, 65, 65, 65, 65, 65, 65, 8, 8, 8, 8, 8, 12, 12, 10, 47, 5, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 17, 78, 565, 0, 0, 0, 68, 68, 68, 68, 68, 68, 68, 8, 8, 8, 8, 8, 13, 12, 10, 49, 6, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 18, 86, 619, 0, 0, 0, 72, 72, 72, 72, 72, 72, 72, 8, 8, 8, 8, 8, 13, 12, 11, 50, 6, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 19, 94, 673, 0, 0, 0, 76, 76, 76, 76, 76, 76, 76, 8, 8, 8, 8, 8, 14, 12, 11, 53, 7, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 20, 102, 727, 0, 0, 0, 79, 79, 79, 79, 79, 79, 79, 10, 10, 10, 10, 10, 14, 12, 12, 55, 7, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 21, 109, 781, 0, 0, 0, 83, 83, 83, 83, 83, 83, 83, 10, 10, 10, 10, 10, 16, 12, 12, 58, 8, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 22, 114, 854, 0, 0, 0, 86, 86, 86, 86, 86, 86, 86, 10, 10, 10, 10, 10, 17, 12, 12, 60, 9, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 23, 119, 928, 0, 0, 0, 90, 90, 90, 90, 90, 90, 90, 11, 11, 11, 11, 11, 17, 12, 12, 62, 9, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 24, 124, 1001, 0, 0, 0, 94, 94, 94, 94, 94, 94, 94, 11, 11, 11, 11, 11, 18, 12, 13, 66, 10, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 25, 128, 1074, 0, 0, 0, 97, 97, 97, 97, 97, 97, 97, 12, 12, 12, 12, 12, 19, 12, 13, 68, 11, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 26, 133, 1147, 0, 0, 0, 102, 102, 102, 102, 102, 102, 102, 12, 12, 12, 12, 12, 19, 12, 13, 71, 11, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 27, 138, 1220, 0, 0, 0, 106, 106, 106, 106, 106, 106, 106, 13, 13, 13, 13, 13, 20, 12, 13, 73, 12, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 28, 143, 1294, 0, 0, 0, 109, 109, 109, 109, 109, 109, 109, 13, 13, 13, 13, 13, 22, 12, 14, 77, 13, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 29, 148, 1367, 0, 0, 0, 113, 113, 113, 113, 113, 113, 113, 14, 14, 14, 14, 14, 22, 12, 14, 79, 14, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 30, 152, 1440, 0, 0, 0, 116, 116, 116, 116, 116, 116, 116, 14, 14, 14, 14, 14, 23, 12, 14, 82, 14, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 31, 162, 1896, 5, 0, 5, 125, 125, 125, 125, 125, 125, 125, 16, 16, 16, 16, 16, 24, 12, 17, 89, 19, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 32, 170, 2352, 10, 0, 10, 132, 132, 132, 132, 132, 132, 132, 17, 17, 17, 17, 17, 26, 12, 19, 95, 24, 30, 100, 100, '13,1^14,1^21,1'),
	(1, 33, 180, 2808, 14, 0, 14, 140, 140, 140, 140, 140, 140, 140, 18, 18, 18, 18, 18, 28, 12, 22, 102, 28, 29, 100, 100, '13,1^14,1^21,1'),
	(1, 34, 190, 3264, 19, 0, 19, 148, 148, 148, 148, 148, 148, 148, 19, 19, 19, 19, 19, 30, 12, 24, 108, 33, 28, 100, 100, '13,1^14,1^21,1'),
	(1, 35, 199, 3720, 24, 0, 24, 156, 156, 156, 156, 156, 156, 156, 20, 20, 20, 20, 20, 31, 12, 26, 115, 37, 27, 100, 100, '13,1^14,1^21,1'),
	(1, 36, 208, 4176, 29, 0, 29, 164, 164, 164, 164, 164, 164, 164, 20, 20, 20, 20, 20, 32, 12, 29, 122, 42, 25, 100, 100, '13,1^14,1^21,1'),
	(1, 37, 217, 4632, 34, 0, 34, 172, 172, 172, 172, 172, 172, 172, 22, 22, 22, 22, 22, 35, 12, 31, 128, 46, 24, 100, 100, '13,1^14,1^21,1'),
	(1, 38, 227, 5088, 38, 0, 38, 180, 180, 180, 180, 180, 180, 180, 23, 23, 23, 23, 23, 36, 12, 34, 136, 51, 23, 100, 100, '13,1^14,1^21,1'),
	(1, 39, 235, 5544, 43, 0, 43, 187, 187, 187, 187, 187, 187, 187, 24, 24, 24, 24, 24, 38, 12, 36, 142, 55, 22, 100, 100, '13,1^14,1^21,1'),
	(1, 40, 245, 6000, 48, 0, 48, 196, 196, 196, 196, 196, 196, 196, 25, 25, 25, 25, 25, 40, 12, 38, 149, 60, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 41, 250, 6360, 50, 0, 50, 199, 199, 199, 199, 199, 199, 199, 26, 26, 26, 26, 26, 41, 12, 40, 152, 64, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 42, 254, 6720, 53, 0, 53, 203, 203, 203, 203, 203, 203, 203, 26, 26, 26, 26, 26, 42, 12, 41, 156, 67, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 43, 260, 7080, 55, 0, 55, 208, 208, 208, 208, 208, 208, 208, 28, 28, 28, 28, 28, 42, 12, 41, 160, 71, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 44, 265, 7440, 58, 0, 58, 211, 211, 211, 211, 211, 211, 211, 28, 28, 28, 28, 28, 43, 12, 42, 163, 74, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 45, 270, 7800, 60, 0, 60, 215, 215, 215, 215, 215, 215, 215, 29, 29, 29, 29, 29, 44, 12, 43, 167, 78, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 46, 275, 8640, 60, 0, 60, 218, 218, 218, 218, 218, 218, 218, 29, 29, 29, 29, 29, 46, 12, 53, 182, 86, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 47, 280, 9480, 60, 0, 60, 222, 222, 222, 222, 222, 222, 222, 30, 30, 30, 30, 30, 47, 12, 61, 198, 95, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 48, 284, 10320, 60, 0, 60, 226, 226, 226, 226, 226, 226, 226, 30, 30, 30, 30, 30, 47, 12, 71, 214, 103, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 49, 289, 11160, 60, 0, 60, 229, 229, 229, 229, 229, 229, 229, 31, 31, 31, 31, 31, 48, 12, 79, 229, 112, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 50, 294, 12000, 60, 0, 60, 233, 233, 233, 233, 233, 233, 233, 31, 31, 31, 31, 31, 49, 12, 89, 245, 120, 21, 100, 100, '13,1^14,1^21,1'),
	(1, 51, 299, 14040, 64, 0, 64, 236, 236, 236, 236, 236, 236, 236, 32, 32, 32, 32, 32, 50, 13, 94, 277, 140, 20, 100, 100, '13,1^14,1^21,1'),
	(1, 52, 304, 16080, 67, 0, 67, 240, 240, 240, 240, 240, 240, 240, 32, 32, 32, 32, 32, 52, 14, 97, 310, 161, 20, 100, 100, '13,1^14,1^21,1'),
	(1, 53, 308, 18120, 71, 0, 71, 244, 244, 244, 244, 244, 244, 244, 34, 34, 34, 34, 34, 52, 16, 102, 341, 181, 20, 100, 100, '13,1^14,1^21,1'),
	(1, 54, 313, 20160, 74, 0, 74, 247, 247, 247, 247, 247, 247, 247, 34, 34, 34, 34, 34, 53, 17, 107, 373, 202, 20, 100, 100, '13,1^14,1^21,1'),
	(1, 55, 319, 22200, 78, 0, 78, 252, 252, 252, 252, 252, 252, 252, 35, 35, 35, 35, 35, 54, 18, 112, 406, 222, 20, 100, 100, '13,1^14,1^21,1'),
	(1, 56, 324, 24240, 82, 0, 82, 256, 256, 256, 256, 256, 256, 256, 35, 35, 35, 35, 35, 55, 19, 115, 438, 242, 20, 100, 100, '13,1^14,1^21,1'),
	(1, 57, 329, 26280, 85, 0, 85, 259, 259, 259, 259, 259, 259, 259, 36, 36, 36, 36, 36, 56, 20, 120, 470, 263, 19, 100, 100, '13,1^14,1^21,1'),
	(1, 58, 334, 28320, 89, 0, 89, 263, 263, 263, 263, 263, 263, 263, 36, 36, 36, 36, 36, 56, 22, 125, 502, 283, 19, 100, 100, '13,1^14,1^21,1'),
	(1, 59, 338, 30360, 92, 0, 92, 266, 266, 266, 266, 266, 266, 266, 37, 37, 37, 37, 37, 58, 23, 128, 534, 304, 19, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 60, 343, 32400, 96, 30, 96, 270, 270, 270, 270, 270, 270, 270, 37, 37, 37, 37, 37, 59, 24, 133, 566, 324, 19, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 61, 348, 34691, 102, 30, 101, 274, 274, 274, 274, 274, 274, 274, 38, 38, 38, 38, 38, 60, 29, 154, 643, 347, 19, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 62, 353, 36982, 109, 30, 104, 277, 277, 277, 277, 277, 277, 277, 38, 38, 38, 38, 38, 61, 34, 174, 719, 370, 18, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 63, 359, 39272, 115, 30, 109, 281, 281, 281, 281, 281, 281, 281, 40, 40, 40, 40, 40, 61, 38, 194, 796, 393, 18, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 64, 364, 41563, 122, 30, 114, 284, 284, 284, 284, 284, 284, 284, 40, 40, 40, 40, 40, 62, 43, 215, 872, 416, 18, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 65, 368, 43854, 128, 35, 118, 288, 288, 288, 288, 288, 288, 288, 41, 41, 41, 41, 41, 64, 48, 235, 948, 439, 18, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 66, 373, 46146, 136, 35, 122, 293, 293, 293, 293, 293, 293, 293, 41, 41, 41, 41, 41, 65, 53, 256, 1025, 461, 18, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 67, 378, 48437, 142, 35, 126, 296, 296, 296, 296, 296, 296, 296, 42, 42, 42, 42, 42, 66, 58, 276, 1100, 484, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 68, 383, 50728, 149, 35, 131, 300, 300, 300, 300, 300, 300, 300, 42, 42, 42, 42, 42, 67, 62, 296, 1177, 507, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 69, 389, 53018, 155, 35, 136, 304, 304, 304, 304, 304, 304, 304, 43, 43, 43, 43, 43, 67, 67, 317, 1254, 530, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 70, 394, 55309, 162, 40, 139, 307, 307, 307, 307, 307, 307, 307, 43, 43, 43, 43, 43, 68, 72, 337, 1330, 553, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 71, 398, 57600, 168, 40, 144, 311, 311, 311, 311, 311, 311, 311, 44, 44, 44, 44, 44, 70, 77, 358, 1406, 576, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 72, 403, 57600, 172, 40, 154, 314, 314, 314, 314, 314, 314, 314, 46, 46, 46, 46, 46, 71, 82, 366, 1432, 576, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 73, 408, 57600, 174, 40, 162, 318, 318, 318, 318, 318, 318, 318, 47, 47, 47, 47, 47, 72, 86, 374, 1457, 576, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 74, 413, 57600, 178, 40, 172, 322, 322, 322, 322, 322, 322, 322, 47, 47, 47, 47, 47, 73, 91, 382, 1482, 576, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 75, 418, 57600, 180, 40, 180, 325, 325, 325, 325, 325, 325, 325, 48, 48, 48, 48, 48, 74, 96, 390, 1507, 576, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 76, 423, 90000, 192, 45, 192, 329, 329, 329, 329, 329, 329, 329, 49, 49, 49, 49, 49, 76, 101, 480, 1920, 900, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 77, 428, 108000, 204, 45, 204, 332, 332, 332, 332, 332, 332, 332, 50, 50, 50, 50, 50, 77, 106, 600, 2460, 1080, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 78, 433, 135600, 216, 45, 216, 336, 336, 336, 336, 336, 336, 336, 52, 52, 52, 52, 52, 78, 110, 713, 2788, 1356, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 79, 438, 156000, 228, 45, 228, 340, 340, 340, 340, 340, 340, 340, 53, 53, 53, 53, 53, 79, 115, 780, 3000, 1560, 17, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 80, 443, 168000, 240, 45, 240, 343, 343, 343, 343, 343, 343, 343, 54, 54, 54, 54, 54, 80, 120, 864, 3359, 1680, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 81, 448, 288000, 360, 45, 360, 347, 347, 347, 347, 347, 347, 347, 55, 55, 55, 55, 55, 82, 125, 960, 4319, 2880, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 82, 453, 408000, 480, 45, 480, 350, 350, 350, 350, 350, 350, 350, 56, 56, 56, 56, 56, 83, 130, 1080, 5519, 4080, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 83, 458, 528000, 492, 45, 492, 354, 354, 354, 354, 354, 354, 354, 58, 58, 58, 58, 58, 84, 134, 1530, 5885, 5280, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 84, 463, 534000, 504, 45, 504, 358, 358, 358, 358, 358, 358, 358, 59, 59, 59, 59, 59, 85, 139, 1560, 6120, 5340, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 85, 468, 540000, 516, 45, 516, 361, 361, 361, 361, 361, 361, 361, 60, 60, 60, 60, 60, 86, 144, 1631, 6350, 5400, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 86, 473, 546000, 528, 45, 528, 365, 365, 365, 365, 365, 365, 365, 61, 61, 61, 61, 61, 88, 149, 1770, 6694, 5460, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 87, 478, 552000, 540, 45, 540, 368, 368, 368, 368, 368, 368, 368, 62, 62, 62, 62, 62, 89, 154, 1812, 7102, 5520, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 88, 483, 558000, 552, 45, 552, 372, 372, 372, 372, 372, 372, 372, 64, 64, 64, 64, 64, 90, 158, 1932, 7440, 5580, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 89, 488, 564000, 564, 45, 564, 376, 376, 376, 376, 376, 376, 376, 65, 65, 65, 65, 65, 91, 163, 1980, 7530, 5640, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(1, 90, 490, 570000, 576, 45, 576, 379, 379, 379, 379, 379, 379, 379, 66, 66, 66, 66, 66, 92, 168, 2040, 7620, 5700, 16, 100, 100, '1,1^8,1^13,1^14,1^21,1'),
	(2, 1, 12, 17, 0, 0, 0, 12, 12, 12, 12, 12, 12, 12, 2, 2, 2, 2, 2, 2, 15, 2, 9, 0, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 2, 17, 41, 0, 0, 0, 17, 17, 17, 17, 17, 17, 17, 4, 4, 4, 4, 4, 4, 15, 2, 12, 0, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 3, 21, 65, 0, 0, 0, 21, 21, 21, 21, 21, 21, 21, 6, 6, 6, 6, 6, 6, 15, 2, 15, 1, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 4, 24, 89, 0, 0, 0, 26, 26, 26, 26, 26, 26, 26, 8, 8, 8, 8, 8, 8, 15, 2, 18, 1, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 5, 29, 113, 0, 0, 0, 30, 30, 30, 30, 30, 30, 30, 10, 10, 10, 10, 10, 10, 15, 2, 21, 1, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 6, 33, 150, 0, 0, 0, 35, 35, 35, 35, 35, 35, 35, 12, 12, 12, 12, 12, 12, 15, 2, 24, 2, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 7, 38, 188, 0, 0, 0, 39, 39, 39, 39, 39, 39, 39, 14, 14, 14, 14, 14, 14, 15, 2, 27, 2, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 8, 42, 225, 0, 0, 0, 44, 44, 44, 44, 44, 44, 44, 16, 16, 16, 16, 16, 16, 15, 2, 30, 2, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 9, 47, 263, 0, 0, 0, 48, 48, 48, 48, 48, 48, 48, 18, 18, 18, 18, 18, 18, 15, 2, 33, 3, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 10, 51, 300, 0, 0, 0, 53, 53, 53, 53, 53, 53, 53, 20, 20, 20, 20, 20, 20, 15, 2, 36, 3, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 11, 56, 351, 0, 0, 0, 57, 57, 57, 57, 57, 57, 57, 22, 22, 22, 22, 22, 22, 15, 5, 41, 4, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 12, 60, 402, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63, 24, 24, 24, 24, 24, 24, 15, 6, 45, 4, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 13, 65, 453, 0, 0, 0, 68, 68, 68, 68, 68, 68, 68, 26, 26, 26, 26, 26, 26, 15, 9, 48, 5, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 14, 69, 504, 0, 0, 0, 72, 72, 72, 72, 72, 72, 72, 28, 28, 28, 28, 28, 28, 15, 11, 53, 5, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 15, 78, 572, 0, 0, 0, 77, 77, 77, 77, 77, 77, 77, 30, 30, 30, 30, 30, 30, 15, 11, 56, 6, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 16, 89, 639, 0, 0, 0, 81, 81, 81, 81, 81, 81, 81, 32, 32, 32, 32, 32, 32, 15, 12, 59, 6, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 17, 98, 707, 0, 0, 0, 86, 86, 86, 86, 86, 86, 86, 34, 34, 34, 34, 34, 34, 15, 12, 62, 7, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 18, 108, 774, 0, 0, 0, 90, 90, 90, 90, 90, 90, 90, 36, 36, 36, 36, 36, 36, 15, 14, 63, 8, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 19, 117, 842, 0, 0, 0, 95, 95, 95, 95, 95, 95, 95, 38, 38, 38, 38, 38, 38, 15, 14, 66, 8, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 20, 128, 909, 0, 0, 0, 99, 99, 99, 99, 99, 99, 99, 40, 40, 40, 40, 40, 40, 15, 15, 69, 9, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 21, 137, 977, 0, 0, 0, 104, 104, 104, 104, 104, 104, 104, 42, 42, 42, 42, 42, 42, 15, 15, 72, 10, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 22, 143, 1068, 0, 0, 0, 108, 108, 108, 108, 108, 108, 108, 44, 44, 44, 44, 44, 44, 15, 15, 75, 11, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 23, 149, 1160, 0, 0, 0, 113, 113, 113, 113, 113, 113, 113, 46, 46, 46, 46, 46, 46, 15, 15, 78, 12, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 24, 155, 1251, 0, 0, 0, 117, 117, 117, 117, 117, 117, 117, 48, 48, 48, 48, 48, 48, 15, 17, 83, 13, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 25, 161, 1343, 0, 0, 0, 122, 122, 122, 122, 122, 122, 122, 50, 50, 50, 50, 50, 50, 15, 17, 86, 13, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 26, 167, 1434, 0, 0, 0, 128, 128, 128, 128, 128, 128, 128, 52, 52, 52, 52, 52, 52, 15, 17, 89, 14, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 27, 173, 1526, 0, 0, 0, 132, 132, 132, 132, 132, 132, 132, 54, 54, 54, 54, 54, 54, 15, 17, 92, 15, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 28, 179, 1617, 0, 0, 0, 137, 137, 137, 137, 137, 137, 137, 56, 56, 56, 56, 56, 56, 15, 18, 96, 16, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 29, 185, 1709, 0, 0, 0, 141, 141, 141, 141, 141, 141, 141, 58, 58, 58, 58, 58, 58, 15, 18, 99, 17, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 30, 191, 1800, 0, 0, 0, 146, 146, 146, 146, 146, 146, 146, 60, 60, 60, 60, 60, 60, 15, 18, 102, 18, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 31, 203, 2370, 6, 0, 6, 156, 156, 156, 156, 156, 156, 156, 62, 62, 62, 62, 62, 62, 15, 21, 111, 24, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 32, 213, 2940, 12, 0, 12, 165, 165, 165, 165, 165, 165, 165, 64, 64, 64, 64, 64, 64, 15, 24, 119, 29, 30, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 33, 225, 3510, 18, 0, 18, 176, 176, 176, 176, 176, 176, 176, 66, 66, 66, 66, 66, 66, 15, 27, 128, 35, 29, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 34, 237, 4080, 24, 0, 24, 185, 185, 185, 185, 185, 185, 185, 68, 68, 68, 68, 68, 68, 15, 30, 135, 41, 28, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 35, 249, 4650, 30, 0, 30, 195, 195, 195, 195, 195, 195, 195, 70, 70, 70, 70, 70, 70, 15, 33, 144, 47, 27, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 36, 260, 5220, 36, 0, 36, 206, 206, 206, 206, 206, 206, 206, 72, 72, 72, 72, 72, 72, 15, 36, 153, 52, 25, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 37, 272, 5790, 42, 0, 42, 215, 215, 215, 215, 215, 215, 215, 74, 74, 74, 74, 74, 74, 15, 39, 161, 58, 24, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 38, 284, 6360, 48, 0, 48, 225, 225, 225, 225, 225, 225, 225, 76, 76, 76, 76, 76, 76, 15, 42, 170, 64, 23, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 39, 294, 6930, 54, 0, 54, 234, 234, 234, 234, 234, 234, 234, 78, 78, 78, 78, 78, 78, 15, 45, 177, 69, 22, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 40, 306, 7500, 60, 0, 60, 245, 245, 245, 245, 245, 245, 245, 80, 80, 80, 80, 80, 80, 15, 48, 186, 75, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 41, 312, 7950, 63, 0, 63, 249, 249, 249, 249, 249, 249, 249, 82, 82, 82, 82, 82, 82, 15, 50, 191, 80, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 42, 318, 8400, 66, 0, 66, 254, 254, 254, 254, 254, 254, 254, 84, 84, 84, 84, 84, 84, 15, 51, 195, 84, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 43, 326, 11000, 69, 0, 69, 260, 260, 260, 260, 260, 260, 260, 86, 86, 86, 86, 86, 86, 15, 51, 200, 110, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 44, 332, 25000, 72, 0, 72, 264, 264, 264, 264, 264, 264, 264, 88, 88, 88, 88, 88, 88, 15, 53, 204, 250, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 45, 338, 35000, 75, 0, 75, 269, 269, 269, 269, 269, 269, 269, 90, 90, 90, 90, 90, 90, 15, 54, 209, 350, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 46, 344, 45000, 75, 0, 75, 273, 273, 273, 273, 273, 273, 273, 92, 92, 92, 92, 92, 92, 15, 66, 228, 450, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 47, 350, 55000, 75, 0, 75, 278, 278, 278, 278, 278, 278, 278, 94, 94, 94, 94, 94, 94, 15, 77, 248, 550, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 48, 356, 70000, 75, 0, 75, 282, 282, 282, 282, 282, 282, 282, 96, 96, 96, 96, 96, 96, 15, 89, 267, 700, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 49, 362, 85000, 75, 0, 75, 287, 287, 287, 287, 287, 287, 287, 98, 98, 98, 98, 98, 98, 15, 99, 287, 850, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 50, 368, 100000, 75, 0, 75, 291, 291, 291, 291, 291, 291, 291, 100, 100, 100, 100, 100, 100, 15, 111, 306, 1000, 21, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 51, 374, 130000, 80, 0, 80, 296, 296, 296, 296, 296, 296, 296, 102, 102, 102, 102, 102, 102, 17, 117, 347, 1300, 20, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 52, 380, 140000, 84, 0, 84, 300, 300, 300, 300, 300, 300, 300, 104, 104, 104, 104, 104, 104, 18, 122, 387, 1400, 20, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 53, 386, 150000, 89, 0, 89, 305, 305, 305, 305, 305, 305, 305, 106, 106, 106, 106, 106, 106, 20, 128, 426, 1500, 19, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 54, 392, 160000, 93, 0, 93, 309, 309, 309, 309, 309, 309, 309, 108, 108, 108, 108, 108, 108, 21, 134, 467, 1600, 19, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 55, 399, 170000, 98, 0, 98, 315, 315, 315, 315, 315, 315, 315, 110, 110, 110, 110, 110, 110, 23, 140, 507, 1700, 18, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 56, 405, 180000, 102, 0, 102, 320, 320, 320, 320, 320, 320, 320, 112, 112, 112, 112, 112, 112, 24, 144, 548, 1800, 17, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 57, 411, 190000, 107, 0, 107, 324, 324, 324, 324, 324, 324, 324, 114, 114, 114, 114, 114, 114, 26, 150, 588, 1900, 17, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 58, 417, 200000, 111, 0, 111, 329, 329, 329, 329, 329, 329, 329, 116, 116, 116, 116, 116, 116, 27, 156, 627, 2000, 16, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 59, 434, 400750, 130, 0, 125, 253, 253, 253, 253, 253, 253, 253, 118, 118, 118, 118, 118, 118, 19, 170, 700, 4008, 16, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 60, 476, 450813, 140, 0, 129, 258, 258, 258, 258, 258, 258, 258, 120, 120, 120, 120, 120, 120, 22, 185, 740, 4508, 15, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 61, 517, 500875, 145, 0, 130, 263, 263, 263, 263, 263, 263, 263, 122, 122, 122, 122, 122, 122, 26, 195, 780, 5009, 15, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 62, 559, 550938, 150, 0, 140, 268, 268, 268, 268, 268, 268, 268, 124, 124, 124, 124, 124, 124, 29, 210, 800, 5509, 15, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 63, 600, 601000, 155, 0, 160, 273, 273, 273, 273, 273, 273, 273, 126, 126, 126, 126, 126, 126, 32, 220, 825, 6010, 15, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 64, 563, 734167, 160, 0, 170, 277, 277, 277, 277, 277, 277, 277, 128, 128, 128, 128, 128, 128, 50, 241, 850, 7342, 15, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 65, 525, 867333, 165, 0, 180, 281, 281, 281, 281, 281, 281, 281, 130, 130, 130, 130, 130, 130, 67, 262, 875, 8673, 14, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 66, 488, 1000500, 170, 0, 190, 285, 285, 285, 285, 285, 285, 285, 132, 132, 132, 132, 132, 132, 85, 283, 904, 10005, 14, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 67, 498, 1013841, 175, 10, 200, 292, 292, 292, 292, 292, 292, 292, 134, 134, 134, 134, 134, 134, 81, 312, 1071, 10138, 14, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 68, 508, 1027182, 180, 20, 225, 300, 300, 300, 300, 300, 300, 300, 136, 136, 136, 136, 136, 136, 77, 341, 1238, 10272, 14, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 69, 519, 1040522, 185, 30, 239, 307, 307, 307, 307, 307, 307, 307, 138, 138, 138, 138, 138, 138, 72, 369, 1404, 10405, 14, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 70, 529, 1053863, 190, 40, 245, 315, 315, 315, 315, 315, 315, 315, 140, 140, 140, 140, 140, 140, 68, 398, 1571, 10539, 14, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 71, 539, 1067204, 200, 50, 255, 322, 322, 322, 322, 322, 322, 322, 142, 142, 142, 142, 142, 142, 64, 427, 1738, 10672, 14, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 72, 547, 1262903, 220, 56, 265, 327, 327, 327, 327, 327, 327, 327, 144, 144, 144, 144, 144, 144, 68, 520, 1979, 12629, 14, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 73, 556, 1458602, 253, 63, 285, 332, 332, 332, 332, 332, 332, 332, 146, 146, 146, 146, 146, 146, 72, 614, 2219, 14586, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 74, 564, 1654301, 306, 69, 300, 337, 337, 337, 337, 337, 337, 337, 148, 148, 148, 148, 148, 148, 76, 707, 2460, 16543, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 75, 572, 1850000, 330, 75, 310, 342, 342, 342, 342, 342, 342, 342, 150, 150, 150, 150, 150, 150, 80, 725, 2700, 18500, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 76, 579, 1900000, 348, 77, 320, 346, 346, 346, 346, 346, 346, 346, 152, 152, 152, 152, 152, 152, 84, 750, 2960, 19000, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 77, 586, 1950000, 355, 79, 330, 350, 350, 350, 350, 350, 350, 350, 154, 154, 154, 154, 154, 154, 88, 775, 3000, 19500, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 78, 594, 2000000, 365, 81, 340, 354, 354, 354, 354, 354, 354, 354, 156, 156, 156, 156, 156, 156, 92, 800, 3100, 20000, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 79, 601, 2050000, 375, 83, 350, 358, 358, 358, 358, 358, 358, 358, 158, 158, 158, 158, 158, 158, 96, 825, 3200, 20500, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 80, 608, 2100000, 380, 85, 360, 362, 362, 362, 362, 362, 362, 362, 160, 160, 160, 160, 160, 160, 100, 850, 3300, 21000, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 81, 615, 2480000, 385, 83, 370, 366, 366, 366, 366, 366, 366, 366, 162, 162, 162, 162, 162, 162, 104, 875, 3350, 24800, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 82, 622, 2860000, 390, 81, 380, 370, 370, 370, 370, 370, 370, 370, 164, 164, 164, 164, 164, 164, 108, 900, 3400, 28600, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 83, 629, 3240000, 395, 79, 390, 375, 375, 375, 375, 375, 375, 375, 166, 166, 166, 166, 166, 166, 112, 925, 3450, 32400, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 84, 636, 3620000, 400, 77, 400, 379, 379, 379, 379, 379, 379, 379, 168, 168, 168, 168, 168, 168, 116, 940, 3500, 36200, 13, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 85, 643, 4000000, 405, 75, 410, 383, 383, 383, 383, 383, 383, 383, 170, 170, 170, 170, 170, 170, 120, 960, 3550, 40000, 12, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 86, 650, 4800000, 410, 77, 420, 387, 387, 387, 387, 387, 387, 387, 172, 172, 172, 172, 172, 172, 124, 980, 3600, 48000, 12, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 87, 657, 5600000, 415, 79, 430, 391, 391, 391, 391, 391, 391, 391, 174, 174, 174, 174, 174, 174, 128, 1000, 3650, 56000, 12, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 88, 665, 6400000, 420, 81, 440, 395, 395, 395, 395, 395, 395, 395, 176, 176, 176, 176, 176, 176, 132, 1010, 3700, 64000, 12, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 89, 672, 7200000, 420, 83, 445, 399, 399, 399, 399, 399, 399, 399, 178, 178, 178, 178, 178, 178, 136, 1018, 3800, 72000, 12, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1'),
	(2, 90, 679, 8000000, 420, 85, 450, 403, 403, 403, 403, 403, 403, 403, 180, 180, 180, 180, 180, 180, 140, 1050, 3900, 80000, 12, 100, 100, '1,1^2,1^8,1^13,1^14,1^15,1^16,1^17,1^21,1^31,1');
)",
	},
	ManifestEntry{
		.version = 9135,
		.description = "2019_01_10_multi_version_spawns.sql",
		.check = "SHOW COLUMNS FROM `spawn2` LIKE 'version'",
		.condition = "contains",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `spawn2` MODIFY `version` SMALLINT(5) SIGNED NOT NULL DEFAULT '0';
)",
	},
	ManifestEntry{
		.version = 9136,
		.description = "2019_02_04_profanity_command.sql",
		.check = "SHOW TABLES LIKE 'profanity_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
DROP TABLE IF EXISTS `profanity_list`;

CREATE TABLE `profanity_list` (
	`word` VARCHAR(16) NOT NULL
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

REPLACE INTO `command_settings` VALUES ('profanity', 150, 'prof');

)",
	},
	ManifestEntry{
		.version = 9137,
		.description = "2018_12_12_client_faction_tables.sql",
		.check = "SHOW TABLES LIKE 'faction_base_data'",
		.condition = "empty",
		.match = "",
		.sql = R"(
--
-- Table structure for table `client_faction_associations`
--

DROP TABLE IF EXISTS `client_faction_associations`;

CREATE TABLE `client_faction_associations` (
  `faction_id` int(11) NOT NULL,
  `other_faction_id` int(11) NOT NULL,
  `mod` int(11) DEFAULT NULL,
  PRIMARY KEY (`faction_id`,`other_faction_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
)",
	},
	ManifestEntry{
		.version = 9139,
		.description = "2019_03_25_optional_npc_model.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'model'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `model` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `stuck_behavior`;
)",
	},
	ManifestEntry{
		.version = 9140,
		.description = "2019_07_03_update_range.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'max_movement_update_range'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD COLUMN `max_movement_update_range` INT(11) UNSIGNED NOT NULL DEFAULT '600' AFTER `npc_max_aggro_dist`;
)",
	},
	ManifestEntry{
		.version = 9141,
		.description = "2019_07_10_npc_flymode.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'flymode'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `flymode` tinyint(4) NOT NULL DEFAULT -1;
)",
	},
	ManifestEntry{
		.version = 9142,
		.description = "2019_09_02_required_spawn_filter.sql",
		.check = "SHOW COLUMNS FROM `spawnentry` LIKE 'condition_value_filter'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spawnentry` ADD COLUMN `condition_value_filter` MEDIUMINT(9) NOT NULL DEFAULT '1' AFTER `chance`;

)",
	},
	ManifestEntry{
		.version = 9143,
		.description = "2019_09_16_account_table_changes.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'ls_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account`
	DROP INDEX `name`,
	DROP INDEX `lsaccount_id`;

ALTER TABLE `account`
	ADD COLUMN `ls_id` VARCHAR(64) NULL DEFAULT 'eqemu' AFTER `status`;

ALTER TABLE `account`
	ADD UNIQUE INDEX `name_ls_id` (`name`, `ls_id`),
	ADD UNIQUE INDEX `ls_id_lsaccount_id` (`ls_id`, `lsaccount_id`);

)",
	},
	ManifestEntry{
		.version = 9145,
		.description = "2019_12_24_banned_ips_update.sql",
		.check = "SHOW TABLES LIKE 'Banned_IPs'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
RENAME TABLE `Banned_IPs` TO `Banned_IPs_`;

CREATE TABLE `banned_ips` (PRIMARY KEY (`ip_address`)) SELECT `ip_address`, `notes` FROM `Banned_IPs_`;

DROP TABLE IF EXISTS `Banned_IPs_`;

)",
	},
	ManifestEntry{
		.version = 9146,
		.description = "2020_01_10_character_soft_deletes.sql",
		.check = "SHOW COLUMNS FROM `character_data` LIKE 'deleted_at'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_data` ADD COLUMN `deleted_at` datetime NULL DEFAULT NULL;
)",
	},
	ManifestEntry{
		.version = 9147,
		.description = "2020_01_24_grid_centerpoint_wp.sql",
		.check = "SHOW COLUMNS FROM `grid_entries` LIKE 'centerpoint'",
		.condition = "empty",
		.match = "",
		.sql = R"(
alter table grid_entries add column `centerpoint` tinyint(4) not null default 0;
alter table spawngroup add column `wp_spawns` tinyint(1) unsigned not null default 0;
)",
	},
	ManifestEntry{
		.version = 9148,
		.description = "2020_01_28_corpse_guild_consent_id.sql",
		.check = "SHOW COLUMNS FROM `character_corpses` LIKE 'guild_consent_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_corpses` ADD COLUMN `guild_consent_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `time_of_death`;

)",
	},
	ManifestEntry{
		.version = 9149,
		.description = "2020_02_06_globalloot.sql",
		.check = "SHOW COLUMNS FROM `global_loot` LIKE 'hot_zone'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `global_loot` ADD `hot_zone` TINYINT NULL;


)",
	},
	ManifestEntry{
		.version = 9150,
		.description = "2020_02_06_aa_reset_on_death.sql",
		.check = "SHOW COLUMNS FROM `aa_ability` LIKE 'reset_on_death'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `aa_ability` ADD `reset_on_death` TINYINT(4) NOT NULL DEFAULT '0';
UPDATE `aa_ability` SET `reset_on_death` = '1' WHERE `id` = 6001;

)",
	},
	ManifestEntry{
		.version = 9151,
		.description = "2020_03_05_npc_always_aggro.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'always_aggro'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `always_aggro` tinyint(1) NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9152,
		.description = "2020_03_09_convert_myisam_to_innodb.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9152",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account_flags` ENGINE=InnoDB;
ALTER TABLE `account_ip` ENGINE=InnoDB;
ALTER TABLE `account` ENGINE=InnoDB;
ALTER TABLE `adventure_template_entry_flavor` ENGINE=InnoDB;
ALTER TABLE `adventure_template_entry` ENGINE=InnoDB;
ALTER TABLE `altadv_vars` ENGINE=InnoDB;
ALTER TABLE `alternate_currency` ENGINE=InnoDB;
ALTER TABLE `banned_ips` ENGINE=InnoDB;
ALTER TABLE `base_data` ENGINE=InnoDB;
ALTER TABLE `blocked_spells` ENGINE=InnoDB;
ALTER TABLE `buyer` ENGINE=InnoDB;
ALTER TABLE `char_create_combinations` ENGINE=InnoDB;
ALTER TABLE `char_create_point_allocations` ENGINE=InnoDB;
ALTER TABLE `character_activities` ENGINE=InnoDB;
ALTER TABLE `character_enabledtasks` ENGINE=InnoDB;
ALTER TABLE `character_tasks` ENGINE=InnoDB;
ALTER TABLE `chatchannels` ENGINE=InnoDB;
ALTER TABLE `completed_tasks` ENGINE=InnoDB;
ALTER TABLE `damageshieldtypes` ENGINE=InnoDB;
ALTER TABLE `discovered_items` ENGINE=InnoDB;
ALTER TABLE `eqtime` ENGINE=InnoDB;
ALTER TABLE `eventlog` ENGINE=InnoDB;
ALTER TABLE `faction_list_mod` ENGINE=InnoDB;
ALTER TABLE `faction_list` ENGINE=InnoDB;
ALTER TABLE `faction_values` ENGINE=InnoDB;
ALTER TABLE `friends` ENGINE=InnoDB;
ALTER TABLE `goallists` ENGINE=InnoDB;
ALTER TABLE `guild_bank` ENGINE=InnoDB;
ALTER TABLE `guild_members` ENGINE=InnoDB;
ALTER TABLE `guild_ranks` ENGINE=InnoDB;
ALTER TABLE `guild_relations` ENGINE=InnoDB;
ALTER TABLE `guilds` ENGINE=InnoDB;
ALTER TABLE `hackers` ENGINE=InnoDB;
ALTER TABLE `horses` ENGINE=InnoDB;
ALTER TABLE `inventory_versions` ENGINE=InnoDB;
ALTER TABLE `item_tick` ENGINE=InnoDB;
ALTER TABLE `items` ENGINE=InnoDB;
ALTER TABLE `keyring` ENGINE=InnoDB;
ALTER TABLE `launcher_zones` ENGINE=InnoDB;
ALTER TABLE `launcher` ENGINE=InnoDB;
ALTER TABLE `ldon_trap_entries` ENGINE=InnoDB;
ALTER TABLE `ldon_trap_templates` ENGINE=InnoDB;
ALTER TABLE `lfguild` ENGINE=InnoDB;
ALTER TABLE `lootdrop_entries` ENGINE=InnoDB;
ALTER TABLE `lootdrop` ENGINE=InnoDB;
ALTER TABLE `loottable_entries` ENGINE=InnoDB;
ALTER TABLE `loottable` ENGINE=InnoDB;
ALTER TABLE `mail` ENGINE=InnoDB;
ALTER TABLE `merc_armorinfo` ENGINE=InnoDB;
ALTER TABLE `merc_buffs` ENGINE=InnoDB;
ALTER TABLE `merc_inventory` ENGINE=InnoDB;
ALTER TABLE `merc_merchant_entries` ENGINE=InnoDB;
ALTER TABLE `merc_merchant_template_entries` ENGINE=InnoDB;
ALTER TABLE `merc_merchant_templates` ENGINE=InnoDB;
ALTER TABLE `merc_name_types` ENGINE=InnoDB;
ALTER TABLE `merc_npc_types` ENGINE=InnoDB;
ALTER TABLE `merc_spell_list_entries` ENGINE=InnoDB;
ALTER TABLE `merc_spell_lists` ENGINE=InnoDB;
ALTER TABLE `merc_stance_entries` ENGINE=InnoDB;
ALTER TABLE `merc_stats` ENGINE=InnoDB;
ALTER TABLE `merc_subtypes` ENGINE=InnoDB;
ALTER TABLE `merc_templates` ENGINE=InnoDB;
ALTER TABLE `merc_types` ENGINE=InnoDB;
ALTER TABLE `merc_weaponinfo` ENGINE=InnoDB;
ALTER TABLE `mercs` ENGINE=InnoDB;
ALTER TABLE `name_filter` ENGINE=InnoDB;
ALTER TABLE `npc_types` ENGINE=InnoDB;
ALTER TABLE `object_contents` ENGINE=InnoDB;
ALTER TABLE `petitions` ENGINE=InnoDB;
ALTER TABLE `pets_equipmentset_entries` ENGINE=InnoDB;
ALTER TABLE `pets_equipmentset` ENGINE=InnoDB;
ALTER TABLE `player_titlesets` ENGINE=InnoDB;
ALTER TABLE `proximities` ENGINE=InnoDB;
ALTER TABLE `races` ENGINE=InnoDB;
ALTER TABLE `raid_details` ENGINE=InnoDB;
ALTER TABLE `raid_leaders` ENGINE=InnoDB;
ALTER TABLE `raid_members` ENGINE=InnoDB;
ALTER TABLE `rule_sets` ENGINE=InnoDB;
ALTER TABLE `rule_values` ENGINE=InnoDB;
ALTER TABLE `saylink` ENGINE=InnoDB;
ALTER TABLE `sharedbank` ENGINE=InnoDB;
ALTER TABLE `skill_caps` ENGINE=InnoDB;
ALTER TABLE `spell_globals` ENGINE=InnoDB;
ALTER TABLE `spells_new` ENGINE=InnoDB;
ALTER TABLE `task_activities` ENGINE=InnoDB;
ALTER TABLE `tasks` ENGINE=InnoDB;
ALTER TABLE `tasksets` ENGINE=InnoDB;
ALTER TABLE `timers` ENGINE=InnoDB;
ALTER TABLE `titles` ENGINE=InnoDB;
ALTER TABLE `trader_audit` ENGINE=InnoDB;
ALTER TABLE `trader` ENGINE=InnoDB;
ALTER TABLE `tradeskill_recipe_entries` ENGINE=InnoDB;
ALTER TABLE `tradeskill_recipe` ENGINE=InnoDB;
ALTER TABLE `variables` ENGINE=InnoDB;
ALTER TABLE `veteran_reward_templates` ENGINE=InnoDB;
)",
	},
	ManifestEntry{
		.version = 9153,
		.description = "2020_05_09_items_subtype.sql",
		.check = "SHOW COLUMNS from `items` LIKE 'UNK219'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
ALTER TABLE `items` CHANGE `UNK219` `subtype` int(11) not null default '0';

)",
	},
	ManifestEntry{
		.version = 9154,
		.description = "2020_04_11_expansions_content_filters.sql",
		.check = "SHOW COLUMNS from `zone` LIKE 'min_expansion'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- zone
ALTER TABLE `zone` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone` ADD `content_flags` varchar(100) NULL;

-- doors
ALTER TABLE `doors` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `doors` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `doors` ADD `content_flags` varchar(100) NULL;

-- object
ALTER TABLE `object` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `object` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `object` ADD `content_flags` varchar(100) NULL;

-- spawn2
ALTER TABLE `spawn2` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `spawn2` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `spawn2` ADD `content_flags` varchar(100) NULL;

-- tradeskill_recipe
ALTER TABLE `tradeskill_recipe` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `tradeskill_recipe` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `tradeskill_recipe` ADD `content_flags` varchar(100) NULL;

-- merchantlist
ALTER TABLE `merchantlist` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `merchantlist` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `merchantlist` ADD `content_flags` varchar(100) NULL;

-- global_loot
ALTER TABLE `global_loot` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `global_loot` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `global_loot` ADD `content_flags` varchar(100) NULL;

-- fishing
ALTER TABLE `fishing` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `fishing` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `fishing` ADD `content_flags` varchar(100) NULL;

-- forage
ALTER TABLE `forage` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `forage` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `forage` ADD `content_flags` varchar(100) NULL;

-- ground_spawns
ALTER TABLE `ground_spawns` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `ground_spawns` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `ground_spawns` ADD `content_flags` varchar(100) NULL;

-- loottable
ALTER TABLE `loottable` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `loottable` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `loottable` ADD `content_flags` varchar(100) NULL;

-- lootdrop
ALTER TABLE `lootdrop` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `lootdrop` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `lootdrop` ADD `content_flags` varchar(100) NULL;

-- starting_items
ALTER TABLE `starting_items` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `starting_items` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `starting_items` ADD `content_flags` varchar(100) NULL;

-- start_zones
ALTER TABLE `start_zones` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `start_zones` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `start_zones` ADD `content_flags` varchar(100) NULL;

-- traps
ALTER TABLE `traps` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `traps` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `traps` ADD `content_flags` varchar(100) NULL;

-- zone_points
ALTER TABLE `zone_points` ADD `min_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone_points` ADD `max_expansion` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `zone_points` ADD `content_flags` varchar(100) NULL;

-- pok books
update doors set min_expansion = 4 where name like '%POKTELE%';

-- content flags
CREATE TABLE `content_flags` (`id` int AUTO_INCREMENT,`flag_name` varchar(75),`enabled` tinyint,`notes` text, PRIMARY KEY (id));

-- content flags disabled

ALTER TABLE `doors` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `fishing` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `forage` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `global_loot` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `ground_spawns` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `lootdrop` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `loottable` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `merchantlist` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `object` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `spawn2` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `start_zones` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `starting_items` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `tradeskill_recipe` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `traps` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `zone` ADD `content_flags_disabled` varchar(100) NULL;
ALTER TABLE `zone_points` ADD `content_flags_disabled` varchar(100) NULL;
)",
	},
	ManifestEntry{
		.version = 9155,
		.description = "2020_08_15_lootdrop_level_filtering.sql",
		.check = "SHOW COLUMNS from `lootdrop_entries` LIKE 'trivial_min_level'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `lootdrop_entries` CHANGE `minlevel` `trivial_min_level` tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `maxlevel` `trivial_max_level` tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` ADD COLUMN `npc_min_level` smallint unsigned NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `lootdrop_entries` ADD COLUMN `npc_max_level` smallint unsigned NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `trivial_min_level` `trivial_min_level` smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT '';
ALTER TABLE `lootdrop_entries` CHANGE `trivial_max_level` `trivial_max_level` smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT '';
UPDATE `lootdrop_entries` SET `trivial_max_level` = 0 WHERE `trivial_max_level` = 127;
)",
	},
	ManifestEntry{
		.version = 9156,
		.description = "2020_08_16_virtual_zonepoints.sql",
		.check = "SHOW COLUMNS from `zone_points` LIKE 'is_virtual'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone_points` ADD COLUMN `is_virtual` tinyint NOT NULL DEFAULT '0' COMMENT '' AFTER `content_flags_disabled`;
ALTER TABLE `zone_points` ADD COLUMN `height` int NOT NULL DEFAULT '0' COMMENT '';
ALTER TABLE `zone_points` ADD COLUMN `width` int NOT NULL DEFAULT '0' COMMENT '';
)",
	},
	ManifestEntry{
		.version = 9157,
		.description = "2020_09_02_pet_taunting.sql",
		.check = "SHOW COLUMNS from `character_pet_info` LIKE 'taunting'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_pet_info` ADD COLUMN `taunting` tinyint(1) NOT NULL DEFAULT '1' COMMENT '';

)",
	},
	ManifestEntry{
		.version = 9158,
		.description = "2020_12_09_underworld.sql",
		.check = "SHOW COLUMNS from `zone` LIKE 'underworld_teleport_index'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD COLUMN `underworld_teleport_index` INT(4) NOT NULL DEFAULT '0';
UPDATE `zone` SET `underworld` = '-2030' WHERE `zoneidnumber` = '71';
UPDATE `zone` SET `underworld_teleport_index` = '11' WHERE `zoneidnumber` = '71';
UPDATE `zone` SET `underworld_teleport_index` = '-1' WHERE `zoneidnumber` = '75';
UPDATE `zone` SET `underworld_teleport_index` = '-1' WHERE `zoneidnumber` = '150';


)",
	},
	ManifestEntry{
		.version = 9159,
		.description = "2020_12_22_expedition_system.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9159",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `expeditions` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`uuid` VARCHAR(36) NOT NULL,
	`dynamic_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`expedition_name` VARCHAR(128) NOT NULL,
	`leader_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`min_players` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`max_players` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`add_replay_on_join` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
	`is_locked` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `dynamic_zone_id` (`dynamic_zone_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `expedition_lockouts` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`expedition_id` INT(10) UNSIGNED NOT NULL,
	`event_name` VARCHAR(256) NOT NULL,
	`expire_time` DATETIME NOT NULL DEFAULT current_timestamp(),
	`duration` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`from_expedition_uuid` VARCHAR(36) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `expedition_id_event_name` (`expedition_id`, `event_name`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `expedition_members` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`expedition_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`character_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`is_current_member` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `expedition_id_character_id` (`expedition_id`, `character_id`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `character_expedition_lockouts` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`character_id` INT(10) UNSIGNED NOT NULL,
	`expedition_name` VARCHAR(128) NOT NULL,
	`event_name` VARCHAR(256) NOT NULL,
	`expire_time` DATETIME NOT NULL DEFAULT current_timestamp(),
	`duration` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`from_expedition_uuid` VARCHAR(36) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `character_id_expedition_name_event_name` (`character_id`, `expedition_name`, `event_name`)
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `dynamic_zones` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`instance_id` INT(10) NOT NULL DEFAULT 0,
	`type` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	`compass_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`compass_x` FLOAT NOT NULL DEFAULT 0,
	`compass_y` FLOAT NOT NULL DEFAULT 0,
	`compass_z` FLOAT NOT NULL DEFAULT 0,
	`safe_return_zone_id` INT(10) UNSIGNED NOT NULL DEFAULT 0,
	`safe_return_x` FLOAT NOT NULL DEFAULT 0,
	`safe_return_y` FLOAT NOT NULL DEFAULT 0,
	`safe_return_z` FLOAT NOT NULL DEFAULT 0,
	`safe_return_heading` FLOAT NOT NULL DEFAULT 0,
	`zone_in_x` FLOAT NOT NULL DEFAULT 0,
	`zone_in_y` FLOAT NOT NULL DEFAULT 0,
	`zone_in_z` FLOAT NOT NULL DEFAULT 0,
	`zone_in_heading` FLOAT NOT NULL DEFAULT 0,
	`has_zone_in` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	UNIQUE INDEX `instance_id` (`instance_id`)
)
COLLATE='utf8mb4_general_ci'
ENGINE=InnoDB
;

)",
	},
	ManifestEntry{
		.version = 9160,
		.description = "2021_02_14_npc_exp_mod.sql",
		.check = "SHOW COLUMNS from `npc_types` LIKE 'exp_mod'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types` ADD COLUMN `exp_mod` INT NOT NULL DEFAULT '100' AFTER `always_aggro`;

)",
	},
	ManifestEntry{
		.version = 9161,
		.description = "2021_02_15_npc_spell_entries_unsigned.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9161",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_spells_entries` MODIFY `spellid` SMALLINT(5) UNSIGNED NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9162,
		.description = "2021_02_17_server_scheduled_events.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9162",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `server_scheduled_events`
(
    `id`              int(11) NOT NULL AUTO_INCREMENT,
    `description`     varchar(255) DEFAULT NULL,
    `event_type`      varchar(100) DEFAULT NULL,
    `event_data`      text         DEFAULT NULL,
    `minute_start`    int(11) DEFAULT 0,
    `hour_start`      int(11) DEFAULT 0,
    `day_start`       int(11) DEFAULT 0,
    `month_start`     int(11) DEFAULT 0,
    `year_start`      int(11) DEFAULT 0,
    `minute_end`      int(11) DEFAULT 0,
    `hour_end`        int(11) DEFAULT 0,
    `day_end`         int(11) DEFAULT 0,
    `month_end`       int(11) DEFAULT 0,
    `year_end`        int(11) DEFAULT 0,
    `cron_expression` varchar(100) DEFAULT NULL,
    `created_at`      datetime     DEFAULT NULL,
    `deleted_at`      datetime     DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9163,
		.description = "2021_04_17_zone_safe_heading_changes.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'safe_heading'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE zone ADD COLUMN safe_heading float NOT NULL DEFAULT 0 AFTER safe_z;

)",
	},
	ManifestEntry{
		.version = 9164,
		.description = "2021_04_23_character_exp_modifiers.sql",
		.check = "SHOW TABLES LIKE 'character_exp_modifiers'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_exp_modifiers`  (
  `character_id` int NOT NULL,
  `zone_id` int NOT NULL,
  `aa_modifier` float NOT NULL,
  `exp_modifier` float NOT NULL,
  PRIMARY KEY (`character_id`, `zone_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

)",
	},
	ManifestEntry{
		.version = 9165,
		.description = "2021_04_28_idle_pathing.sql",
		.check = "SHOW COLUMNS FROM `spawn2` LIKE 'path_when_zone_idle'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Add new path_when_zone_idle flag to allow some spawns to path in empty zones
ALTER TABLE spawn2 ADD COLUMN path_when_zone_idle tinyint(1) NOT NULL DEFAULT 0 AFTER pathgrid;

-- Update spawns that used to path in empty zones because of their grid type
-- to behave the same using the new mechanism.  The code that checked path grid
-- types has been removed as it was coincidentally coupled to idle movement.
-- The new flag path_when_zone_idle is the new mechanism, and allows any moving
-- mob, not just those on grids, to path while the zone is idle.
UPDATE spawn2 s
LEFT JOIN zone z ON z.short_name = s.zone
LEFT JOIN grid g ON g.id = s.pathgrid AND g.zoneid = z.zoneidnumber
SET path_when_zone_idle = 1
WHERE pathgrid != 0 AND g.type IN (4, 6);

)",
	},
	ManifestEntry{
		.version = 9166,
		.description = "2021_02_12_dynamic_zone_members.sql",
		.check = "SHOW TABLES LIKE 'dynamic_zone_members'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `dynamic_zone_members` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `dynamic_zone_id` int(10) unsigned NOT NULL DEFAULT 0,
  `character_id` int(10) unsigned NOT NULL DEFAULT 0,
  `is_current_member` tinyint(3) unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  UNIQUE KEY `dynamic_zone_id_character_id` (`dynamic_zone_id`,`character_id`),
  KEY `character_id` (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE `expedition_members`;

)",
	},
	ManifestEntry{
		.version = 9167,
		.description = "2021_06_06_beastlord_pets.sql",
		.check = "SHOW TABLES LIKE 'pets_beastlord_data'",
		.condition = "empty",
		.match = "",
		.sql = R"(
SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for pets_beastlord_data
-- ----------------------------
DROP TABLE IF EXISTS `pets_beastlord_data`;
CREATE TABLE `pets_beastlord_data`  (
  `player_race` int UNSIGNED NOT NULL DEFAULT 1,
  `pet_race` int UNSIGNED NOT NULL DEFAULT 42,
  `texture` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `helm_texture` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `gender` tinyint UNSIGNED NOT NULL DEFAULT 2,
  `size_modifier` float UNSIGNED NULL DEFAULT 1,
  `face` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`player_race`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Records of pets_beastlord_data
-- ----------------------------
INSERT INTO `pets_beastlord_data` VALUES (2, 42, 2, 0, 2, 1, 0); -- Barbarian
INSERT INTO `pets_beastlord_data` VALUES (9, 91, 0, 0, 2, 2.5, 0); -- Troll
INSERT INTO `pets_beastlord_data` VALUES (10, 43, 3, 0, 2, 1, 0); -- Ogre
INSERT INTO `pets_beastlord_data` VALUES (128, 42, 0, 0, 1, 2, 0); -- Iksar
INSERT INTO `pets_beastlord_data` VALUES (130, 63, 0, 0, 2, 0.8, 0); -- Vah Shir

SET FOREIGN_KEY_CHECKS = 1;
)",
	},
	ManifestEntry{
		.version = 9168,
		.description = "2021_08_31_pvp_duration.sql",
		.check = "SHOW COLUMNS FROM `spells_new` LIKE 'pvp_duration'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `spells_new` CHANGE `field181` `pvp_duration` int(11) NOT NULL DEFAULT '0';
ALTER TABLE `spells_new` CHANGE `field182` `pvp_duration_cap` int(11) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9169,
		.description = "2021_06_06_dynamic_zone_moved_columns.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9169",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `dynamic_zones`
	ADD COLUMN `uuid` VARCHAR(36) NOT NULL COLLATE 'latin1_swedish_ci' AFTER `type`,
	ADD COLUMN `name` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `uuid`,
	ADD COLUMN `leader_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `name`,
	ADD COLUMN `min_players` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `leader_id`,
	ADD COLUMN `max_players` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `min_players`;

-- migrate any currently active expeditions
UPDATE dynamic_zones
INNER JOIN expeditions ON expeditions.dynamic_zone_id = dynamic_zones.id
SET
  dynamic_zones.uuid = expeditions.uuid,
  dynamic_zones.name = expeditions.expedition_name,
  dynamic_zones.leader_id = expeditions.leader_id,
  dynamic_zones.min_players = expeditions.min_players,
  dynamic_zones.max_players = expeditions.max_players;

ALTER TABLE `expeditions`
  DROP COLUMN `uuid`,
  DROP COLUMN `expedition_name`,
  DROP COLUMN `leader_id`,
  DROP COLUMN `min_players`,
  DROP COLUMN `max_players`;

)",
	},
	ManifestEntry{
		.version = 9170,
		.description = "2021_03_03_instance_safereturns.sql",
		.check = "SHOW TABLES LIKE 'character_instance_safereturns'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_instance_safereturns` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `character_id` int(10) unsigned NOT NULL,
  `instance_zone_id` int(11) NOT NULL DEFAULT 0,
  `instance_id` int(11) NOT NULL DEFAULT 0,
  `safe_zone_id` int(11) NOT NULL DEFAULT 0,
  `safe_x` float NOT NULL DEFAULT 0,
  `safe_y` float NOT NULL DEFAULT 0,
  `safe_z` float NOT NULL DEFAULT 0,
  `safe_heading` float NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `character_id` (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

)",
	},
	ManifestEntry{
		.version = 9171,
		.description = "2021_03_30_remove_dz_is_current_member.sql",
		.check = "SHOW COLUMNS FROM `dynamic_zone_members` LIKE 'is_current_member'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
-- remove any non-current members for new behavior
DELETE FROM `dynamic_zone_members`
WHERE is_current_member = 0;

ALTER TABLE `dynamic_zone_members`
  DROP COLUMN `is_current_member`;

)",
	},
	ManifestEntry{
		.version = 9172,
		.description = "2021_05_21_shared_tasks.sql",
		.check = "SHOW TABLES LIKE 'shared_tasks'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- shared task tables
CREATE TABLE `shared_tasks`
(
    `id`              bigint(20) NOT NULL AUTO_INCREMENT,
    `task_id`         int(11) DEFAULT NULL,
    `accepted_time`   datetime DEFAULT NULL,
    `expire_time`     datetime DEFAULT NULL,
    `completion_time` datetime DEFAULT NULL,
    `is_locked`       tinyint(1) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_members`
(
    `shared_task_id` bigint(20) NOT NULL,
    `character_id`   bigint(20) NOT NULL,
    `is_leader`      tinyint(4) DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_activity_state`
(
    `shared_task_id` bigint(20) NOT NULL,
    `activity_id`    int(11) NOT NULL,
    `done_count`     int(11) DEFAULT NULL,
    `updated_time`   datetime DEFAULT NULL,
    `completed_time` datetime DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `activity_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `shared_task_dynamic_zones`
(
    `shared_task_id`  bigint(20) NOT NULL,
    `dynamic_zone_id` int(10) unsigned NOT NULL,
    PRIMARY KEY (`shared_task_id`, `dynamic_zone_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- completed shared task tables - simply stores completed for reporting and logging

CREATE TABLE `completed_shared_tasks`
(
    `id`              bigint(20) NOT NULL,
    `task_id`         int(11) DEFAULT NULL,
    `accepted_time`   datetime DEFAULT NULL,
    `expire_time`     datetime DEFAULT NULL,
    `completion_time` datetime DEFAULT NULL,
    `is_locked`       tinyint(1) DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `completed_shared_task_members`
(
    `shared_task_id` bigint(20) NOT NULL,
    `character_id`   bigint(20) NOT NULL,
    `is_leader`      tinyint(4) DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `completed_shared_task_activity_state`
(
    `shared_task_id` bigint(20) NOT NULL,
    `activity_id`    int(11) NOT NULL,
    `done_count`     int(11) DEFAULT NULL,
    `updated_time`   datetime DEFAULT NULL,
    `completed_time` datetime DEFAULT NULL,
    PRIMARY KEY (`shared_task_id`, `activity_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- tasks

ALTER TABLE `tasks`
    ADD COLUMN `level_spread` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `maxlevel`,
  ADD COLUMN `min_players` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `level_spread`,
  ADD COLUMN `max_players` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `min_players`,
  ADD COLUMN `replay_timer_seconds` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `completion_emote`,
  ADD COLUMN `request_timer_seconds` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `replay_timer_seconds`;

-- character timers

CREATE TABLE `character_task_timers`
(
    `id`           int(10) unsigned NOT NULL AUTO_INCREMENT,
    `character_id` int(10) unsigned NOT NULL DEFAULT 0,
    `task_id`      int(10) unsigned NOT NULL DEFAULT 0,
    `timer_type`   int(11) NOT NULL DEFAULT 0,
    `expire_time`  datetime NOT NULL DEFAULT current_timestamp(),
    PRIMARY KEY (`id`),
    KEY            `character_id` (`character_id`),
    KEY            `task_id` (`task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `tasks`
    CHANGE COLUMN `completion_emote` `completion_emote` VARCHAR (512) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `faction_reward`;

ALTER TABLE `tasks`
    ADD COLUMN `reward_radiant_crystals` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `rewardmethod`,
  ADD COLUMN `reward_ebon_crystals` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `reward_radiant_crystals`;

)",
	},
	ManifestEntry{
		.version = 9173,
		.description = "2021_09_14_zone_lava_damage.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'lava_damage'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE zone ADD lava_damage INT(11) NULL DEFAULT '50' AFTER underworld_teleport_index, ADD min_lava_damage INT(11) NOT NULL DEFAULT '10' AFTER lava_damage;

)",
	},
	ManifestEntry{
		.version = 9174,
		.description = "2021_10_09_not_null_door_columns.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9174",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- update any null columns to non-null value first to avoid data truncation errors
-- this will likely only affect the buffer column
update `doors` set `doors`.`dest_x` = 0 where `doors`.`dest_x` is null;
update `doors` set `doors`.`dest_y` = 0 where `doors`.`dest_y` is null;
update `doors` set `doors`.`dest_z` = 0 where `doors`.`dest_z` is null;
update `doors` set `doors`.`dest_heading` = 0 where `doors`.`dest_heading` is null;
update `doors` set `doors`.`invert_state` = 0 where `doors`.`invert_state` is null;
update `doors` set `doors`.`incline` = 0 where `doors`.`incline` is null;
update `doors` set `doors`.`buffer` = 0 where `doors`.`buffer` is null;

ALTER TABLE `doors`
	CHANGE COLUMN `dest_x` `dest_x` FLOAT NOT NULL DEFAULT '0' AFTER `dest_instance`,
	CHANGE COLUMN `dest_y` `dest_y` FLOAT NOT NULL DEFAULT '0' AFTER `dest_x`,
	CHANGE COLUMN `dest_z` `dest_z` FLOAT NOT NULL DEFAULT '0' AFTER `dest_y`,
	CHANGE COLUMN `dest_heading` `dest_heading` FLOAT NOT NULL DEFAULT '0' AFTER `dest_z`,
	CHANGE COLUMN `invert_state` `invert_state` INT(11) NOT NULL DEFAULT '0' AFTER `dest_heading`,
	CHANGE COLUMN `incline` `incline` INT(11) NOT NULL DEFAULT '0' AFTER `invert_state`,
	CHANGE COLUMN `buffer` `buffer` FLOAT NOT NULL DEFAULT '0' AFTER `size`;

)",
	},
	ManifestEntry{
		.version = 9175,
		.description = "2022_01_02_expansion_default_value_all.sql",
		.check = "SHOW COLUMNS FROM `forage` LIKE 'min_expansion'",
		.condition = "contains",
		.match = "unsigned",
		.sql = R"(
-- forage

ALTER TABLE `forage` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `forage` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE forage set min_expansion = -1 where min_expansion = 0;
UPDATE forage set max_expansion = -1 where max_expansion = 0;

-- tradeskill_recipe

ALTER TABLE `tradeskill_recipe` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `tradeskill_recipe` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE tradeskill_recipe set min_expansion = -1 where min_expansion = 0;
UPDATE tradeskill_recipe set max_expansion = -1 where max_expansion = 0;

-- fishing

ALTER TABLE `fishing` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `fishing` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE fishing set min_expansion = -1 where min_expansion = 0;
UPDATE fishing set max_expansion = -1 where max_expansion = 0;

-- zone

ALTER TABLE `zone` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `zone` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE zone set min_expansion = -1 where min_expansion = 0;
UPDATE zone set max_expansion = -1 where max_expansion = 0;

-- traps

ALTER TABLE `traps` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `traps` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE traps set min_expansion = -1 where min_expansion = 0;
UPDATE traps set max_expansion = -1 where max_expansion = 0;

-- loottable

ALTER TABLE `loottable` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `loottable` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE loottable set min_expansion = -1 where min_expansion = 0;
UPDATE loottable set max_expansion = -1 where max_expansion = 0;

-- ground_spawns

ALTER TABLE `ground_spawns` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `ground_spawns` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE ground_spawns set min_expansion = -1 where min_expansion = 0;
UPDATE ground_spawns set max_expansion = -1 where max_expansion = 0;

-- starting_items

ALTER TABLE `starting_items` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `starting_items` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE starting_items set min_expansion = -1 where min_expansion = 0;
UPDATE starting_items set max_expansion = -1 where max_expansion = 0;

-- spawn2

ALTER TABLE `spawn2` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `spawn2` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE spawn2 set min_expansion = -1 where min_expansion = 0;
UPDATE spawn2 set max_expansion = -1 where max_expansion = 0;

-- zone_points

ALTER TABLE `zone_points` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `zone_points` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE zone_points set min_expansion = -1 where min_expansion = 0;
UPDATE zone_points set max_expansion = -1 where max_expansion = 0;

-- lootdrop

ALTER TABLE `lootdrop` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `lootdrop` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE lootdrop set min_expansion = -1 where min_expansion = 0;
UPDATE lootdrop set max_expansion = -1 where max_expansion = 0;

-- global_loot

ALTER TABLE `global_loot` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `global_loot` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE global_loot set min_expansion = -1 where min_expansion = 0;
UPDATE global_loot set max_expansion = -1 where max_expansion = 0;

-- doors

ALTER TABLE `doors` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `doors` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE doors set min_expansion = -1 where min_expansion = 0;
UPDATE doors set max_expansion = -1 where max_expansion = 0;

-- object

ALTER TABLE `object` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `object` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE object set min_expansion = -1 where min_expansion = 0;
UPDATE object set max_expansion = -1 where max_expansion = 0;

-- start_zones

ALTER TABLE `start_zones` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `start_zones` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE start_zones set min_expansion = -1 where min_expansion = 0;
UPDATE start_zones set max_expansion = -1 where max_expansion = 0;

-- merchantlist

ALTER TABLE `merchantlist` CHANGE `max_expansion` `max_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
ALTER TABLE `merchantlist` CHANGE `min_expansion` `min_expansion` tinyint(4)  NOT NULL DEFAULT -1  COMMENT '';
UPDATE merchantlist set min_expansion = -1 where min_expansion = 0;
UPDATE merchantlist set max_expansion = -1 where max_expansion = 0;

-- spawnentry
ALTER TABLE `spawnentry` ADD `min_expansion` tinyint(4)  NOT NULL DEFAULT -1;
ALTER TABLE `spawnentry` ADD `max_expansion` tinyint(4)  NOT NULL DEFAULT -1;
ALTER TABLE `spawnentry` ADD `content_flags` varchar(100) NULL;
ALTER TABLE `spawnentry` ADD `content_flags_disabled` varchar(100) NULL;

)",
	},
	ManifestEntry{
		.version = 9176,
		.description = "2022_01_10_checksum_verification.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'crc_eqgame'",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_eqgame', '0', 'Client CRC64 Checksum on: eqgame.exe', '2021-09-23 14:16:27');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_skillcaps', '0', 'Client CRC64 Checksum on: SkillCaps.txt', '2021-09-23 14:16:21');
INSERT INTO `variables` (`varname`, `value`, `information`, `ts`) VALUES ('crc_basedata', '0', 'Client CRC64 Checksum on: BaseData.txt','2021-09-23 14:16:21');

ALTER TABLE `account`
	ADD COLUMN `crc_eqgame` TEXT NULL AFTER `suspend_reason`,
	ADD COLUMN `crc_skillcaps` TEXT NULL AFTER `crc_eqgame`,
	ADD COLUMN `crc_basedata` TEXT NULL AFTER `crc_skillcaps`;

ALTER TABLE `account` CHANGE `suspendeduntil` `suspendeduntil` datetime  NULL  COMMENT '';

)",
	},
	ManifestEntry{
		.version = 9177,
		.description = "2022_03_06_table_structure_changes.sql",
		.check = "SHOW COLUMNS FROM `pets` LIKE 'id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `pets` DROP PRIMARY KEY;
ALTER TABLE `pets` ADD `id` int(20) PRIMARY KEY NOT NULL AUTO_INCREMENT FIRST;
CREATE UNIQUE INDEX `type_petpower` ON pets (type, petpower);

ALTER TABLE `horses` DROP PRIMARY KEY;
ALTER TABLE `horses` ADD `id` int(20) PRIMARY KEY NOT NULL AUTO_INCREMENT FIRST;
CREATE UNIQUE INDEX `filename` ON horses (filename);

ALTER TABLE books DROP INDEX `id`;
ALTER TABLE `books` ADD `id` int(20) PRIMARY KEY NOT NULL AUTO_INCREMENT FIRST;
CREATE UNIQUE INDEX `filename` ON books (name);

)",
	},
	ManifestEntry{
		.version = 9178,
		.description = "2022_03_07_saylink_collation.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9178",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE saylink CONVERT TO CHARACTER SET utf8 COLLATE utf8_bin;

)",
	},
	ManifestEntry{
		.version = 9179,
		.description = "2022_04_30_hp_regen_per_second.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'hp_regen_per_second'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE npc_types ADD COLUMN hp_regen_per_second bigint DEFAULT 0 AFTER hp_regen_rate;

)",
	},
	ManifestEntry{
		.version = 9180,
		.description = "2022_05_01_character_peqzone_flags.sql",
		.check = "SHOW TABLES LIKE 'character_peqzone_flags'",
		.condition = "empty",
		.match = "",
		.sql = R"(
SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for character_peqzone_flags
-- ----------------------------
DROP TABLE IF EXISTS `character_peqzone_flags`;
CREATE TABLE `character_peqzone_flags`  (
  `id` int NOT NULL DEFAULT 0,
  `zone_id` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`, `zone_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

SET FOREIGN_KEY_CHECKS = 1;

)",
	},
	ManifestEntry{
		.version = 9181,
		.description = "2022_05_03_task_activity_goal_match_list.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'goal_match_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE task_activities ADD goal_match_list text AFTER goalid;

)",
	},
	ManifestEntry{
		.version = 9182,
		.description = "2022_05_02_npc_types_int64.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'hp'",
		.condition = "missing",
		.match = "bigint",
		.sql = R"(
ALTER TABLE npc_types MODIFY COLUMN hp BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN mana BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN hp_regen_rate BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN mana_regen_rate BIGINT NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9183,
		.description = "2022_05_07_merchant_data_buckets.sql",
		.check = "SHOW COLUMNS FROM `merchantlist` LIKE 'bucket_comparison'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merchantlist`
ADD COLUMN `bucket_name` varchar(100) NOT NULL DEFAULT '' AFTER `probability`,
ADD COLUMN `bucket_value` varchar(100) NOT NULL DEFAULT '' AFTER `bucket_name`,
ADD COLUMN `bucket_comparison` tinyint UNSIGNED NULL DEFAULT 0 AFTER `bucket_value`;
)",
	},
	ManifestEntry{
		.version = 9184,
		.description = "2022_05_21_schema_consistency.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9184",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE npc_types MODIFY COLUMN hp BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN mana BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN hp_regen_rate BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN mana_regen_rate BIGINT NOT NULL DEFAULT 0;
ALTER TABLE npc_types MODIFY COLUMN hp_regen_per_second BIGINT NOT NULL DEFAULT 0;

)",
	},
	ManifestEntry{
		.version = 9185,
		.description = "2022_05_07_discord_webhooks.sql",
		.check = "SHOW TABLES LIKE 'discord_webhooks'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE discord_webhooks
(
    id           INT auto_increment primary key NULL,
    webhook_name varchar(100) NULL,
    webhook_url  varchar(255) NULL,
    created_at   DATETIME NULL,
    deleted_at   DATETIME NULL
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_general_ci;

ALTER TABLE logsys_categories
    ADD log_to_discord smallint(11) default 0 AFTER log_to_gmsay;
ALTER TABLE logsys_categories
    ADD discord_webhook_id int(11) default 0 AFTER log_to_discord;

)",
	},
	ManifestEntry{
		.version = 9186,
		.description = "2022_07_09_zone_expansion_deprecate.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'expansion'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(

)",
	},
	ManifestEntry{
		.version = 9187,
		.description = "2022_07_09_task_zone_version_matching.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'zone_version'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `task_activities` ADD COLUMN `zone_version` int(11) default -1 AFTER zones;

)",
	},
	ManifestEntry{
		.version = 9189,
		.description = "2022_07_10_character_task_rewarded.sql",
		.check = "SHOW COLUMNS FROM `character_tasks` LIKE 'was_rewarded'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_tasks`
	ADD COLUMN `was_rewarded` TINYINT NOT NULL DEFAULT '0' AFTER `acceptedtime`;

)",
	},
	ManifestEntry{
		.version = 9190,
		.description = "2022_07_13_task_reward_points.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'reward_points'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks`
	ADD COLUMN `reward_points` INT NOT NULL DEFAULT '0' AFTER `rewardmethod`,
	ADD COLUMN `reward_point_type` INT NOT NULL DEFAULT '0' AFTER `reward_points`;

-- convert don crystal points to new fields
UPDATE tasks SET reward_point_type = 4 WHERE reward_radiant_crystals > 0;
UPDATE tasks SET reward_point_type = 5 WHERE reward_ebon_crystals > 0;
UPDATE tasks SET reward_points = reward_radiant_crystals WHERE reward_radiant_crystals > 0;
UPDATE tasks SET reward_points = reward_ebon_crystals WHERE reward_ebon_crystals > 0;

ALTER TABLE `tasks`
	DROP COLUMN `reward_radiant_crystals`,
	DROP COLUMN `reward_ebon_crystals`;

)",
	},
	ManifestEntry{
		.version = 9191,
		.description = "2022_07_28_gm_state_changes.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'invulnerable'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account`
    ADD COLUMN `invulnerable` TINYINT(4) NULL DEFAULT '0' AFTER `gmspeed`,
    ADD COLUMN `flymode` TINYINT(4) NULL DEFAULT '0' AFTER `invulnerable`,
    ADD COLUMN `ignore_tells` TINYINT(4) NULL DEFAULT '0' AFTER `flymode`;

)",
	},
	ManifestEntry{
		.version = 9192,
		.description = "2022_07_13_task_lock_activity.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'lock_activity_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks`
  ADD COLUMN `lock_activity_id` INT NOT NULL DEFAULT '-1' AFTER `request_timer_seconds`;

)",
	},
	ManifestEntry{
		.version = 9193,
		.description = "2022_07_16_task_timer_groups.sql",
		.check = "SHOW COLUMNS FROM `tasks` LIKE 'replay_timer_group'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `tasks`
  ADD COLUMN `replay_timer_group` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `completion_emote`,
  ADD COLUMN `request_timer_group` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `replay_timer_seconds`;

ALTER TABLE `character_task_timers`
  ADD COLUMN `timer_group` INT NOT NULL DEFAULT '0' AFTER `timer_type`;

)",
	},
	ManifestEntry{
		.version = 9194,
		.description = "2022_07_23_dz_switch_id.sql",
		.check = "SHOW COLUMNS FROM `doors` LIKE 'dz_switch_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `doors`
  ADD COLUMN `dz_switch_id` INT NOT NULL DEFAULT '0' AFTER `is_ldon_door`;

ALTER TABLE `dynamic_zones`
  ADD COLUMN `dz_switch_id` INT NOT NULL DEFAULT '0' AFTER `max_players`;

)",
	},
	ManifestEntry{
		.version = 9195,
		.description = "2022_07_23_dz_templates.sql",
		.check = "SHOW TABLES like 'dynamic_zone_templates'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `dynamic_zone_templates` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `zone_id` int(11) NOT NULL DEFAULT 0,
  `zone_version` int(11) NOT NULL DEFAULT 0,
  `name` varchar(128) NOT NULL DEFAULT '',
  `min_players` int(11) NOT NULL DEFAULT 0,
  `max_players` int(11) NOT NULL DEFAULT 0,
  `duration_seconds` int(11) NOT NULL DEFAULT 0,
  `dz_switch_id` int(11) NOT NULL DEFAULT 0,
  `compass_zone_id` int(11) NOT NULL DEFAULT 0,
  `compass_x` float NOT NULL DEFAULT 0,
  `compass_y` float NOT NULL DEFAULT 0,
  `compass_z` float NOT NULL DEFAULT 0,
  `return_zone_id` int(11) NOT NULL DEFAULT 0,
  `return_x` float NOT NULL DEFAULT 0,
  `return_y` float NOT NULL DEFAULT 0,
  `return_z` float NOT NULL DEFAULT 0,
  `return_h` float NOT NULL DEFAULT 0,
  `override_zone_in` tinyint(4) NOT NULL DEFAULT 0,
  `zone_in_x` float NOT NULL DEFAULT 0,
  `zone_in_y` float NOT NULL DEFAULT 0,
  `zone_in_z` float NOT NULL DEFAULT 0,
  `zone_in_h` float NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `tasks`
  ADD COLUMN `dz_template_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `request_timer_seconds`;

)",
	},
	ManifestEntry{
		.version = 9196,
		.description = "2022_07_30_merchantlist_temp.sql",
		.check = "SHOW COLUMNS FROM `merchantlist_temp` LIKE 'zone_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merchantlist_temp` ADD COLUMN `zone_id` INT(11) NOT NULL DEFAULT '0' AFTER `slot`;
ALTER TABLE `merchantlist_temp` ADD COLUMN `instance_id` INT(11) NOT NULL DEFAULT '0' AFTER `zone_id`;
ALTER TABLE `merchantlist_temp` DROP PRIMARY KEY, ADD PRIMARY KEY (`npcid`, `slot`, `zone_id`, `instance_id`);

)",
	},
	ManifestEntry{
		.version = 9197,
		.description = "2022_08_01_drop_expansion_account.sql",
		.check = "SHOW COLUMNS FROM `account` LIKE 'expansion'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
ALTER TABLE `account` DROP `expansion`;

)",
	},
	ManifestEntry{
		.version = 9198,
		.description = "2022_08_14_exp_modifier_instance_versions.sql",
		.check = "SHOW COLUMNS FROM `character_exp_modifiers` LIKE 'instance_version'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE character_exp_modifiers
ADD COLUMN instance_version int NOT NULL DEFAULT -1 AFTER zone_id,
DROP PRIMARY KEY,
ADD PRIMARY KEY (character_id, zone_id, instance_version) USING BTREE;
)",
	},
	ManifestEntry{
		.version = 9199,
		.description = "2022_08_08_task_req_activity_id.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'req_activity_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `task_activities`
  ADD COLUMN `req_activity_id` INT SIGNED NOT NULL DEFAULT '-1' AFTER `activityid`;

)",
	},
	ManifestEntry{
		.version = 9200,
		.description = "2022_08_19_zone_expansion_consistency.sql",
		.check = "SELECT * FROM db_version WHERE version >= 9200",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone`
    ADD COLUMN `bypass_expansion_check` tinyint(3) NOT NULL DEFAULT 0 AFTER `expansion`;

UPDATE `zone` SET `bypass_expansion_check` = 1 WHERE `short_name`
IN (
    'befallenb',
    'commonlands',
    'freeportacademy',
    'freeportarena',
    'freeportcityhall',
    'freeporteast',
    'freeporthall',
    'freeportmilitia',
    'freeportsewers',
    'freeportwest',
    'guildhall',
    'guildlobby',
    'highpasshold',
    'highpasskeep',
    'innothuleb',
    'kithforest',
    'mistythicket',
    'moors',
    'nektulosa',
    'northro',
    'oceanoftears',
    'southro',
    'steamfontmts',
    'toxxulia'
);

)",
	},
	ManifestEntry{
		.version = 9201,
		.description = "2022_08_22_npc_types_heroic_strikethrough.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'heroic_strikethrough'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types`
ADD COLUMN `heroic_strikethrough` INT NOT NULL DEFAULT 0 AFTER `exp_mod`;
)",
	},
	ManifestEntry{
		.version = 9202,
		.description = "2022_08_24_task_activities_step.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'step'",
		.condition = "contains",
		.match = "unsigned",
		.sql = R"(
ALTER TABLE `task_activities` MODIFY `step` INT(11) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9203,
		.description = "2022_08_07_replace_task_goals.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'item_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- backup original since this is a complex migration
CREATE TABLE `task_activities_backup_9203` LIKE `task_activities`;
INSERT INTO `task_activities_backup_9203` SELECT * FROM `task_activities`;

ALTER TABLE `task_activities`
  CHANGE COLUMN `description_override` `description_override` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `goalcount`,
  ADD COLUMN `npc_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `description_override`,
  ADD COLUMN `npc_goal_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `npc_id`,
  ADD COLUMN `npc_match_list` TEXT NULL DEFAULT NULL AFTER `npc_goal_id`,
  ADD COLUMN `item_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `npc_match_list`,
  ADD COLUMN `item_goal_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `item_id`,
  ADD COLUMN `item_id_list` TEXT NULL DEFAULT NULL AFTER `item_goal_id`,
  CHANGE COLUMN `item_list` `item_list` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `item_id_list`,
  ADD COLUMN `dz_switch_id` INT(11) NOT NULL DEFAULT '0' AFTER `delivertonpc`,
  ADD COLUMN `min_x` FLOAT NOT NULL DEFAULT 0 AFTER `dz_switch_id`,
  ADD COLUMN `min_y` FLOAT NOT NULL DEFAULT 0 AFTER `min_x`,
  ADD COLUMN `min_z` FLOAT NOT NULL DEFAULT 0 AFTER `min_y`,
  ADD COLUMN `max_x` FLOAT NOT NULL DEFAULT 0 AFTER `min_z`,
  ADD COLUMN `max_y` FLOAT NOT NULL DEFAULT 0 AFTER `max_x`,
  ADD COLUMN `max_z` FLOAT NOT NULL DEFAULT 0 AFTER `max_y`,
  CHANGE COLUMN `skill_list` `skill_list` VARCHAR(64) NOT NULL DEFAULT '-1' COLLATE 'latin1_swedish_ci' AFTER `max_z`,
  CHANGE COLUMN `spell_list` `spell_list` VARCHAR(64) NOT NULL DEFAULT '0' COLLATE 'latin1_swedish_ci' AFTER `skill_list`;

-- move Explore (5) goalid proximities to the new location fields
-- does not migrate where zone was different and ignores lists (unsupported)
UPDATE `task_activities`
INNER JOIN `proximities`
  ON `task_activities`.`goalid` = `proximities`.`exploreid`
  AND CAST(`task_activities`.`zones` AS INT) = `proximities`.`zoneid`
SET
  `task_activities`.`goalid` = 0,
  `task_activities`.`min_x` = `proximities`.`minx`,
  `task_activities`.`min_y` = `proximities`.`miny`,
  `task_activities`.`min_z` = `proximities`.`minz`,
  `task_activities`.`max_x` = `proximities`.`maxx`,
  `task_activities`.`max_y` = `proximities`.`maxy`,
  `task_activities`.`max_z` = `proximities`.`maxz`
WHERE
  `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` = 5;

-- dz_switch_id for Touch (11)
UPDATE `task_activities`
SET `task_activities`.`dz_switch_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` = 11;

-- single item ids for Deliver (1), Loot (3), TradeSkill (6), Fish (7), Forage (8)
UPDATE `task_activities`
SET `task_activities`.`item_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- item goallist id
UPDATE `task_activities`
SET `task_activities`.`item_goal_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- item id match list
UPDATE `task_activities`
SET `task_activities`.`item_id_list` = `task_activities`.`goal_match_list`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (1, 3, 6, 7, 8);

-- single npc ids for Kill (2), SpeakWith (4)
UPDATE `task_activities`
SET `task_activities`.`npc_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 0
  AND `task_activities`.`activitytype` IN (2, 4);

-- npc goallist id
UPDATE `task_activities`
SET `task_activities`.`npc_goal_id` = `task_activities`.`goalid`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (2, 4);

-- npc match list
UPDATE `task_activities`
SET `task_activities`.`npc_match_list` = `task_activities`.`goal_match_list`
WHERE `task_activities`.`goalmethod` = 1
  AND `task_activities`.`activitytype` IN (2, 4);

-- delivertonpc npc_ids for Deliver (1), GiveCash (100)
UPDATE `task_activities`
SET `task_activities`.`npc_id` = `task_activities`.`delivertonpc`
WHERE `task_activities`.`activitytype` IN (1, 100);

ALTER TABLE `task_activities`
  DROP COLUMN `goalid`,
  DROP COLUMN `goal_match_list`,
  DROP COLUMN `delivertonpc`;

-- leave proximities table backup in case of regressions
ALTER TABLE `proximities` RENAME `proximities_backup_9203`;

)",
	},
	ManifestEntry{
		.version = 9204,
		.description = "2022_09_02_faction_association.sql",
		.check = "SHOW TABLES LIKE 'faction_association'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `faction_association` (
	`id` INT(10) NOT NULL,
	`id_1` INT(10) NOT NULL DEFAULT 0,
	`mod_1` FLOAT NOT NULL DEFAULT 0,
	`id_2` INT(10) NOT NULL DEFAULT 0,
	`mod_2` FLOAT NOT NULL DEFAULT 0,
	`id_3` INT(10) NOT NULL DEFAULT 0,
	`mod_3` FLOAT NOT NULL DEFAULT 0,
	`id_4` INT(10) NOT NULL DEFAULT 0,
	`mod_4` FLOAT NOT NULL DEFAULT 0,
	`id_5` INT(10) NOT NULL DEFAULT 0,
	`mod_5` FLOAT NOT NULL DEFAULT 0,
	`id_6` INT(10) NOT NULL DEFAULT 0,
	`mod_6` FLOAT NOT NULL DEFAULT 0,
	`id_7` INT(10) NOT NULL DEFAULT 0,
	`mod_7` FLOAT NOT NULL DEFAULT 0,
	`id_8` INT(10) NOT NULL DEFAULT 0,
	`mod_8` FLOAT NOT NULL DEFAULT 0,
	`id_9` INT(10) NOT NULL DEFAULT 0,
	`mod_9` FLOAT NOT NULL DEFAULT 0,
	`id_10` INT(10) NOT NULL DEFAULT 0,
	`mod_10` FLOAT NOT NULL DEFAULT 0,
	PRIMARY KEY(`id`)
);

ALTER TABLE `npc_types` ADD `faction_amount` INT(10) NOT NULL DEFAULT '0';
ALTER TABLE `tasks` ADD `faction_amount` INT(10) NOT NULL DEFAULT '0';

)",
	},
	ManifestEntry{
		.version = 9208,
		.description = "2022_09_25_task_concat_matchlists.sql",
		.check = "SHOW COLUMNS FROM `task_activities` LIKE 'npc_id'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
SET SESSION group_concat_max_len = 1048576;
SET collation_connection = latin1_swedish_ci;

-- backup original(s)
CREATE TABLE `goallists_backup_9_25_2022` LIKE `goallists`;
INSERT INTO `goallists_backup_9_25_2022` SELECT * FROM `goallists`;
CREATE TABLE `tasks_backup_9_25_2022` LIKE `tasks`;
INSERT INTO `tasks_backup_9_25_2022` SELECT * FROM `tasks`;

-- npc id
UPDATE `task_activities`
SET `task_activities`.`npc_match_list` = CONCAT_WS('|', `npc_match_list`, `npc_id`)
WHERE npc_id != 0;

-- npc_goal_id goallists
UPDATE `task_activities`
INNER JOIN
(
  SELECT `goallists`.`listid`, GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids`
  FROM `goallists`
  GROUP BY `goallists`.`listid`
) AS `goallist_group`
  ON `task_activities`.`npc_goal_id` = `goallist_group`.`listid`
SET `task_activities`.`npc_match_list` = CONCAT_WS('|', `npc_match_list`, `goallist_ids`)
WHERE npc_goal_id != 0;

-- item id
UPDATE `task_activities`
SET `task_activities`.`item_id_list` = CONCAT_WS('|', `item_id_list`, `item_id`)
WHERE item_id != 0;

-- item_goal_id goallists
UPDATE `task_activities`
INNER JOIN
(
  SELECT `goallists`.`listid`, GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids`
  FROM `goallists`
  GROUP BY `goallists`.`listid`
) AS `goallist_group`
  ON `task_activities`.`item_goal_id` = `goallist_group`.`listid`
SET `task_activities`.`item_id_list` = CONCAT_WS('|', `item_id_list`, `goallist_ids`)
WHERE item_goal_id != 0;

ALTER TABLE `task_activities`
  DROP COLUMN `npc_id`,
  DROP COLUMN `npc_goal_id`,
  DROP COLUMN `item_id`,
  DROP COLUMN `item_goal_id`;


-- Reward cleanup and task table cleanup

ALTER TABLE `tasks`
    CHANGE COLUMN `reward` `reward_text` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' AFTER `description`,
    CHANGE COLUMN `rewardid` `reward_id_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL AFTER `reward_text`,
    CHANGE COLUMN `cashreward` `cash_reward` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `reward_id_list`,
    CHANGE COLUMN `rewardmethod` `reward_method` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `xpreward`,
    CHANGE COLUMN `minlevel` `min_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `reward_point_type`,
    CHANGE COLUMN `maxlevel` `max_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `min_level`;

ALTER Table `tasks` CHANGE COLUMN `xpreward` `exp_reward` int(10) NOT NULL DEFAULT 0 AFTER `cash_reward`;

UPDATE tasks SET reward_id_list =
 (
     SELECT GROUP_CONCAT(`goallists`.`entry` ORDER BY `goallists`.`entry` SEPARATOR '|') AS `goallist_ids` FROM `goallists` WHERE listid = reward_id_list)
WHERE
reward_method = 1;

-- deprecated table
DROP table goallists;

)",
	},
	ManifestEntry{
		.version = 9209,
		.description = "2022_09_28_discord_webhooks.sql",
		.check = "SHOW COLUMNS FROM `logsys_categories` LIKE 'log_to_discord'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE logsys_categories
    ADD log_to_discord smallint(11) default 0 AFTER log_to_gmsay;
ALTER TABLE logsys_categories
    ADD discord_webhook_id int(11) default 0 AFTER log_to_discord;

)",
	},
	ManifestEntry{
		.version = 9213,
		.description = "2022_12_24_npc_keeps_sold_items.sql",
		.check = "SHOW COLUMNS FROM `npc_types` LIKE 'keeps_sold_items'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_types`
ADD COLUMN `keeps_sold_items` tinyint(1) UNSIGNED NOT NULL DEFAULT 1 AFTER `faction_amount`;
)",
	},
	ManifestEntry{
		.version = 9214,
		.description = "2022_12_24_character_exp_toggle.sql",
		.check = "SHOW COLUMNS FROM `character_data` LIKE 'exp_enabled'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_data`
ADD COLUMN `exp_enabled` tinyint(1) UNSIGNED NOT NULL DEFAULT 1 AFTER `exp`;

)",
	},
	ManifestEntry{
		.version = 9215,
		.description = "2023_01_08_zone_max_level.sql",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'max_level'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone`
ADD COLUMN `max_level` tinyint(3) UNSIGNED NOT NULL DEFAULT 255 AFTER `min_level`;
)",
	},
	ManifestEntry{
		.version = 9216,
		.description = "2023_01_15_merc_data.sql",
		.check = "SHOW TABLES LIKE 'mercs'",
		.condition = "empty",
		.match = "",
		.sql = R"(
SET NAMES utf8;
SET
FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `merc_armorinfo`;
CREATE TABLE `merc_armorinfo`
(
    `id`               int(11) NOT NULL AUTO_INCREMENT,
    `merc_npc_type_id` int(11) UNSIGNED NOT NULL,
    `minlevel`         tinyint(2) UNSIGNED NOT NULL DEFAULT 1,
    `maxlevel`         tinyint(2) UNSIGNED NOT NULL DEFAULT 255,
    `texture`          tinyint(2) UNSIGNED NOT NULL DEFAULT 0,
    `helmtexture`      tinyint(2) UNSIGNED NOT NULL DEFAULT 0,
    `armortint_id`     int(10) UNSIGNED NOT NULL DEFAULT 0,
    `armortint_red`    tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `armortint_green`  tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `armortint_blue`   tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 41 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_buffs`;
CREATE TABLE `merc_buffs`
(
    `MercBuffId`         int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `MercId`             int(10) UNSIGNED NOT NULL DEFAULT 0,
    `SpellId`            int(10) UNSIGNED NOT NULL DEFAULT 0,
    `CasterLevel`        int(10) UNSIGNED NOT NULL DEFAULT 0,
    `DurationFormula`    int(10) UNSIGNED NOT NULL DEFAULT 0,
    `TicsRemaining`      int(11) NOT NULL DEFAULT 0,
    `PoisonCounters`     int(11) UNSIGNED NOT NULL DEFAULT 0,
    `DiseaseCounters`    int(11) UNSIGNED NOT NULL DEFAULT 0,
    `CurseCounters`      int(11) UNSIGNED NOT NULL DEFAULT 0,
    `CorruptionCounters` int(11) UNSIGNED NOT NULL DEFAULT 0,
    `HitCount`           int(10) UNSIGNED NOT NULL DEFAULT 0,
    `MeleeRune`          int(10) UNSIGNED NOT NULL DEFAULT 0,
    `MagicRune`          int(10) UNSIGNED NOT NULL DEFAULT 0,
    `dot_rune`           int(10) NOT NULL DEFAULT 0,
    `caston_x`           int(10) NOT NULL DEFAULT 0,
    `Persistent`         tinyint(1) NOT NULL DEFAULT 0,
    `caston_y`           int(10) NOT NULL DEFAULT 0,
    `caston_z`           int(10) NOT NULL DEFAULT 0,
    `ExtraDIChance`      int(10) NOT NULL DEFAULT 0,
    PRIMARY KEY (`MercBuffId`) USING BTREE,
    INDEX                `FK_mercbuff_1`(`MercId`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_inventory`;
CREATE TABLE `merc_inventory`
(
    `merc_inventory_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_subtype_id`   int(10) UNSIGNED NOT NULL DEFAULT 0,
    `item_id`           int(11) UNSIGNED NOT NULL DEFAULT 0,
    `min_level`         int(10) UNSIGNED NOT NULL DEFAULT 0,
    `max_level`         int(10) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`merc_inventory_id`) USING BTREE,
    INDEX               `FK_merc_inventory_1`(`merc_subtype_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 42 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_merchant_entries`;
CREATE TABLE `merc_merchant_entries`
(
    `merc_merchant_entry_id`    int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_merchant_template_id` int(10) UNSIGNED NOT NULL,
    `merchant_id`               int(11) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_merchant_entry_id`) USING BTREE,
    INDEX                       `FK_merc_merchant_entries_1`(`merc_merchant_template_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 57 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_merchant_template_entries`;
CREATE TABLE `merc_merchant_template_entries`
(
    `merc_merchant_template_entry_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_merchant_template_id`       int(10) UNSIGNED NOT NULL,
    `merc_template_id`                int(10) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_merchant_template_entry_id`) USING BTREE,
    INDEX                             `FK_merc_merchant_template_entries_1`(`merc_merchant_template_id`) USING BTREE,
    INDEX                             `FK_merc_merchant_template_entries_2`(`merc_template_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 554 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_merchant_templates`;
CREATE TABLE `merc_merchant_templates`
(
    `merc_merchant_template_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `name`                      varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `qglobal`                   varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
    PRIMARY KEY (`merc_merchant_template_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 25 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_name_types`;
CREATE TABLE `merc_name_types`
(
    `name_type_id` int(10) UNSIGNED NOT NULL,
    `class_id`     int(10) UNSIGNED NOT NULL,
    `prefix`       varchar(25) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `suffix`       varchar(25) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    PRIMARY KEY (`name_type_id`, `class_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_npc_types`;
CREATE TABLE `merc_npc_types`
(
    `merc_npc_type_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `proficiency_id`   tinyint(3) UNSIGNED NOT NULL,
    `tier_id`          tinyint(3) UNSIGNED NOT NULL,
    `class_id`         int(10) UNSIGNED NOT NULL,
    `name`             varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
    PRIMARY KEY (`merc_npc_type_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 41 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_spell_list_entries`;
CREATE TABLE `merc_spell_list_entries`
(
    `merc_spell_list_entry_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_spell_list_id`       int(10) UNSIGNED NOT NULL,
    `spell_id`                 int(10) UNSIGNED NOT NULL,
    `spell_type`               int(10) UNSIGNED NOT NULL DEFAULT 0,
    `stance_id`                tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `minlevel`                 tinyint(3) UNSIGNED NOT NULL DEFAULT 1,
    `maxlevel`                 tinyint(3) UNSIGNED NOT NULL DEFAULT 255,
    `slot`                     tinyint(4) NOT NULL DEFAULT -1,
    `procChance`               tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`merc_spell_list_entry_id`) USING BTREE,
    INDEX                      `FK_merc_spell_lists_1`(`merc_spell_list_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 730 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_spell_lists`;
CREATE TABLE `merc_spell_lists`
(
    `merc_spell_list_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `class_id`           int(10) UNSIGNED NOT NULL,
    `proficiency_id`     tinyint(3) UNSIGNED NOT NULL,
    `name`               varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    PRIMARY KEY (`merc_spell_list_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 9 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_stance_entries`;
CREATE TABLE `merc_stance_entries`
(
    `merc_stance_entry_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `class_id`             int(10) UNSIGNED NOT NULL,
    `proficiency_id`       tinyint(3) UNSIGNED NOT NULL,
    `stance_id`            tinyint(3) UNSIGNED NOT NULL,
    `isdefault`            tinyint(1) NOT NULL,
    PRIMARY KEY (`merc_stance_entry_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 23 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_stats`;
CREATE TABLE `merc_stats`
(
    `merc_npc_type_id`  int(11) UNSIGNED NOT NULL,
    `clientlevel`       tinyint(2) UNSIGNED NOT NULL DEFAULT 1,
    `level`             tinyint(2) UNSIGNED NOT NULL DEFAULT 1,
    `hp`                int(11) NOT NULL DEFAULT 1,
    `mana`              int(11) NOT NULL DEFAULT 0,
    `AC`                smallint(5) NOT NULL DEFAULT 1,
    `ATK`               mediumint(9) NOT NULL DEFAULT 1,
    `STR`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `STA`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `DEX`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `AGI`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `_INT`              mediumint(8) UNSIGNED NOT NULL DEFAULT 80,
    `WIS`               mediumint(8) UNSIGNED NOT NULL DEFAULT 80,
    `CHA`               mediumint(8) UNSIGNED NOT NULL DEFAULT 75,
    `MR`                smallint(5) NOT NULL DEFAULT 15,
    `CR`                smallint(5) NOT NULL DEFAULT 15,
    `DR`                smallint(5) NOT NULL DEFAULT 15,
    `FR`                smallint(5) NOT NULL DEFAULT 15,
    `PR`                smallint(5) NOT NULL DEFAULT 15,
    `Corrup`            smallint(5) NOT NULL DEFAULT 15,
    `mindmg`            int(10) UNSIGNED NOT NULL DEFAULT 1,
    `maxdmg`            int(10) UNSIGNED NOT NULL DEFAULT 1,
    `attack_count`      smallint(6) NOT NULL DEFAULT 0,
    `attack_speed`      tinyint(3) NOT NULL DEFAULT 0,
    `attack_delay`      tinyint(3) UNSIGNED NOT NULL DEFAULT 30,
    `special_abilities` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
    `Accuracy`          mediumint(9) NOT NULL DEFAULT 0,
    `hp_regen_rate`     int(11) UNSIGNED NOT NULL DEFAULT 1,
    `mana_regen_rate`   int(11) UNSIGNED NOT NULL DEFAULT 1,
    `runspeed`          float NOT NULL DEFAULT 0,
    `statscale`         int(11) NOT NULL DEFAULT 100,
    `spellscale`        float NOT NULL DEFAULT 100,
    `healscale`         float NOT NULL DEFAULT 100,
    PRIMARY KEY (`merc_npc_type_id`, `clientlevel`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_subtypes`;
CREATE TABLE `merc_subtypes`
(
    `merc_subtype_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `class_id`        int(10) UNSIGNED NOT NULL,
    `tier_id`         tinyint(3) UNSIGNED NOT NULL,
    `confidence_id`   tinyint(3) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_subtype_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 21 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_templates`;
CREATE TABLE `merc_templates`
(
    `merc_template_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `merc_type_id`     int(10) UNSIGNED NOT NULL,
    `merc_subtype_id`  int(10) UNSIGNED NOT NULL,
    `merc_npc_type_id` int(11) UNSIGNED NOT NULL,
    `dbstring`         varchar(12) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `name_type_id`     tinyint(4) NOT NULL DEFAULT 0,
    `clientversion`    int(10) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_template_id`) USING BTREE,
    INDEX              `FK_merc_templates_1`(`merc_type_id`) USING BTREE,
    INDEX              `FK_merc_templates_2`(`merc_subtype_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 554 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_types`;
CREATE TABLE `merc_types`
(
    `merc_type_id`   int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `race_id`        int(10) UNSIGNED NOT NULL,
    `proficiency_id` tinyint(3) UNSIGNED NOT NULL,
    `dbstring`       varchar(12) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `clientversion`  int(10) UNSIGNED NOT NULL,
    PRIMARY KEY (`merc_type_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 49 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `merc_weaponinfo`;
CREATE TABLE `merc_weaponinfo`
(
    `id`               int(11) NOT NULL AUTO_INCREMENT,
    `merc_npc_type_id` int(11) NOT NULL,
    `minlevel`         tinyint(2) UNSIGNED NOT NULL DEFAULT 0,
    `maxlevel`         tinyint(2) UNSIGNED NOT NULL DEFAULT 0,
    `d_melee_texture1` int(11) NOT NULL DEFAULT 0,
    `d_melee_texture2` int(11) NOT NULL DEFAULT 0,
    `prim_melee_type`  tinyint(4) UNSIGNED NOT NULL DEFAULT 28,
    `sec_melee_type`   tinyint(4) UNSIGNED NOT NULL DEFAULT 28,
    PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 61 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

DROP TABLE IF EXISTS `mercs`;
CREATE TABLE `mercs`
(
    `MercID`           int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `OwnerCharacterID` int(10) UNSIGNED NOT NULL,
    `Slot`             tinyint(1) UNSIGNED NOT NULL DEFAULT 0,
    `Name`             varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
    `TemplateID`       int(10) UNSIGNED NOT NULL DEFAULT 0,
    `SuspendedTime`    int(11) UNSIGNED NOT NULL DEFAULT 0,
    `IsSuspended`      tinyint(1) UNSIGNED NOT NULL DEFAULT 0,
    `TimerRemaining`   int(11) UNSIGNED NOT NULL DEFAULT 0,
    `Gender`           tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `MercSize`         float NOT NULL DEFAULT 5,
    `StanceID`         tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
    `HP`               int(11) UNSIGNED NOT NULL DEFAULT 0,
    `Mana`             int(11) UNSIGNED NOT NULL DEFAULT 0,
    `Endurance`        int(11) UNSIGNED NOT NULL DEFAULT 0,
    `Face`             int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinHairStyle`  int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinHairColor`  int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinEyeColor`   int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinEyeColor2`  int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinBeardColor` int(10) UNSIGNED NOT NULL DEFAULT 1,
    `LuclinBeard`      int(10) UNSIGNED NOT NULL DEFAULT 0,
    `DrakkinHeritage`  int(10) UNSIGNED NOT NULL DEFAULT 0,
    `DrakkinTattoo`    int(10) UNSIGNED NOT NULL DEFAULT 0,
    `DrakkinDetails`   int(10) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`MercID`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

SET
FOREIGN_KEY_CHECKS = 1;

)",
	},
	ManifestEntry{
		.version = 9217,
		.description = "2023_01_15_chatchannel_reserved_names.sql",
		.check = "SHOW TABLES LIKE 'chatchannel_reserved_names'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `chatchannel_reserved_names`
(
    `id`   int(11) NOT NULL AUTO_INCREMENT,
    `name` varchar(64) NOT NULL,
    PRIMARY KEY (`id`) USING BTREE,
    UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

ALTER TABLE `chatchannels`
    ADD COLUMN `id` int(11) NOT NULL AUTO_INCREMENT FIRST,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`) USING BTREE,
ADD UNIQUE INDEX(`name`)

)",
	},
	ManifestEntry{
		.version = 9218,
		.description = "2023_01_24_item_recast.sql",
		.check = "show columns from character_item_recast like '%recast_type%'",
		.condition = "contains",
		.match = "smallint",
		.sql = R"(
ALTER TABLE `character_item_recast`
	CHANGE COLUMN `recast_type` `recast_type` INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `id`;
)",
	},
	ManifestEntry{
		.version = 9219,
		.description = "2023_01_29_merchant_status_requirements.sql",
		.check = "SHOW COLUMNS FROM merchantlist LIKE 'min_status'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `merchantlist`
ADD COLUMN `min_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `level_required`,
ADD COLUMN `max_status` tinyint(3) UNSIGNED NOT NULL DEFAULT 255 AFTER `min_status`;
)"
	},
	ManifestEntry{
		.version = 9220,
		.description = "2022_12_19_player_events_tables.sql",
		.check = "SHOW TABLES LIKE 'player_event_logs'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `player_event_log_settings`
(
    `id`                 bigint(20) NOT NULL,
    `event_name`         varchar(100) DEFAULT NULL,
    `event_enabled`      tinyint(1) DEFAULT NULL,
    `retention_days`     int(11) DEFAULT 0,
    `discord_webhook_id` int(11) DEFAULT 0,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `player_event_logs`
(
    `id`              bigint(20) NOT NULL AUTO_INCREMENT,
    `account_id`      bigint(20) DEFAULT NULL,
    `character_id`    bigint(20) DEFAULT NULL,
    `zone_id`         int(11) DEFAULT NULL,
    `instance_id`     int(11) DEFAULT NULL,
    `x`               float        DEFAULT NULL,
    `y`               float        DEFAULT NULL,
    `z`               float        DEFAULT NULL,
    `heading`         float        DEFAULT NULL,
    `event_type_id`   int(11) DEFAULT NULL,
    `event_type_name` varchar(255) DEFAULT NULL,
    `event_data`      longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL CHECK (json_valid(`event_data`)),
    `created_at`      datetime     DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY               `event_created_at` (`event_type_id`,`created_at`),
    KEY               `zone_id` (`zone_id`),
    KEY               `character_id` (`character_id`,`zone_id`) USING BTREE,
    KEY               `created_at` (`created_at`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4;

DROP TABLE `hackers`;
DROP TABLE `eventlog`;

)"
	},
	ManifestEntry{
		.version = 9221,
		.description = "2023_02_24_npc_scaling_zone_id_instance_version.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'zone_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_scale_global_base`
ADD COLUMN `zone_id` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `level`,
ADD COLUMN `instance_version` int(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `zone_id`,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`type`, `level`, `zone_id`, `instance_version`) USING BTREE;
)"
	},
	ManifestEntry{
		.version = 9222,
		.description = "2023_02_28_npc_scaling_zone_list_version_list.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'zone_id_list'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_scale_global_base`
    CHANGE COLUMN `zone_id` `zone_id_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER `level`,
    CHANGE COLUMN `instance_version` `instance_version_list` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER `zone_id_list`,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY (`type`, `level`, `zone_id_list`(255), `instance_version_list`(255)) USING BTREE;
)"
	}, ManifestEntry{
		.version = 9223,
		.description = "2023_03_04_npc_scale_global_base_heroic_strikethrough.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'heroic_strikethrough'",
		.condition = "empty",
		.match = "",
		.sql = R"(
UPDATE `npc_scale_global_base` SET ac = 0 WHERE ac IS NULL;
UPDATE `npc_scale_global_base` SET hp = 0 WHERE hp IS NULL;
UPDATE `npc_scale_global_base` SET accuracy = 0 WHERE accuracy IS NULL;
UPDATE `npc_scale_global_base` SET slow_mitigation = 0 WHERE slow_mitigation IS NULL;
UPDATE `npc_scale_global_base` SET attack = 0 WHERE attack IS NULL;
UPDATE `npc_scale_global_base` SET strength = 0 WHERE strength IS NULL;
UPDATE `npc_scale_global_base` SET stamina = 0 WHERE stamina IS NULL;
UPDATE `npc_scale_global_base` SET dexterity = 0 WHERE dexterity IS NULL;
UPDATE `npc_scale_global_base` SET agility = 0 WHERE agility IS NULL;
UPDATE `npc_scale_global_base` SET intelligence = 0 WHERE intelligence IS NULL;
UPDATE `npc_scale_global_base` SET wisdom = 0 WHERE wisdom IS NULL;
UPDATE `npc_scale_global_base` SET charisma = 0 WHERE charisma IS NULL;
UPDATE `npc_scale_global_base` SET magic_resist = 0 WHERE magic_resist IS NULL;
UPDATE `npc_scale_global_base` SET cold_resist = 0 WHERE cold_resist IS NULL;
UPDATE `npc_scale_global_base` SET fire_resist = 0 WHERE fire_resist IS NULL;
UPDATE `npc_scale_global_base` SET poison_resist = 0 WHERE poison_resist IS NULL;
UPDATE `npc_scale_global_base` SET disease_resist = 0 WHERE disease_resist IS NULL;
UPDATE `npc_scale_global_base` SET corruption_resist = 0 WHERE corruption_resist IS NULL;
UPDATE `npc_scale_global_base` SET physical_resist = 0 WHERE physical_resist IS NULL;
UPDATE `npc_scale_global_base` SET min_dmg = 0 WHERE min_dmg IS NULL;
UPDATE `npc_scale_global_base` SET max_dmg = 0 WHERE max_dmg IS NULL;
UPDATE `npc_scale_global_base` SET hp_regen_rate = 0 WHERE hp_regen_rate IS NULL;
UPDATE `npc_scale_global_base` SET attack_delay = 0 WHERE attack_delay IS NULL;
UPDATE `npc_scale_global_base` SET physical_resist = 0 WHERE physical_resist IS NULL;
UPDATE `npc_scale_global_base` SET spell_scale = 100 WHERE spell_scale IS NULL;
UPDATE `npc_scale_global_base` SET heal_scale = 100 WHERE heal_scale IS NULL;
UPDATE `npc_scale_global_base` SET special_abilities = '' WHERE special_abilities IS NULL;
ALTER TABLE `npc_scale_global_base`
    MODIFY COLUMN `ac` int(11) NOT NULL DEFAULT 0 AFTER `instance_version_list`,
    MODIFY COLUMN `hp` int(11) NOT NULL DEFAULT 0 AFTER `ac`,
    MODIFY COLUMN `accuracy` int(11) NOT NULL DEFAULT 0 AFTER `hp`,
    MODIFY COLUMN `slow_mitigation` int(11) NOT NULL DEFAULT 0 AFTER `accuracy`,
    MODIFY COLUMN `attack` int(11) NOT NULL DEFAULT 0 AFTER `slow_mitigation`,
    MODIFY COLUMN `strength` int(11) NOT NULL DEFAULT 0 AFTER `attack`,
    MODIFY COLUMN `stamina` int(11) NOT NULL DEFAULT 0 AFTER `strength`,
    MODIFY COLUMN `dexterity` int(11) NOT NULL DEFAULT 0 AFTER `stamina`,
    MODIFY COLUMN `agility` int(11) NOT NULL DEFAULT 0 AFTER `dexterity`,
    MODIFY COLUMN `intelligence` int(11) NOT NULL DEFAULT 0 AFTER `agility`,
    MODIFY COLUMN `wisdom` int(11) NOT NULL DEFAULT 0 AFTER `intelligence`,
    MODIFY COLUMN `charisma` int(11) NOT NULL DEFAULT 0 AFTER `wisdom`,
    MODIFY COLUMN `magic_resist` int(11) NOT NULL DEFAULT 0 AFTER `charisma`,
    MODIFY COLUMN `cold_resist` int(11) NOT NULL DEFAULT 0 AFTER `magic_resist`,
    MODIFY COLUMN `fire_resist` int(11) NOT NULL DEFAULT 0 AFTER `cold_resist`,
    MODIFY COLUMN `poison_resist` int(11) NOT NULL DEFAULT 0 AFTER `fire_resist`,
    MODIFY COLUMN `disease_resist` int(11) NOT NULL DEFAULT 0 AFTER `poison_resist`,
    MODIFY COLUMN `corruption_resist` int(11) NOT NULL DEFAULT 0 AFTER `disease_resist`,
    MODIFY COLUMN `physical_resist` int(11) NOT NULL DEFAULT 0 AFTER `corruption_resist`,
    MODIFY COLUMN `min_dmg` int(11) NOT NULL DEFAULT 0 AFTER `physical_resist`,
    MODIFY COLUMN `max_dmg` int(11) NOT NULL DEFAULT 0 AFTER `min_dmg`,
    MODIFY COLUMN `hp_regen_rate` int(11) NOT NULL DEFAULT 0 AFTER `max_dmg`,
    MODIFY COLUMN `attack_delay` int(11) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
    MODIFY COLUMN `spell_scale` int(11) NOT NULL DEFAULT 100 AFTER `attack_delay`,
    MODIFY COLUMN `heal_scale` int(11) NOT NULL DEFAULT 100 AFTER `spell_scale`,
    MODIFY COLUMN special_abilities text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER heal_scale,
    ADD COLUMN `heroic_strikethrough` int(11) NOT NULL DEFAULT 0 AFTER `heal_scale`;

)"
	},
	ManifestEntry{
		.version = 9224,
		.description = "2023_03_08_npc_scale_global_base_avoidance.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'hp_regen_per_second'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_scale_global_base`
MODIFY COLUMN `hp` bigint(20) NOT NULL DEFAULT 0 AFTER `ac`,
MODIFY COLUMN `hp_regen_rate` bigint(20) NOT NULL DEFAULT 0 AFTER `max_dmg`,
ADD COLUMN `hp_regen_per_second` bigint(20) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
ADD COLUMN `avoidance` int(11) unsigned NOT NULL DEFAULT 0 AFTER `heal_scale`;

)"
	},
	ManifestEntry{
		.version = 9225,
		.description = "2023_01_21_bots_raid_members.sql",
		.check = "SHOW COLUMNS FROM `raid_members` LIKE 'bot_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `npc_scale_global_base`
MODIFY COLUMN `hp` bigint(20) NOT NULL DEFAULT 0 AFTER `ac`,
MODIFY COLUMN `hp_regen_rate` bigint(20) NOT NULL DEFAULT 0 AFTER `max_dmg`,
ADD COLUMN `hp_regen_per_second` bigint(20) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
ADD COLUMN `avoidance` int(11) unsigned NOT NULL DEFAULT 0 AFTER `heal_scale`;

)"
	},
	ManifestEntry{
		.version = 9226,
		.description = "2023_03_17_corpse_fields.sql",
		.check = "SHOW COLUMNS FROM `character_corpse_items` LIKE 'custom_data'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `character_corpse_items`
	ADD COLUMN `custom_data` TEXT NULL AFTER `attuned`,
	ADD COLUMN `ornamenticon` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `custom_data`,
	ADD COLUMN `ornamentidfile` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamenticon`,
	ADD COLUMN `ornament_hero_model` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `ornamentidfile`;

)"
	},
	ManifestEntry{
		.version = 9227,
		.description = "2023_03_24_npc_scale_global_base_verify.sql",
		.check = "SHOW COLUMNS FROM `npc_scale_global_base` LIKE 'heroic_strikethrough'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
UPDATE `npc_scale_global_base` SET ac = 0 WHERE ac IS NULL;
UPDATE `npc_scale_global_base` SET hp = 0 WHERE hp IS NULL;
UPDATE `npc_scale_global_base` SET accuracy = 0 WHERE accuracy IS NULL;
UPDATE `npc_scale_global_base` SET slow_mitigation = 0 WHERE slow_mitigation IS NULL;
UPDATE `npc_scale_global_base` SET attack = 0 WHERE attack IS NULL;
UPDATE `npc_scale_global_base` SET strength = 0 WHERE strength IS NULL;
UPDATE `npc_scale_global_base` SET stamina = 0 WHERE stamina IS NULL;
UPDATE `npc_scale_global_base` SET dexterity = 0 WHERE dexterity IS NULL;
UPDATE `npc_scale_global_base` SET agility = 0 WHERE agility IS NULL;
UPDATE `npc_scale_global_base` SET intelligence = 0 WHERE intelligence IS NULL;
UPDATE `npc_scale_global_base` SET wisdom = 0 WHERE wisdom IS NULL;
UPDATE `npc_scale_global_base` SET charisma = 0 WHERE charisma IS NULL;
UPDATE `npc_scale_global_base` SET magic_resist = 0 WHERE magic_resist IS NULL;
UPDATE `npc_scale_global_base` SET cold_resist = 0 WHERE cold_resist IS NULL;
UPDATE `npc_scale_global_base` SET fire_resist = 0 WHERE fire_resist IS NULL;
UPDATE `npc_scale_global_base` SET poison_resist = 0 WHERE poison_resist IS NULL;
UPDATE `npc_scale_global_base` SET disease_resist = 0 WHERE disease_resist IS NULL;
UPDATE `npc_scale_global_base` SET corruption_resist = 0 WHERE corruption_resist IS NULL;
UPDATE `npc_scale_global_base` SET physical_resist = 0 WHERE physical_resist IS NULL;
UPDATE `npc_scale_global_base` SET min_dmg = 0 WHERE min_dmg IS NULL;
UPDATE `npc_scale_global_base` SET max_dmg = 0 WHERE max_dmg IS NULL;
UPDATE `npc_scale_global_base` SET hp_regen_rate = 0 WHERE hp_regen_rate IS NULL;
UPDATE `npc_scale_global_base` SET attack_delay = 0 WHERE attack_delay IS NULL;
UPDATE `npc_scale_global_base` SET physical_resist = 0 WHERE physical_resist IS NULL;
UPDATE `npc_scale_global_base` SET spell_scale = 100 WHERE spell_scale IS NULL;
UPDATE `npc_scale_global_base` SET heal_scale = 100 WHERE heal_scale IS NULL;
UPDATE `npc_scale_global_base` SET special_abilities = '' WHERE special_abilities IS NULL;
ALTER TABLE `npc_scale_global_base`
    MODIFY COLUMN `ac` int(11) NOT NULL DEFAULT 0 AFTER `instance_version_list`,
    MODIFY COLUMN `hp` bigint(20) NOT NULL DEFAULT 0 AFTER `ac`,
    MODIFY COLUMN `accuracy` int(11) NOT NULL DEFAULT 0 AFTER `hp`,
    MODIFY COLUMN `slow_mitigation` int(11) NOT NULL DEFAULT 0 AFTER `accuracy`,
    MODIFY COLUMN `attack` int(11) NOT NULL DEFAULT 0 AFTER `slow_mitigation`,
    MODIFY COLUMN `strength` int(11) NOT NULL DEFAULT 0 AFTER `attack`,
    MODIFY COLUMN `stamina` int(11) NOT NULL DEFAULT 0 AFTER `strength`,
    MODIFY COLUMN `dexterity` int(11) NOT NULL DEFAULT 0 AFTER `stamina`,
    MODIFY COLUMN `agility` int(11) NOT NULL DEFAULT 0 AFTER `dexterity`,
    MODIFY COLUMN `intelligence` int(11) NOT NULL DEFAULT 0 AFTER `agility`,
    MODIFY COLUMN `wisdom` int(11) NOT NULL DEFAULT 0 AFTER `intelligence`,
    MODIFY COLUMN `charisma` int(11) NOT NULL DEFAULT 0 AFTER `wisdom`,
    MODIFY COLUMN `magic_resist` int(11) NOT NULL DEFAULT 0 AFTER `charisma`,
    MODIFY COLUMN `cold_resist` int(11) NOT NULL DEFAULT 0 AFTER `magic_resist`,
    MODIFY COLUMN `fire_resist` int(11) NOT NULL DEFAULT 0 AFTER `cold_resist`,
    MODIFY COLUMN `poison_resist` int(11) NOT NULL DEFAULT 0 AFTER `fire_resist`,
    MODIFY COLUMN `disease_resist` int(11) NOT NULL DEFAULT 0 AFTER `poison_resist`,
    MODIFY COLUMN `corruption_resist` int(11) NOT NULL DEFAULT 0 AFTER `disease_resist`,
    MODIFY COLUMN `physical_resist` int(11) NOT NULL DEFAULT 0 AFTER `corruption_resist`,
    MODIFY COLUMN `min_dmg` int(11) NOT NULL DEFAULT 0 AFTER `physical_resist`,
    MODIFY COLUMN `max_dmg` int(11) NOT NULL DEFAULT 0 AFTER `min_dmg`,
    MODIFY COLUMN `hp_regen_rate` bigint(20) NOT NULL DEFAULT 0 AFTER `max_dmg`,
    MODIFY COLUMN `attack_delay` int(11) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
    MODIFY COLUMN `hp_regen_per_second` bigint(20) NOT NULL DEFAULT 0 AFTER `hp_regen_rate`,
    MODIFY COLUMN `spell_scale` int(11) NOT NULL DEFAULT 100 AFTER `attack_delay`,
    MODIFY COLUMN `heal_scale` int(11) NOT NULL DEFAULT 100 AFTER `spell_scale`,
    MODIFY COLUMN `heroic_strikethrough` int(11) NOT NULL DEFAULT 0 AFTER `avoidance`,
    MODIFY COLUMN `avoidance` int(11) unsigned NOT NULL DEFAULT 0 AFTER `heal_scale`,
    MODIFY COLUMN special_abilities text CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL AFTER heroic_strikethrough;

)"
	},

};
