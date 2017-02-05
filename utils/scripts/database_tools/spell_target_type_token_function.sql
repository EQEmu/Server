DELIMITER $$

DROP FUNCTION IF EXISTS `GetSpellTargetTypeToken`;

-- This function converts a numeric spell target type to a string label based on server code designations
-- Note: `cast_restriction` look-up is not implemented at this time
--
-- example:
-- SELECT `id`, `name`, GetSpellTargetTypeToken(`targettype`, `CastRestriction`) FROM `spells_new` WHERE `id` IN ('6836', '10763', '30057');
CREATE FUNCTION `GetSpellTargetTypeToken` (`target_type` INT(11), `cast_restriction` INT(11)) RETURNS VARCHAR(64)
BEGIN
	DECLARE `token` VARCHAR(64) DEFAULT '';
	
	CASE `target_type`
		WHEN '1' THEN SET `token` = 'ST_TargetOptional';
		WHEN '2' THEN SET `token` = 'ST_AEClientV1';
		WHEN '3' THEN SET `token` = 'ST_GroupTeleport';
		WHEN '4' THEN SET `token` = 'ST_AECaster';
		WHEN '5' THEN SET `token` = 'ST_Target';
		WHEN '6' THEN SET `token` = 'ST_Self';
		WHEN '8' THEN SET `token` = 'ST_AETarget';
		WHEN '9' THEN SET `token` = 'ST_Animal';
		WHEN '10' THEN SET `token` = 'ST_Undead';
		WHEN '11' THEN SET `token` = 'ST_Summoned';
		WHEN '13' THEN SET `token` = 'ST_Tap';
		WHEN '14' THEN SET `token` = 'ST_Pet';
		WHEN '15' THEN SET `token` = 'ST_Corpse';
		WHEN '16' THEN SET `token` = 'ST_Plant';
		WHEN '17' THEN SET `token` = 'ST_Giant';
		WHEN '18' THEN SET `token` = 'ST_Dragon';
		WHEN '20' THEN SET `token` = 'ST_TargetAETap';
		WHEN '24' THEN SET `token` = 'ST_UndeadAE';
		WHEN '25' THEN SET `token` = 'ST_SummonedAE';
		WHEN '32' THEN SET `token` = 'ST_AETargetHateList';
		WHEN '33' THEN SET `token` = 'ST_HateList';
		WHEN '34' THEN SET `token` = 'ST_LDoNChest_Cursed';
		WHEN '35' THEN SET `token` = 'ST_Muramite';
		WHEN '36' THEN SET `token` = 'ST_AreaClientOnly';
		WHEN '37' THEN SET `token` = 'ST_AreaNPCOnly';
		WHEN '38' THEN SET `token` = 'ST_SummonedPet';
		WHEN '39' THEN SET `token` = 'ST_GroupNoPets';
		WHEN '40' THEN SET `token` = 'ST_AEBard';
		WHEN '41' THEN SET `token` = 'ST_Group';
		WHEN '42' THEN SET `token` = 'ST_Directional';
		WHEN '43' THEN SET `token` = 'ST_GroupClientAndPet';
		WHEN '44' THEN SET `token` = 'ST_Beam';
		WHEN '45' THEN SET `token` = 'ST_Ring';
		WHEN '46' THEN SET `token` = 'ST_TargetsTarget';
		WHEN '47' THEN SET `token` = 'ST_PetMaster';
		WHEN '50' THEN SET `token` = 'ST_TargetAENoPlayersPets';
		ELSE SET `token` = 'unk';
	END CASE;
	
	SET `token` = CONCAT(`token`, '(', `target_type`, ')', '->(', `cast_restriction`, ')');
	
	RETURN `token`;
END$$

DELIMITER ;
