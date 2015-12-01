-- needs criteria refinement

SELECT
--	base <begin>
	CASE
		WHEN `targettype` = '4' AND `CastRestriction` = '0' THEN 'AECaster'
		WHEN `targettype` = '5' AND `CastRestriction` = '0' THEN 'Single'
		WHEN `targettype` = '8' AND `CastRestriction` = '0' THEN 'AETarget'
		WHEN `targettype` = '10' AND `CastRestriction` = '0' THEN 'Undead'
		WHEN `targettype` = '11' AND `CastRestriction` = '0' THEN 'Summoned'
		ELSE CONCAT(`targettype`, ', ', `CastRestriction`) -- 'UNDEFINED'
	END target_type,
	CASE
		WHEN `zonetype` NOT IN ('-1', '0') THEN `zonetype`
		ELSE '0'
	END zone_type,
	caster_class,
	spell_level,
	`id` spell_id,
	CONCAT('"', `name`, '"') spell_name,
	`mana` mana_cost,
--	base <end>
--	extra <begin>
	`ResistDiff` resist_diff,
	`max1` max_target_level
--	extra <end>

FROM (
	SELECT 'BARD' caster_class, `classes8` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `ResistDiff`, `max1`
	FROM `spells_new`
	WHERE `classes8` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('12', '13', '25', '27', '41', '43')
--		45-ring
		AND `targettype` NOT IN ('45')
		AND `effectid1` = '31'
UNION ALL
	SELECT 'NECROMANCER' caster_class, `classes11` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `ResistDiff`, `max1`
	FROM `spells_new`
	WHERE `classes11` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('12', '13', '25', '27', '41', '43')
--		45-ring
		AND `targettype` NOT IN ('45')
		AND `effectid1` = '31'
UNION ALL
	SELECT 'MAGICIAN' caster_class, `classes13` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `ResistDiff`, `max1`
	FROM `spells_new`
	WHERE `classes13` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('12', '13', '25', '27', '41', '43')
--		45-ring
		AND `targettype` NOT IN ('45')
		AND `effectid1` = '31'
UNION ALL
	SELECT 'ENCHANTER' caster_class, `classes14` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `ResistDiff`, `max1`
	FROM `spells_new`
	WHERE `classes14` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('12', '13', '25', '27', '41', '43')
--		45-ring
		AND `targettype` NOT IN ('45')
		AND `effectid1` = '31'
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY resist_diff,
	FIELD(target_type, 'Undead', 'Summoned', 'Single', 'AECaster', 'AETarget'),
	max_target_level DESC,
	spell_level DESC,
	FIELD(caster_class, 'BARD', 'NECROMANCER', 'MAGICIAN', 'ENCHANTER')
