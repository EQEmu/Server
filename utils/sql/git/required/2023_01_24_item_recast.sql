ALTER TABLE `character_item_recast`
	CHANGE COLUMN `recast_type` `recast_type` INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER `id`;