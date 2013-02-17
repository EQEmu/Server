ALTER TABLE `items` 
ADD COLUMN `heroic_pr` smallint(6) NOT NULL default '0' AFTER `heroic_cha`,
ADD COLUMN `heroic_dr` smallint(6) NOT NULL default '0' AFTER `heroic_pr`,
ADD COLUMN `heroic_fr` smallint(6) NOT NULL default '0' AFTER `heroic_dr`,
ADD COLUMN `heroic_cr` smallint(6) NOT NULL default '0' AFTER `heroic_fr`,
ADD COLUMN `heroic_mr` smallint(6) NOT NULL default '0' AFTER `heroic_cr`,
ADD COLUMN `heroic_svcorrup` smallint(6) NOT NULL default '0' AFTER `heroic_mr`;