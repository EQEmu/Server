ALTER TABLE `npc_types`
ADD COLUMN `spellscale`  float NOT NULL DEFAULT 100 AFTER `emoteid`,
ADD COLUMN `healscale`  float NOT NULL DEFAULT 100 AFTER `spellscale`;