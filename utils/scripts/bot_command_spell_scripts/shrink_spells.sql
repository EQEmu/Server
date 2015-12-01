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
	SELECT 'SHAMAN' caster_class, `classes10` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes10` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '10'
--		6-self
		AND `targettype` NOT IN ('6')
		AND `effectid1` = '89' -- implementation restricted to `effectid1`
		AND `effect_base_value1` < 100 -- implementation restricted to `effect_base_value1`
UNION ALL
	SELECT 'BEASTLORD' caster_class, `classes15` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`
	FROM `spells_new`
	WHERE `classes15` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '10'
--		6-self
		AND `targettype` NOT IN ('6')
		AND `effectid1` = '89' -- implementation restricted to `effectid1`
		AND `effect_base_value1` < 100 -- implementation restricted to `effect_base_value1`
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(target_type, 'Single'),
	FIELD(caster_class, 'SHAMAN', 'BEASTLORD'),
	spell_level,
	spell_name
