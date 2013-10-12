UPDATE `npc_spells_entries` SET `type` = 1024 WHERE `npc_spells_id` = 711 and `type` = 8;

REPLACE INTO `npc_spells_entries` (`npc_spells_id`, `spellid`, `type`, `minlevel`, `maxlevel`, `priority`) VALUES
(711, 717, 8, 5, 24, 1),
(711, 4395, 8, 25, 255, 2),
(711, 2605, 8, 49, 255, 1),
(711, 735, 8, 24, 64, 3),
(711, 2602, 8, 15, 64, 3),
(711, 1765, 8, 59, 255, 3),
(711, 2603, 8, 30, 64, 3);

-- 717	- "Selo's Accelerando"
-- 4395	- "Selo's Rhythm of Speed" (indoor usable)
-- 2605	- "Selo's Accelerating Chorus"
-- 735	- "Lyssa's Veracious Concord"
-- 2602	- "Song of Sustenance"
-- 1765	- "Solon's Charismatic Concord"
-- 2603	- "Amplification"