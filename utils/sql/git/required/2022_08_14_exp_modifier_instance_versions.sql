ALTER TABLE character_exp_modifiers 
ADD COLUMN instance_version int NOT NULL DEFAULT -1 AFTER zone_id,
DROP PRIMARY KEY,
ADD PRIMARY KEY (character_id, zone_id, instance_version) USING BTREE;