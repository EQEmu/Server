ALTER TABLE `tasks`
	ADD COLUMN `reward_points` INT NOT NULL DEFAULT '0' AFTER `rewardmethod`,
	ADD COLUMN `reward_point_type` INT NOT NULL DEFAULT '0' AFTER `reward_points`;

-- convert don crystal points to new fields
UPDATE tasks SET reward_point_type = 4 WHERE reward_radiant_crystals > 0;
UPDATE tasks SET reward_point_type = 5 WHERE reward_ebon_crystals > 0;
UPDATE tasks SET reward_points = reward_radiant_crystals WHERE reward_radiant_crystals > 0;
UPDATE tasks SET reward_points = reward_ebon_crystals WHERE reward_ebon_crystals > 0;

ALTER TABLE `tasks`
	DROP COLUMN `reward_radiant_crystals`,
	DROP COLUMN `reward_ebon_crystals`;
