ALTER TABLE  `npc_types` ADD  `PhR` smallint( 5 ) UNSIGNED NOT NULL DEFAULT  '0' AFTER `Corrup`;

-- Approximate baseline live npc values based on extensive parsing.
UPDATE npc_types SET PhR = 10 WHERE PhR = 0 AND level <= 50;
UPDATE npc_types SET PhR = (10 + (level - 50))  WHERE PhR = 0 AND (level > 50 AND level <= 60);
UPDATE npc_types SET PhR = (20 + ((level - 60)*4))  WHERE PhR = 0 AND level > 60;

