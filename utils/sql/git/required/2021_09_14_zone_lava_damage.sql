ALTER TABLE zone ADD lava_damage INT(11) NULL DEFAULT '50' AFTER underworld_teleport_index, ADD min_lava_damage INT(11) NOT NULL DEFAULT '10' AFTER lava_damage;
