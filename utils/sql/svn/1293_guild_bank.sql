CREATE TABLE `guild_bank` (
  `guildid` int(10) unsigned NOT NULL default '0',
  `area` tinyint(3) unsigned NOT NULL default '0',
  `slot` int(4) unsigned NOT NULL default '0',
  `itemid` int(10) unsigned NOT NULL default '0',
  `qty` int(10) unsigned NOT NULL default '0',
  `donator` varchar(64) default NULL,
  `permissions` tinyint(3) unsigned NOT NULL default '0',
  `whofor` varchar(64) default NULL,
  KEY `guildid` (`guildid`),
  KEY `area` (`area`),
  KEY `slot` (`slot`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

insert into rule_values values (1,'World:GuildBankZoneID','345', 'ID of zone the Guild Bank works in. Default 345, guildhall');
