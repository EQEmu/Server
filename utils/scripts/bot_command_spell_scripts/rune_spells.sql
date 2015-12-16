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
	`max1` max_absorbtion
--	extra <end>

FROM (
	SELECT 'ENCHANTER' caster_class, `classes14` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `max1`
	FROM `spells_new`
	WHERE `classes14` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '2'
		AND `effectid1` = '55'
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(target_type, 'Single', 'GroupV1', 'GroupV2'),
	spell_level DESC,
	max1 DESC
