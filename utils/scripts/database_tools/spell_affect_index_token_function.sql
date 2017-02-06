DELIMITER $$

DROP FUNCTION IF EXISTS `GetSpellAffectIndexToken`;

-- This function converts a numeric spell affect index id to a string label based on server code designations
--
-- example:
-- SELECT `id`, `name`, GetSpellAffectIndexToken(`SpellAffectIndex`) FROM `spells_new` WHERE `id` IN ('73', '2253', '2319');
CREATE FUNCTION `GetSpellAffectIndexToken` (`affect_index_id` INT(11)) RETURNS VARCHAR(64)
BEGIN
	DECLARE `token` VARCHAR(64) DEFAULT '';
	
	CASE `affect_index_id`
		WHEN '-1' THEN SET `token` = 'SAI_Summon_Mount_Unclass';
		WHEN '0' THEN SET `token` = 'SAI_Direct_Damage';
		WHEN '1' THEN SET `token` = 'SAI_Heal_Cure';
		WHEN '2' THEN SET `token` = 'SAI_AC_Buff';
		WHEN '3' THEN SET `token` = 'SAI_AE_Damage';
		WHEN '4' THEN SET `token` = 'SAI_Summon';
		WHEN '5' THEN SET `token` = 'SAI_Sight';
		WHEN '6' THEN SET `token` = 'SAI_Mana_Regen_Resist_Song';
		WHEN '7' THEN SET `token` = 'SAI_Stat_Buff';
		WHEN '9' THEN SET `token` = 'SAI_Vanish';
		WHEN '10' THEN SET `token` = 'SAI_Appearance';
		WHEN '11' THEN SET `token` = 'SAI_Enchanter_Pet';
		WHEN '12' THEN SET `token` = 'SAI_Calm';
		WHEN '13' THEN SET `token` = 'SAI_Fear';
		WHEN '14' THEN SET `token` = 'SAI_Dispell_Sight';
		WHEN '15' THEN SET `token` = 'SAI_Stun';
		WHEN '16' THEN SET `token` = 'SAI_Haste_Runspeed';
		WHEN '17' THEN SET `token` = 'SAI_Combat_Slow';
		WHEN '18' THEN SET `token` = 'SAI_Damage_Shield';
		WHEN '19' THEN SET `token` = 'SAI_Cannibalize_Weapon_Proc';
		WHEN '20' THEN SET `token` = 'SAI_Weaken';
		WHEN '21' THEN SET `token` = 'SAI_Banish';
		WHEN '22' THEN SET `token` = 'SAI_Blind_Poison';
		WHEN '23' THEN SET `token` = 'SAI_Cold_DD';
		WHEN '24' THEN SET `token` = 'SAI_Poison_Disease_DD';
		WHEN '25' THEN SET `token` = 'SAI_Fire_DD';
		WHEN '27' THEN SET `token` = 'SAI_Memory_Blur';
		WHEN '28' THEN SET `token` = 'SAI_Gravity_Fling';
		WHEN '29' THEN SET `token` = 'SAI_Suffocate';
		WHEN '30' THEN SET `token` = 'SAI_Lifetap_Over_Time';
		WHEN '31' THEN SET `token` = 'SAI_Fire_AE';
		WHEN '33' THEN SET `token` = 'SAI_Cold_AE';
		WHEN '34' THEN SET `token` = 'SAI_Poison_Disease_AE';
		WHEN '40' THEN SET `token` = 'SAI_Teleport';
		WHEN '41' THEN SET `token` = 'SAI_Direct_Damage_Song';
		WHEN '42' THEN SET `token` = 'SAI_Combat_Buff_Song';
		WHEN '43' THEN SET `token` = 'SAI_Calm_Song';
		WHEN '45' THEN SET `token` = 'SAI_Firework';
		WHEN '46' THEN SET `token` = 'SAI_Firework_AE';
		WHEN '47' THEN SET `token` = 'SAI_Weather_Rocket';
		WHEN '50' THEN SET `token` = 'SAI_Convert_Vitals';
		WHEN '60' THEN SET `token` = 'SAI_NPC_Special_60';
		WHEN '61' THEN SET `token` = 'SAI_NPC_Special_61';
		WHEN '62' THEN SET `token` = 'SAI_NPC_Special_62';
		WHEN '63' THEN SET `token` = 'SAI_NPC_Special_63';
		WHEN '70' THEN SET `token` = 'SAI_NPC_Special_70';
		WHEN '71' THEN SET `token` = 'SAI_NPC_Special_71';
		WHEN '80' THEN SET `token` = 'SAI_NPC_Special_80';
		WHEN '88' THEN SET `token` = 'SAI_Lock_Trap';
		ELSE SET `token` = 'unk';
	END CASE;
	
	SET `token` = CONCAT(`token`, '(', `affect_index_id`, ')');
	
	RETURN `token`;
END$$

DELIMITER ;
