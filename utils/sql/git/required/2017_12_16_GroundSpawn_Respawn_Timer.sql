ALTER TABLE `ground_spawns` MODIFY `respawn_timer` int(11) unsigned NOT NULL default 300;
UPDATE `ground_spawns` SET `respawn_timer` = `respawn_timer` / 1000;
