-- needs criteria refinement

SELECT
--	base <begin>
	CASE
		WHEN `targettype` = '5' AND `CastRestriction` = '0' THEN 'Single'
		WHEN `targettype` = '8' AND `CastRestriction` = '0' THEN 'AETarget'
		WHEN `targettype` = '9' AND `CastRestriction` = '0' THEN 'Animal'
		WHEN `targettype` = '10' AND `CastRestriction` = '0' THEN 'Undead'
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
	`mana` mana_cost
--	base <end>

FROM (
	SELECT 'CLERIC' caster_class, `classes2` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes2` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '12'
		AND '18' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'PALADIN' caster_class, `classes3` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes3` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '12'
		AND '18' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'RANGER' caster_class, `classes4` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes4` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '12'
		AND '18' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'SHADOWKNIGHT' caster_class, `classes5` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes5` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '12'
		AND '18' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '12'
		AND '18' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'NECROMANCER' caster_class, `classes11` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes11` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '12'
		AND '18' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'ENCHANTER' caster_class, `classes14` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes14` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '12'
		AND '18' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(target_type, 'Animal', 'Undead', 'Single', 'AETarget'),
	target_type,
	zone_type,
	FIELD(caster_class, 'CLERIC', 'PALADIN', 'RANGER', 'SHADOWKNIGHT', 'DRUID', 'NECROMANCER', 'ENCHANTER'),
	spell_level,
	spell_id,
	spell_name
