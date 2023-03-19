ALTER TABLE `character_tasks`
	ADD COLUMN `was_rewarded` TINYINT NOT NULL DEFAULT '0' AFTER `acceptedtime`;
