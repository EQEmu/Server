UPDATE `pets` SET `npcID`=682 WHERE `type`='SwarmPetDG1' LIMIT 1;
UPDATE `pets` SET `npcID`=683 WHERE `type`='SwarmPetDG2' LIMIT 1;
UPDATE `pets` SET `npcID`=684 WHERE `type`='SwarmPetDG3' LIMIT 1;

UPDATE `npc_types` SET `id`=682 WHERE `id`=678 && `name`='SwarmPetDG1' LIMIT 1;
UPDATE `npc_types` SET `id`=683 WHERE `id`=679 && `name`='SwarmPetDG2' LIMIT 1;
UPDATE `npc_types` SET `id`=684 WHERE `id`=680 && `name`='SwarmPetDG3' LIMIT 1;