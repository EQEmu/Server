DELIMITER $$

DROP FUNCTION IF EXISTS `GetSpellTargetTypeToken`;

-- This function converts a numeric spell target type to a string label based on server code designations
-- Note: `cast_restriction` data taken from MacroQuest2 definitions
--
-- example:
-- SELECT `id`, `name`, GetSpellTargetTypeToken(`targettype`, `CastRestriction`) FROM `spells_new` WHERE `id` IN ('6836', '10763', '25039');
CREATE FUNCTION `GetSpellTargetTypeToken` (`target_type` INT(11), `cast_restriction` INT(11)) RETURNS VARCHAR(128)
BEGIN
	DECLARE `token` VARCHAR(128) DEFAULT '';
	DECLARE `token2` VARCHAR(64) DEFAULT '';
	
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
	
	SET `token` = CONCAT(`token`, '(', `target_type`, ')');
	
	CASE `cast_restriction`
		WHEN '0' THEN SET `token2` = 'None';
		WHEN '100' THEN SET `token2` = 'Only works on Animal or Humanoid';
		WHEN '101' THEN SET `token2` = 'Only works on Dragon';
		WHEN '102' THEN SET `token2` = 'Only works on Animal or Insect';
		WHEN '104' THEN SET `token2` = 'Only works on Animal';
		WHEN '105' THEN SET `token2` = 'Only works on Plant';
		WHEN '106' THEN SET `token2` = 'Only works on Giant';
		WHEN '108' THEN SET `token2` = 'Doesn\'t work on Animals or Humanoids';
		WHEN '109' THEN SET `token2` = 'Only works on Bixie';
		WHEN '110' THEN SET `token2` = 'Only works on Harpy';
		WHEN '111' THEN SET `token2` = 'Only works on Gnoll';
		WHEN '112' THEN SET `token2` = 'Only works on Sporali';
		WHEN '113' THEN SET `token2` = 'Only works on Kobold';
		WHEN '114' THEN SET `token2` = 'Only works on Shade';
		WHEN '115' THEN SET `token2` = 'Only works on Drakkin';
		WHEN '117' THEN SET `token2` = 'Only works on Animals or Plants';
		WHEN '118' THEN SET `token2` = 'Only works on Summoned';
		WHEN '119' THEN SET `token2` = 'Only works on Fire_Pet';
		WHEN '120' THEN SET `token2` = 'Only works on Undead';
		WHEN '121' THEN SET `token2` = 'Only works on Living';
		WHEN '122' THEN SET `token2` = 'Only works on Fairy';
		WHEN '123' THEN SET `token2` = 'Only works on Humanoid';
		WHEN '124' THEN SET `token2` = 'Undead HP Less Than 10%';
		WHEN '125' THEN SET `token2` = 'Clockwork HP Less Than 45%';
		WHEN '126' THEN SET `token2` = 'Wisp HP Less Than 10%';
		WHEN '190' THEN SET `token2` = 'Doesn\'t work on Raid Bosses';
		WHEN '191' THEN SET `token2` = 'Only works on Raid Bosses';
		WHEN '201' THEN SET `token2` = 'HP Above 75%';
		WHEN '203' THEN SET `token2` = 'HP Less Than 20%';
		WHEN '204' THEN SET `token2` = 'HP Less Than 50%';
		WHEN '216' THEN SET `token2` = 'Not In Combat';
		WHEN '221' THEN SET `token2` = 'At Least 1 Pet On Hatelist';
		WHEN '222' THEN SET `token2` = 'At Least 2 Pets On Hatelist';
		WHEN '223' THEN SET `token2` = 'At Least 3 Pets On Hatelist';
		WHEN '224' THEN SET `token2` = 'At Least 4 Pets On Hatelist';
		WHEN '225' THEN SET `token2` = 'At Least 5 Pets On Hatelist';
		WHEN '226' THEN SET `token2` = 'At Least 6 Pets On Hatelist';
		WHEN '227' THEN SET `token2` = 'At Least 7 Pets On Hatelist';
		WHEN '228' THEN SET `token2` = 'At Least 8 Pets On Hatelist';
		WHEN '229' THEN SET `token2` = 'At Least 9 Pets On Hatelist';
		WHEN '230' THEN SET `token2` = 'At Least 10 Pets On Hatelist';
		WHEN '231' THEN SET `token2` = 'At Least 11 Pets On Hatelist';
		WHEN '232' THEN SET `token2` = 'At Least 12 Pets On Hatelist';
		WHEN '233' THEN SET `token2` = 'At Least 13 Pets On Hatelist';
		WHEN '234' THEN SET `token2` = 'At Least 14 Pets On Hatelist';
		WHEN '235' THEN SET `token2` = 'At Least 15 Pets On Hatelist';
		WHEN '236' THEN SET `token2` = 'At Least 16 Pets On Hatelist';
		WHEN '237' THEN SET `token2` = 'At Least 17 Pets On Hatelist';
		WHEN '238' THEN SET `token2` = 'At Least 18 Pets On Hatelist';
		WHEN '239' THEN SET `token2` = 'At Least 19 Pets On Hatelist';
		WHEN '240' THEN SET `token2` = 'At Least 20 Pets On Hatelist';
		WHEN '250' THEN SET `token2` = 'HP Less Than 35%';
		WHEN '304' THEN SET `token2` = 'Chain Plate Classes';
		WHEN '399' THEN SET `token2` = 'HP Between 15 and 25%';
		WHEN '400' THEN SET `token2` = 'HP Between 1 and 25%';
		WHEN '401' THEN SET `token2` = 'HP Between 25 and 35%';
		WHEN '402' THEN SET `token2` = 'HP Between 35 and 45%';
		WHEN '403' THEN SET `token2` = 'HP Between 45 and 55%';
		WHEN '404' THEN SET `token2` = 'HP Between 55 and 65%';
		WHEN '412' THEN SET `token2` = 'HP Above 99%';
		WHEN '501' THEN SET `token2` = 'HP Below 5%';
		WHEN '502' THEN SET `token2` = 'HP Below 10%';
		WHEN '503' THEN SET `token2` = 'HP Below 15%';
		WHEN '504' THEN SET `token2` = 'HP Below 20%';
		WHEN '505' THEN SET `token2` = 'HP Below 25%';
		WHEN '506' THEN SET `token2` = 'HP Below 30%';
		WHEN '507' THEN SET `token2` = 'HP Below 35%';
		WHEN '508' THEN SET `token2` = 'HP Below 40%';
		WHEN '509' THEN SET `token2` = 'HP Below 45%';
		WHEN '510' THEN SET `token2` = 'HP Below 50%';
		WHEN '511' THEN SET `token2` = 'HP Below 55%';
		WHEN '512' THEN SET `token2` = 'HP Below 60%';
		WHEN '513' THEN SET `token2` = 'HP Below 65%';
		WHEN '514' THEN SET `token2` = 'HP Below 70%';
		WHEN '515' THEN SET `token2` = 'HP Below 75%';
		WHEN '516' THEN SET `token2` = 'HP Below 80%';
		WHEN '517' THEN SET `token2` = 'HP Below 85%';
		WHEN '518' THEN SET `token2` = 'HP Below 90%';
		WHEN '519' THEN SET `token2` = 'HP Below 95%';
		WHEN '521' THEN SET `token2` = 'Mana Below X%';
		WHEN '522' THEN SET `token2` = 'End Below 40%';
		WHEN '523' THEN SET `token2` = 'Mana Below 40%';
		WHEN '603' THEN SET `token2` = 'Only works on Undead2';
		WHEN '608' THEN SET `token2` = 'Only works on Undead3';
		WHEN '624' THEN SET `token2` = 'Only works on Summoned2';
		WHEN '701' THEN SET `token2` = 'Doesn\'t work on Pets';
		WHEN '818' THEN SET `token2` = 'Only works on Undead4';
		WHEN '819' THEN SET `token2` = 'Doesn\'t work on Undead4';
		WHEN '825' THEN SET `token2` = 'End Below 21%';
		WHEN '826' THEN SET `token2` = 'End Below 25%';
		WHEN '827' THEN SET `token2` = 'End Below 29%';
		WHEN '836' THEN SET `token2` = 'Only works on Regular Servers';
		WHEN '837' THEN SET `token2` = 'Doesn\'t work on Progression Servers';
		WHEN '842' THEN SET `token2` = 'Only works on Humanoid Level 84 Max';
		WHEN '843' THEN SET `token2` = 'Only works on Humanoid Level 86 Max';
		WHEN '844' THEN SET `token2` = 'Only works on Humanoid Level 88 Max';
		WHEN '1000' THEN SET `token2` = 'Between Level 1 and 75';
		WHEN '1001' THEN SET `token2` = 'Between Level 76 and 85';
		WHEN '1002' THEN SET `token2` = 'Between Level 86 and 95';
		WHEN '1003' THEN SET `token2` = 'Between Level 96 and 100';
		WHEN '1004' THEN SET `token2` = 'HP Less Than 80%';
		WHEN '38311' THEN SET `token2` = 'Mana Below 20%';
		WHEN '38312' THEN SET `token2` = 'Mana Below 10%';
		ELSE SET `token2` = 'unk';
	END CASE;
	
	SET `token2` = CONCAT(`token2`, '(', `cast_restriction`, ')');
	
	SET `token` = CONCAT(`token`, ':', `token2`);
	
	RETURN `token`;
END$$

DELIMITER ;
