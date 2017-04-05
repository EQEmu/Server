DELIMITER $$

DROP FUNCTION IF EXISTS `GetBodyTypeToken`;

-- This function converts a numeric body type to a string label based on server code designations
-- Note: A preceeding '*' indicates a non-client verified token
--
-- example:
-- SELECT `id`, `name`, GetBodyTypeToken(`bodytype`) FROM `npc_types` WHERE `id` IN ('116539', '154086', '164042');
CREATE FUNCTION `GetBodyTypeToken` (`body_type` INT(11)) RETURNS VARCHAR(64)
BEGIN
	DECLARE `token` VARCHAR(64) DEFAULT '';
	
	CASE `body_type`
		WHEN '1' THEN SET `token` = 'BT_Humanoid';
		WHEN '2' THEN SET `token` = 'BT_Lycanthrope';
		WHEN '3' THEN SET `token` = 'BT_Undead';
		WHEN '4' THEN SET `token` = 'BT_Giant';
		WHEN '5' THEN SET `token` = 'BT_Construct';
		WHEN '6' THEN SET `token` = 'BT_Extraplanar';
		WHEN '7' THEN SET `token` = 'BT_Magical';
		WHEN '8' THEN SET `token` = '*BT_SummonedUndead';
		WHEN '9' THEN SET `token` = '*BT_RaidGiant';
		WHEN '11' THEN SET `token` = '*BT_NoTarget';
		WHEN '12' THEN SET `token` = 'BT_Vampyre';
		WHEN '13' THEN SET `token` = 'BT_Atenha_Ra';
		WHEN '14' THEN SET `token` = 'BT_Greater_Akheva';
		WHEN '15' THEN SET `token` = 'BT_Khati_Sha';
		WHEN '16' THEN SET `token` = '*BT_Seru';
		WHEN '18' THEN SET `token` = 'BT_Draz_Nurakk';
		WHEN '19' THEN SET `token` = 'BT_Zek';
		WHEN '20' THEN SET `token` = 'BT_Luggald';
		WHEN '21' THEN SET `token` = 'BT_Animal';
		WHEN '22' THEN SET `token` = 'BT_Insect';
		WHEN '23' THEN SET `token` = 'BT_Monster';
		WHEN '24' THEN SET `token` = 'BT_Elemental/*BT_Summoned';
		WHEN '25' THEN SET `token` = 'BT_Plant';
		WHEN '26' THEN SET `token` = 'BT_Dragon';
		WHEN '27' THEN SET `token` = '*BT_Summoned2';
		WHEN '28' THEN SET `token` = 'BT_Summoned_Creature/*BT_Summoned3';
		WHEN '30' THEN SET `token` = '*BT_VeliousDragon';
		WHEN '32' THEN SET `token` = '*BT_Dragon3';
		WHEN '33' THEN SET `token` = '*BT_Boxes';
		WHEN '34' THEN SET `token` = 'BT_Muramite';
		WHEN '60' THEN SET `token` = '*BT_NoTarget2';
		WHEN '63' THEN SET `token` = '*BT_SwarmPet';
		WHEN '66' THEN SET `token` = '*BT_InvisMan';
		WHEN '67' THEN SET `token` = '*BT_Special';
		ELSE SET `token` = 'BT_UNKNOWN_BODYTYPE';
	END CASE;
	
	SET `token` = CONCAT(`token`, '(', `body_type`, ')');
	
	RETURN `token`;
END$$

DELIMITER ;
