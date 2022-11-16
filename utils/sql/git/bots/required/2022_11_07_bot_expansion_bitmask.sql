ALTER TABLE `bot_data`
ADD COLUMN `expansion_bitmask` int(11) NOT NULL DEFAULT -1 AFTER `stop_melee_level`;
