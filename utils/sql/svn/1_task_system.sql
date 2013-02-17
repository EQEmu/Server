
DROP TABLE IF EXISTS `tasks`;
CREATE TABLE `tasks` (
  `id` int(11) unsigned NOT NULL,
  `duration` int(11) unsigned NOT NULL,
  `title` varchar(100) NOT NULL,
  `description` varchar(2047) NOT NULL,
  `reward` varchar(64) NOT NULL,
  `rewardid` int(11) unsigned NOT NULL default '0',
  `cashreward` int(11) unsigned NOT NULL default '0',
  `xpreward` int(10) unsigned NOT NULL default '0',
  `rewardmethod` tinyint(3) unsigned NOT NULL default '2',
  `startzone` int(11) NOT NULL,
  `stepped` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `activities`;
CREATE TABLE `activities` (
  `taskid` int(11) unsigned NOT NULL,
  `activityid` int(11) unsigned NOT NULL default '0',
  `step` int(11) unsigned NOT NULL default '0',
  `activitytype` tinyint(3) unsigned NOT NULL default '0',
  `text1` varchar(64) NOT NULL,
  `text2` varchar(64) NOT NULL,
  `text3` varchar(128) NOT NULL,
  `goalid` int(11) unsigned NOT NULL default '0',
  `goalmethod` int(10) unsigned NOT NULL default '0',
  `goalcount` int(11) default '1',
  `delivertonpc` int(11) unsigned NOT NULL default '0',
  `zoneid` int(11) NOT NULL default '0',
  `optional` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`taskid`,`activityid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


INSERT INTO `activities` VALUES (1,0,1,1,'Guard Brookrock','Elven Bottle Of Wine','',36078,0,1,54124,54,0),(1,1,2,5,'Camp of Outsiders','','',1,0,1,0,54,0),(1,2,3,4,'Guard Brookrock','','',54124,0,1,0,54,0),(1,3,4,2,'Bandits','','',7,1,3,0,54,0),(1,4,5,4,'Guard Brookrock','','',54124,0,1,0,54,0),(1,5,6,5,'the entrance to Crushbone Citadel','','',2,0,1,0,54,0),(1,6,6,2,'Orcs','','',1,1,5,0,54,0),(1,7,7,4,'Guard Brookrock','','',54124,0,1,0,54,0),(1,8,8,4,'V\'Lynn Renloe','','',202291,0,1,0,202,0),(2,0,0,2,'Orcs','','',1,1,5,0,21,0),(2,1,0,3,'any creature','Rusty Items','',2,1,3,0,0,0),(2,2,0,5,'','','Locate the Antonica Spires in the Luclin Nexus',1,0,1,0,152,0),(3,0,0,7,'','Fish','',4,1,1,0,69,0),(3,1,0,8,'','Roots, berries, mushrooms or cherries','',3,1,1,0,69,0),(3,2,0,2,'','','Kill Pyzjn',4147,0,1,0,0,1),(4,0,0,11,'','','Locate the lair of Innoruuk',0,0,1,0,76,0),(4,1,0,11,'','','Find the lair of Cazic Thule',0,0,1,0,72,0),(5,0,0,3,'','','Recover the Ring of the Ancients',12268,0,1,0,0,0),(5,1,0,3,'','','Retrieve a Shadowed Rapier',7100,0,1,0,0,0),(5,2,2,1,'','','Return the Ring of the Ancients to Hasten Bootstrutter',12268,0,1,50320,50,0),(5,3,2,1,'','','Present the Shadowed Rapier to Hasten Bootstrutter',7100,0,1,50320,50,0),(5,4,2,100,'','','Give 3500 gold pieces to Hasten Bootstrutter',0,0,350000,50320,50,0),(6,0,0,6,'Class 1 Wood Point Arrows','Class 1 Wood Point Arrows','',8,1,10,0,0,0),(7,0,0,2,'Rats','','',9,1,10,0,4,0),(7,1,0,4,'Guard Philbin','','',4062,2,1,0,4,0),(7,2,0,2,'','','Kill 1 more rat please',9,1,1,0,4,0),(7,3,0,4,'Guard Philbin','','',4062,2,1,0,4,0),(8,0,0,2,'Bats','','',10,1,5,0,4,0),(8,1,0,4,'Guard Philbin','','',4062,2,1,0,4,0),(9,0,0,2,'Snakes','','',11,1,2,0,4,0),(9,1,0,4,'Guard Philbin','','',4062,2,1,0,4,0),(10,0,0,2,'Gnolls','','',12,1,3,0,4,0),(10,1,0,4,'Guard Philbin','','',4062,2,1,0,4,0),(11,0,0,2,'Skeletons','','',13,1,6,0,4,0),(11,1,0,4,'Guard Philbin','','',4062,2,1,0,4,0);


DROP TABLE IF EXISTS `character_tasks`;
CREATE TABLE `character_tasks` (
  `charid` int(11) unsigned NOT NULL,
  `taskid` int(11) unsigned NOT NULL,
  `slot` int(11) unsigned NOT NULL,
  `acceptedtime` int(11) unsigned default NULL,
  PRIMARY KEY  (`charid`,`taskid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `character_activities`;
CREATE TABLE `character_activities` (
  `charid` int(11) unsigned NOT NULL,
  `taskid` int(11) unsigned NOT NULL,
  `activityid` int(11) unsigned NOT NULL default '0',
  `donecount` int(11) unsigned NOT NULL default '0',
  `completed` tinyint(1) default '0',
  PRIMARY KEY  (`charid`,`taskid`,`activityid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `character_enabledtasks`;
CREATE TABLE `character_enabledtasks` (
  `charid` int(11) unsigned NOT NULL,
  `taskid` int(11) unsigned NOT NULL,
  PRIMARY KEY  (`charid`,`taskid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `completed_tasks`;
CREATE TABLE `completed_tasks` (
  `charid` int(11) unsigned NOT NULL,
  `completedtime` int(11) unsigned NOT NULL,
  `taskid` int(11) unsigned NOT NULL,
  `activityid` int(11) NOT NULL,
  PRIMARY KEY  (`charid`,`completedtime`,`taskid`,`activityid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `goallists`;
CREATE TABLE `goallists` (
  `listid` int(10) unsigned NOT NULL,
  `entry` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`listid`,`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


INSERT INTO `goallists` VALUES (1,5001),(1,5002),(1,5003),(1,5078),(1,5079),(1,5080),(1,5082),(1,5083),(1,5084),(1,5085),(1,5089),(1,5108),(1,5109),(1,5112),(1,5114),(1,5130),(1,9129),(1,10000),(1,10159),(1,10166),(1,20028),(1,20033),(1,20036),(1,20048),(1,20171),(1,21005),(1,21012),(1,21026),(1,21038),(1,21039),(1,21096),(1,21097),(1,21102),(1,21107),(1,21115),(1,21118),(1,21119),(1,21121),(1,22005),(1,22022),(1,22031),(1,22052),(1,22053),(1,22055),(1,22142),(1,22143),(1,22149),(1,22155),(1,22165),(1,22171),(1,22172),(1,22179),(1,30033),(1,30034),(1,30042),(1,33033),(1,33034),(1,33036),(1,33037),(1,33038),(1,33064),(1,33108),(1,33113),(1,33123),(1,33124),(1,33156),(1,34015),(1,34105),(1,35007),(1,35008),(1,35009),(1,35011),(1,35034),(1,35039),(1,35042),(1,35043),(1,35044),(1,35052),(1,35091),(1,35092),(1,35116),(1,35129),(1,37014),(1,37015),(1,37017),(1,37018),(1,37026),(1,37032),(1,37035),(1,37040),(1,37069),(1,50027),(1,50028),(1,50033),(1,50034),(1,50065),(1,50070),(1,50211),(1,50259),(1,51088),(1,51089),(1,51090),(1,51110),(1,51111),(1,54001),(1,54003),(1,54004),(1,54008),(1,54015),(1,54016),(1,54019),(1,54027),(1,54037),(1,54039),(1,54185),(1,54187),(1,54188),(1,54190),(1,54193),(1,54194),(1,54195),(1,54200),(1,57001),(1,57003),(1,57004),(1,57015),(1,57020),(1,57040),(1,57041),(1,57056),(1,57085),(1,57090),(1,57136),(1,58000),(1,58001),(1,58002),(1,58003),(1,58004),(1,58005),(1,58006),(1,58008),(1,58009),(1,58011),(1,58012),(1,58013),(1,58015),(1,58016),(1,58024),(1,58025),(1,58026),(1,58027),(1,58029),(1,58033),(1,58034),(1,58035),(1,58036),(1,58037),(1,58038),(1,58039),(1,58040),(1,58041),(1,58042),(1,58043),(1,58045),(1,58047),(1,58053),(1,58054),(1,58055),(1,68055),(1,68056),(1,68168),(1,68169),(1,68190),(1,68200),(1,68223),(1,70010),(1,70011),(1,70042),(1,255702),(1,269011),(1,269012),(2,2310),(2,5013),(2,5014),(2,5015),(2,5016),(2,5019),(2,5020),(2,5021),(2,5022),(2,5023),(2,5024),(2,5025),(2,5040),(2,5072),(2,5074),(2,5076),(2,6011),(2,6013),(2,6014),(2,6015),(2,6016),(2,6838),(2,7007),(2,7008),(2,7009),(2,7010),(2,7790),(2,13346),(2,13970),(2,16253),(2,19943),(2,19950),(2,20036),(2,20176),(2,20187),(2,20198),(2,20258),(2,20297),(2,20414),(2,26800),(2,30980),(2,31992),(2,36750),(2,60084),(2,67531),(2,70482),(2,86875),(2,86961),(2,89615),(2,94100),(2,94114),(2,94128),(2,94170),(2,94212),(3,13045),(3,13046),(3,13047),(3,13048),(3,13106),(3,13419),(3,14905),(3,14969),(3,22745),(3,31485),(4,13019),(4,22745),(5,2300),(5,17981),(6,5500),(6,5507),(6,8003),(7,54035),(7,54036),(7,54207),(7,54235),(7,54250),(7,54254),(8,8500),(8,8572),(8,8644),(9,4007),(9,4009),(9,4013),(9,4024),(9,4036),(9,4043),(9,4078),(9,4080),(10,4002),(10,4009),(10,4011),(10,4012),(10,4014),(10,4025),(10,4039),(10,4042),(10,4043),(10,4050),(10,4051),(10,4074),(10,4075),(10,4143),(11,4017),(11,4018),(11,4023),(11,4044),(12,4006),(12,4015),(12,4021),(12,4028),(12,4029),(12,4033),(12,4079),(12,4096),(12,4152),(12,4191),(13,4010),(13,4022),(13,4045),(13,4047),(13,4091),(13,4092),(13,4094),(13,4101),(13,4108),(13,4109),(13,4110),(13,4112),(13,4114),(13,4115),(13,4116),(13,4119),(13,4120),(13,4122),(13,4125),(13,4128),(13,4129),(13,4137),(13,4139),(13,4145),(13,4146),(13,4148),(13,4150);


DROP TABLE IF EXISTS `proximities`;
CREATE TABLE `proximities` (
  `zoneid` int(10) unsigned NOT NULL,
  `exploreid` int(10) unsigned NOT NULL,
  `minx` float(14,6) NOT NULL,
  `maxx` float(14,6) NOT NULL,
  `miny` float(14,6) NOT NULL,
  `maxy` float(14,6) NOT NULL,
  `minz` float(14,6) NOT NULL,
  `maxz` float(14,6) NOT NULL,
  PRIMARY KEY  (`zoneid`,`exploreid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `tasksets`;
CREATE TABLE `tasksets` (
  `id` int(11) unsigned NOT NULL,
  `taskid` int(11) unsigned NOT NULL,
  PRIMARY KEY  (`id`,`taskid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO `tasksets` VALUES (1,7),(1,8),(1,9),(1,10),(1,11);

drop table if exists `keyring`;
create table `keyring`(
`char_id` integer not null,
`item_id` integer not null
) engine=MyISAM DEFAULT CHARSET=latin1; 

insert into rule_values values (1,'TaskSystem:EnableTaskSystem','true');
insert into rule_values values (1,'TaskSystem:PeriodicCheckTimer',5);
insert into rule_values values (1,'TaskSystem:RecordCompletedTasks','true');
insert into rule_values values (1,'TaskSystem:RecordCompletedOptionalActivities','true');
insert into rule_values values (1,'TaskSystem:KeepOneRecordPerCompletedTask','true');
insert into rule_values values (1,'TaskSystem:EnableTaskProximity','true');
insert into rule_values values (1,'World:EnableTutorialButton','true');
insert into rule_values values (1,'World:EnableReturnHomeButton','true');
insert into rule_values values (1,'World:MaxLevelForTutorial','10');
insert into rule_values values (1,'World:MinOfflineTimeToReturnHome','21600');
Insert into rule_values values (1, 'World:AddMaxClientsPerIP', -1);
Insert into rule_values values (1, 'World:AddMaxClientsStatus', -1);
INSERT INTO `activities` VALUES ('12', '0', '0', '2', 'giant thicket rats', '', '', '14', '1', '10', '0', '33', '0');
INSERT INTO `activities` VALUES ('12', '1', '0', '3', 'any creature', 'rat whiskers', '', '13071', '0', '4', '0', '33', '0');
INSERT INTO `activities` VALUES ('12', '2', '1', '4', 'Ace Slighthand', '', '', '0', '0', '1', '0', '19', '0');
INSERT INTO `activities` VALUES ('12', '3', '2', '1', 'Swish Appletop', 'rat whiskers', '', '13071', '0', '4', '33145', '33', '0');
INSERT INTO `goallists` VALUES ('14', '33027');
INSERT INTO `goallists` VALUES ('14', '33039');
INSERT INTO `goallists` VALUES ('14', '33055');
INSERT INTO `tasks` VALUES ('12', '0', 'Extraordinary Rodents', '[1,2, Go to the Misty Thicket, kill 10 giant thicket rats and loot four rat whiskers.]', '', '0', '722', '100', '0', '19', '1');
INSERT INTO variables VALUES ('curInstFlagNum', 2000, 'Determines what instance flag will be handed out next', '2008-09-05 04:46:47');
ALTER TABLE `zone` ADD column `insttype` tinyint (1) zerofill unsigned NOT NULL default '0';
ALTER table character_ ADD column `instZflagNum` int(10) unsigned NOT NULL default '0';
ALTER table character_ ADD column `instZOrgID` int(11) NOT NULL default '0';
INSERT INTO variables VALUES ('dfltInstZflag',1000, 'Used to determine if a zone is instanced, must be 1000 or greater', '2008-09-05 04:46:47');

ALTER TABLE `spawngroup` ADD `dist` FLOAT NOT NULL DEFAULT '0.0',
ADD `max_x` FLOAT NOT NULL DEFAULT '0.0',
ADD `min_x` FLOAT NOT NULL DEFAULT '0.0',
ADD `max_y` FLOAT NOT NULL DEFAULT '0.0',
ADD `min_y` FLOAT NOT NULL DEFAULT '0.0',
ADD `delay` INT NOT NULL DEFAULT '0';

CREATE TABLE `raid_details` (
  `raidid` int(4) NOT NULL,
  `loottype` int(4) NOT NULL,
  `locked` tinyint(1) NOT NULL,
  PRIMARY KEY  (`raidid`)
);

CREATE TABLE `raid_members` (
  `raidid` int(4) NOT NULL,
  `charid` int(4) NOT NULL,
  `groupid` int(4) NOT NULL,
  `_class` tinyint(4) NOT NULL,
  `level` tinyint(4) NOT NULL,
  `name` varchar(64) NOT NULL,
  `isgroupleader` tinyint(1) NOT NULL,
  `israidleader` tinyint(1) NOT NULL,
  `islooter` tinyint(1) NOT NULL,
  PRIMARY KEY  (`charid`)
);
