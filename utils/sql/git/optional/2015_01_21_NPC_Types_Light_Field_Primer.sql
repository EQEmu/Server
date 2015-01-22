-- 'greater lightstone' light level on 'wisp' race
UPDATE `npc_types` SET `light` = 11 WHERE `race` = 69;

-- 'fire beetle eye' light level on 'beetle' race with name criteria
UPDATE `npc_types` SET `light` = 12 WHERE `race` = 22 AND (`name` LIKE '%fire%' OR `name` LIKE '%lava%');