SELECT
--	base <begin>
	CASE
		WHEN `targettype` = '5' AND `CastRestriction` = '0' THEN 'Single'
		WHEN `targettype` = '41' AND `CastRestriction` = '0' THEN 'GroupV2'
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
	SELECT 'RANGER' caster_class, `classes4` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes4` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('2', '14')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `effectid1` = '14'
UNION ALL
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('2', '14')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `effectid1` = '14'
UNION ALL
	SELECT 'SHAMAN' caster_class, `classes10` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes10` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('2', '14')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `effectid1` = '14'
UNION ALL
	SELECT 'NECROMANCER' caster_class, `classes11` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes11` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('2', '14')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `effectid1` = '14'
UNION ALL
	SELECT 'ENCHANTER' caster_class, `classes14` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes14` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('2', '14')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `effectid1` = '14'
UNION ALL
	SELECT 'BEASTLORD' caster_class, `classes15` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes15` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('2', '14')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `effectid1` = '14'
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(target_type, 'Single', 'GroupV2'),
	spell_level,
	spell_name,
	FIELD(caster_class, 'RANGER', 'DRUID', 'SHAMAN', 'NECROMANCER', 'ENCHANTER', 'BEASTLORD')
