SELECT
--	base <begin>
	CASE
		WHEN `targettype` = '3' AND `CastRestriction` = '0' THEN 'GroupV1'
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
	`mana` mana_cost,
--	base <end>
--	extra <begin>
	`effect_base_value2` base_speed
--	extra <end>

FROM (
	SELECT 'RANGER' caster_class, `classes4` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value2`
	FROM `spells_new`
	WHERE `classes4` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('7', '10')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `CastRestriction` = '0'
		AND `effectdescnum` = '65'
UNION ALL
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value2`
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('7', '10')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `CastRestriction` = '0'
		AND `effectdescnum` = '65'
UNION ALL
	SELECT 'SHAMAN' caster_class, `classes10` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value2`
	FROM `spells_new`
	WHERE `classes10` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('7', '10')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `CastRestriction` = '0'
		AND `effectdescnum` = '65'
UNION ALL
	SELECT 'BEASTLORD' caster_class, `classes15` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effect_base_value2`
	FROM `spells_new`
	WHERE `classes15` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('7', '10')
--		6-self
		AND `targettype` NOT IN ('6')
		AND `CastRestriction` = '0'
		AND `effectdescnum` = '65'
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(target_type, 'Single', 'GroupV1', 'GroupV2'),
	effect_base_value2 DESC,
	spell_level,
	FIELD(caster_class, 'RANGER', 'DRUID', 'SHAMAN', 'BEASTLORD')
