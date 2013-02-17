-- 
-- Table structure for table `chatchannels`
-- 

CREATE TABLE `chatchannels` (
  `name` varchar(64) NOT NULL,
  `owner` varchar(64) NOT NULL,
  `password` varchar(64) NOT NULL,
  `minstatus` int(5) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- 
-- Dumping data for table `chatchannels`
-- 

INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Newplayers', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('General', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Warrior', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Cleric', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Paladin', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Ranger', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Shadowknight', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Druid', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Monk', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Bard', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Rogue', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Shaman', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Necromancer', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Wizard', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Magician', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Enchanter', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Beastlord', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Berserker', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Antonica', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Odus', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Faydwer', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Kunark', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Velious', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Luclin', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Planes', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Brokenskull', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Taelosia', '*System*', '', 0);
INSERT INTO `chatchannels` (`name`, `owner`, `password`, `minstatus`) VALUES ('Kuua', '*System*', '', 0);
