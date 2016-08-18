SELECT
--	base <begin>
	CASE
		WHEN `targettype` = '1' AND `CastRestriction` = '0' THEN 'TargetOptional'
		WHEN `targettype` = '3' AND `CastRestriction` = '0' THEN 'GroupV1'
		WHEN `targettype` = '4' AND `CastRestriction` = '0' THEN 'AECaster'
		WHEN `targettype` = '5' AND `CastRestriction` = '0' THEN 'Single'
		WHEN `targettype` = '5' AND `CastRestriction` = '150' THEN 'Animal'
		WHEN `targettype` = '6' AND `CastRestriction` = '0' THEN 'Self'
		WHEN `targettype` = '8' AND `CastRestriction` = '0' THEN 'AETarget'
		WHEN `targettype` = '9' AND `CastRestriction` = '0' THEN 'Animal'
		WHEN `targettype` = '10' AND `CastRestriction` = '0' THEN 'Undead'
		WHEN `targettype` = '11' AND `CastRestriction` = '0' THEN 'Summoned'
		WHEN `targettype` = '13' AND `CastRestriction` = '0' THEN 'Tap'
		WHEN `targettype` = '14' AND `CastRestriction` = '0' THEN 'Pet'
		WHEN `targettype` = '15' AND `CastRestriction` = '0' THEN 'Corpse'
		WHEN `targettype` = '16' AND `CastRestriction` = '0' THEN 'Plant'
		WHEN `targettype` = '17' AND `CastRestriction` = '0' THEN 'Giant'
		WHEN `targettype` = '18' AND `CastRestriction` = '0' THEN 'Dragon'
		WHEN `targettype` = '34' AND `CastRestriction` = '0' THEN 'LDoNChest_Cursed'
		WHEN `targettype` = '38' AND `CastRestriction` = '0' THEN 'SummonedPet'
		WHEN `targettype` = '39' AND `CastRestriction` = '0' THEN 'GroupNoPets' -- V1 or V2?
		WHEN `targettype` = '40' AND `CastRestriction` = '0' THEN 'AEBard'
		WHEN `targettype` = '41' AND `CastRestriction` = '0' THEN 'GroupV2'
		WHEN `targettype` = '42' AND `CastRestriction` = '0' THEN 'Directional'
		WHEN `targettype` = '43' AND `CastRestriction` = '0' THEN 'GroupClientAndPet'
		WHEN `targettype` = '44' AND `CastRestriction` = '0' THEN 'Beam'
		WHEN `targettype` = '45' AND `CastRestriction` = '0' THEN 'Ring'
		WHEN `targettype` = '46' AND `CastRestriction` = '0' THEN 'TargetsTarget'
		ELSE CONCAT(`targettype`, ', ', `CastRestriction`) -- 'UNDEFINED'
	END target_type,
	CASE
		WHEN `zonetype` NOT IN ('-1', '0') THEN `zonetype`
		ELSE '0'
	END zone_type,
	caster_class,
	spell_level,
	`id` spell_id,
	CONCAT('"', `name`, '"') spell_name
--	base <end>

FROM (
	SELECT 'WARRIOR' caster_class, `classes1` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes1` NOT IN ('254', '255')
UNION ALL
	SELECT 'CLERIC' caster_class, `classes2` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes2` NOT IN ('254', '255')
UNION ALL
	SELECT 'PALADIN' caster_class, `classes3` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes3` NOT IN ('254', '255')
UNION ALL
	SELECT 'RANGER' caster_class, `classes4` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes4` NOT IN ('254', '255')
UNION ALL
	SELECT 'SHADOWKNIGHT' caster_class, `classes5` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes5` NOT IN ('254', '255')
UNION ALL
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
UNION ALL
	SELECT 'MONK' caster_class, `classes7` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes7` NOT IN ('254', '255')
UNION ALL
	SELECT 'BARD' caster_class, `classes8` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes8` NOT IN ('254', '255')
UNION ALL
	SELECT 'ROGUE' caster_class, `classes9` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes9` NOT IN ('254', '255')
UNION ALL
	SELECT 'SHAMAN' caster_class, `classes10` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes10` NOT IN ('254', '255')
UNION ALL
	SELECT 'NECROMANCER' caster_class, `classes11` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes11` NOT IN ('254', '255')
UNION ALL
	SELECT 'WIZARD' caster_class, `classes12` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes12` NOT IN ('254', '255')
UNION ALL
	SELECT 'MAGICIAN' caster_class, `classes13` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes13` NOT IN ('254', '255')
UNION ALL
	SELECT 'ENCHANTER' caster_class, `classes14` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes14` NOT IN ('254', '255')
UNION ALL
	SELECT 'BEASTLORD' caster_class, `classes15` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes15` NOT IN ('254', '255')
UNION ALL
	SELECT 'BERSERKER' caster_class, `classes16` spell_level,
		`spells_new`.*
	FROM `spells_new`
	WHERE `classes16` NOT IN ('254', '255')
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(target_type, 'Animal', 'Undead', 'Summoned', 'Pet', 'Plant', 'TargetsTarget', 'Single', 'Self', 'GroupV1', 'GroupV2', 'GroupNoPets', 'AECaster', 'AETarget', 'Corpse'),
	zone_type,
	FIELD(caster_class, 'WARRIOR', 'CLERIC', 'PALADIN', 'RANGER', 'SHADOWKNIGHT', 'DRUID', 'MONK', 'BARD', 'ROGUE', 'SHAMAN', 'NECROMANCER', 'WIZARD', 'MAGICIAN', 'ENCHANTER', 'BEASTLORD', 'BERSERKER'),
	spell_level,
	spell_id,
	spell_name
