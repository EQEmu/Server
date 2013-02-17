-- 
-- Table structure for table `aa_required_level_cost`
-- 

CREATE TABLE `aa_required_level_cost` (
  `skill_id` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL,
  `cost` int(10) unsigned NOT NULL default '0',
  `description` varchar(64) default NULL COMMENT 'For reference only',
  PRIMARY KEY  (`skill_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- 
-- Dumping data for table `aa_required_level_cost`
-- 

INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7800, 1, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7801, 6, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7802, 11, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7803, 16, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7804, 21, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7805, 26, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7806, 31, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7807, 36, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7808, 41, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7809, 46, 0, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7810, 51, 3, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7811, 56, 4, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7812, 61, 5, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7813, 66, 6, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7814, 71, 7, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7815, 76, 8, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7816, 81, 8, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7817, 83, 9, 'Harm Touch');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7850, 5, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7851, 11, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7852, 16, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7853, 21, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7854, 26, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7855, 31, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7856, 36, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7857, 41, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7858, 46, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7859, 51, 0, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7860, 56, 3, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7861, 61, 4, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7862, 66, 5, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7863, 71, 6, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7864, 76, 7, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7865, 81, 8, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (7866, 85, 9, 'Lay on Hands');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (593, 61, 2, 'Fervent Blessing');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (594, 63, 4, 'Fervent Blessing');
INSERT INTO `aa_required_level_cost` (`skill_id`, `level`, `cost`, `description`) VALUES (595, 65, 6, 'Fervent Blessing');
