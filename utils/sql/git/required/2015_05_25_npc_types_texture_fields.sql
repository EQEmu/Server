ALTER TABLE npc_types
ADD COLUMN `armtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `raid_target`,
ADD COLUMN `bracertexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `armtexture`,
ADD COLUMN `handtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `bracertexture`,
ADD COLUMN `legtexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `handtexture`,
ADD COLUMN `feettexture` tinyint(2) NOT NULL DEFAULT '0' AFTER `legtexture`;