SELECT
--	base <begin>
	CASE
		WHEN `targettype` = '5' AND `CastRestriction` = '0' THEN 'Single'
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
		AND `SpellAffectIndex` = '14'
		AND '25' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '14'
		AND '25' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'SHAMAN' caster_class, `classes10` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes10` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '14'
		AND '25' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'NECROMANCER' caster_class, `classes11` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes11` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '14'
		AND '25' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'WIZARD' caster_class, `classes12` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes12` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '14'
		AND '25' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'MAGICIAN' caster_class, `classes13` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes13` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '14'
		AND '25' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
UNION ALL
	SELECT 'ENCHANTER' caster_class, `classes14` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes14` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '14'
		AND '25' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY mana_cost DESC,
	FIELD(target_type, 'Single'),
	spell_level,
	FIELD(caster_class, 'CLERIC', 'DRUID', 'SHAMAN', 'NECROMANCER', 'WIZARD', 'MAGICIAN', 'ENCHANTER')
