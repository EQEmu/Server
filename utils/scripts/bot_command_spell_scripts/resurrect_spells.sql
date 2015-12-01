SELECT
--	base <begin>
	CASE
		WHEN `targettype` = '4' AND `CastRestriction` = '0' THEN 'AECaster'
		WHEN `targettype` = '15' AND `CastRestriction` = '0' THEN 'Corpse'
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
	`effect_base_value1` percent
--	extra <end>

FROM (
	SELECT 'CLERIC' caster_class, `classes2` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value1`
	FROM `spells_new`
	WHERE `classes2` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('1', '13')
		AND `effectid1` = '81'
UNION ALL
	SELECT 'PALADIN' caster_class, `classes3` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value1`
	FROM `spells_new`
	WHERE `classes3` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('1', '13')
		AND `effectid1` = '81'
UNION ALL
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value1`
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('1', '13')
		AND `effectid1` = '81'
UNION ALL
	SELECT 'SHAMAN' caster_class, `classes10` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value1`
	FROM `spells_new`
	WHERE `classes10` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('1', '13')
		AND `effectid1` = '81'
UNION ALL
	SELECT 'NECROMANCER' caster_class, `classes11` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value1`
	FROM `spells_new`
	WHERE `classes11` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('1', '13')
		AND `effectid1` = '81'
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY percent DESC,
	FIELD(target_type, 'Corpse', 'AETarget'),
	zone_type,
	FIELD(caster_class, 'CLERIC', 'PALADIN', 'DRUID', 'SHAMAN', 'NECROMANCER'),
	spell_level,
	spell_id,
	spell_name
