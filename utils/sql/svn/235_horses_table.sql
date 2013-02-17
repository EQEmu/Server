DROP TABLE IF EXISTS `horses`;
CREATE TABLE `horses` (
  `filename` varchar(32) NOT NULL,
  `race` smallint(3) NOT NULL default '216',
  `gender` tinyint(1) NOT NULL default '0',
  `texture` tinyint(2) NOT NULL default '0',
  `mountspeed` float(4,2) NOT NULL default '0.75',
  `notes` varchar(64) default 'Notes',
  PRIMARY KEY  (`filename`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


INSERT INTO `horses` VALUES ('SumChimeraFast', '216', '0', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumCragslither1Fast', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumCragslither2Fast', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumCragslither3Fast', '216', '1', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBlFast', '216', '0', '2', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBlRun1', '216', '0', '2', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBlRun2', '216', '0', '2', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBlSlow1', '216', '0', '2', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBlSlow2', '216', '0', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBrFast', '216', '0', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBrRun1', '216', '0', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBrRun2', '216', '0', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBrSlow1', '216', '0', '0', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseBrSlow2', '216', '0', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseTaFast', '216', '0', '3', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseTaRun1', '216', '0', '3', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseTaRun2', '216', '0', '3', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseTaSlow1', '216', '0', '3', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseTaSlow2', '216', '0', '3', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseWhFast', '216', '0', '1', '1.75', '2871 - Summon Horse SumHorseWhFast');
INSERT INTO `horses` VALUES ('SumHorseWhRun1', '216', '0', '1', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseWhRun2', '216', '0', '1', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseWhSlow1', '216', '0', '1', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumHorseWhSlow2', '216', '0', '1', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumKirin0Fast', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumKirin2Fast', '216', '1', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardBlkFast', '216', '1', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardBlkRun1', '216', '1', '1', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardBlkRun2', '216', '1', '1', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardBlkSlow1', '216', '1', '1', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardBlkSlow2', '216', '1', '1', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardGrnFast', '216', '1', '2', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardGrnRun1', '216', '1', '2', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardGrnRun2', '216', '1', '2', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardGrnSlow1', '216', '1', '2', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardGrnSlow2', '216', '1', '2', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardRedFast', '216', '1', '3', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardRedRun1', '216', '1', '3', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardRedRun2', '216', '1', '3', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardRedSlow1', '216', '1', '3', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardRedSlow2', '216', '1', '3', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardWhtFast', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardWhtRun1', '216', '1', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardWhtRun2', '216', '1', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardWhtSlow1', '216', '1', '0', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumLizardWhtSlow2', '216', '1', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumNightmareFast', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumPuma1Fast', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumPuma3Fast', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumRoboboar', '472', '2', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumRoboboarFast', '472', '2', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumRoboboarRun1', '472', '2', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumRoboboarRun2', '472', '2', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumRoboboarSlow2', '472', '2', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumUnicornFast', '216', '0', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBlFast', '216', '0', '2', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBlRun1', '216', '0', '2', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBlRun2', '216', '0', '2', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBlSlow1', '216', '0', '2', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBlSlow2', '216', '0', '2', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBrFast', '216', '0', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBrRun1', '216', '0', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBrRun2', '216', '0', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBrSlow1', '216', '0', '0', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseBrSlow2', '216', '0', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseTaFast', '216', '0', '3', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseTaRun1', '216', '0', '3', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseTaRun2', '216', '0', '3', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseTaSlow1', '216', '0', '3', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseTaSlow2', '216', '0', '3', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseWhFast', '216', '0', '1', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseWhRun1', '216', '0', '1', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseWhRun2', '216', '0', '1', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseWhSlow1', '216', '0', '1', '0.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWarHorseWhSlow2', '216', '0', '1', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumWorgFastClaimDigital', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWorgFastClaimRetailBox', '216', '1', '0', '1.75', 'Notes');
INSERT INTO `horses` VALUES ('SumWorgRun1ClaimDigital', '216', '1', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumWorgRun1ClaimRetailBox', '216', '1', '0', '1.25', 'Notes');
INSERT INTO `horses` VALUES ('SumWorgRun2ClaimDigital', '216', '1', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumWorgRun2ClaimRetailBox', '216', '1', '0', '1.50', 'Notes');
INSERT INTO `horses` VALUES ('SumWorgSlow2ClaimDigital', '216', '1', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('SumWorgSlow2ClaimRetailBox', '216', '1', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('TestHorseA', '216', '0', '0', '1.00', 'Notes');
INSERT INTO `horses` VALUES ('TestWarHorseA', '216', '0', '0', '1.00', 'Notes');

