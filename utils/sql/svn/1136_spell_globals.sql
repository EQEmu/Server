DROP TABLE IF EXISTS `spell_globals`;
CREATE TABLE `spell_globals` (
  `spellid` int(11) NOT NULL,
  `spell_name` varchar(64) NOT NULL default '',
  `qglobal` varchar(65) NOT NULL default '',
  `value` varchar(65) NOT NULL default '',
  PRIMARY KEY  (`spellid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

INSERT INTO `rule_values` VALUES ('1', 'Spells:EnableSpellGlobals', 'false', 'If true, spells check the spell_globals table against quest globals before allowing spells to scribe via quest::scribespell');