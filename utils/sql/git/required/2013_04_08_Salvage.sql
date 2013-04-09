-- Add row to tre
ALTER TABLE `tradeskill_recipe_entries` ADD `salvagecount` tinyint(2) DEFAULT '0' NOT NULL AFTER `componentcount`;

-- Fix level req on Salvage
UPDATE `altadv_vars` SET `level_inc` = '5' WHERE `skill_id` = '997';

-- Set aa_effects for Salvage
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2374', '997', '1', '313', '5', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2375', '998', '1', '313', '15', '0');
INSERT INTO `aa_effects` (`id`, `aaid`, `slot`, `effectid`, `base1`, `base2`) VALUES ('2376', '999', '1', '313', '25', '0');

