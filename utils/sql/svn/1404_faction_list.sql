ALTER TABLE `faction_list` ADD COLUMN `mod_r42` smallint(6) NOT NULL default '0' AFTER `mod_r14`;
ALTER TABLE `faction_list` ADD COLUMN `mod_r367` smallint(6) NOT NULL default '0' AFTER `mod_r330`;
ALTER TABLE `faction_list` ADD COLUMN `mod_r522` smallint(6) NOT NULL default '0' AFTER `mod_r367`;
UPDATE faction_list SET mod_r367 = mod_r60;
UPDATE faction_list SET mod_r42 = mod_r120;
ALTER TABLE `faction_list` DROP COLUMN `mod_r60`;
ALTER TABLE `faction_list` DROP COLUMN `mod_r120`;