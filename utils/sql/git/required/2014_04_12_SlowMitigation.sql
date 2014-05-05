-- Convert all values from FLOAT to INT
UPDATE npc_types SET slow_mitigation = slow_mitigation * 100;

-- Change variable type from FLOAT TO INT
ALTER TABLE npc_types MODIFY slow_mitigation smallint(4) NOT NULL DEFAULT  '0'; 


