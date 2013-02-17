ALTER TABLE `altadv_vars` 
ADD COLUMN `aa_expansion` tinyint(3) unsigned NOT NULL default '3' AFTER `cost_inc`,
ADD COLUMN `special_category` int(10) unsigned NOT NULL default '4294967295' AFTER `aa_expansion`,
ADD COLUMN `sof_type` tinyint(3) unsigned NOT NULL default '1' AFTER `special_category`,
ADD COLUMN `sof_cost_inc` tinyint(3) NOT NULL default '0' AFTER `sof_type`,
ADD COLUMN `sof_max_level` tinyint(3) unsigned NOT NULL default '1' AFTER `sof_cost_inc`,
ADD COLUMN `sof_next_skill` int(10) unsigned NOT NULL default '0' AFTER `sof_max_level`,
ADD COLUMN `clientver` tinyint(3) unsigned NOT NULL default '1' AFTER `sof_next_skill`;