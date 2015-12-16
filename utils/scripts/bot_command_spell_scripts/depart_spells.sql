SELECT
--	base <begin>
	CASE
		WHEN `targettype` = '3' AND `CastRestriction` = '0' THEN 'GroupV1'
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
	CONCAT('"', `teleport_zone`, '"') short_name,
	CONCAT('"', IFNULL((SELECT `long_name` FROM `zone` WHERE `short_name` = `teleport_zone` LIMIT 1), 'Unreachable Destination'), '"') long_name
--	extra <end>

FROM (
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `teleport_zone`
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '40'
		AND (
			'83' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '86' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR (
				'88' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
					AND `teleport_zone` NOT LIKE 'same'
			)
		)
UNION ALL
	SELECT 'WIZARD' caster_class, `classes12` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `teleport_zone`
	FROM `spells_new`
	WHERE `classes12` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '40'
		AND (
			'83' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '86' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR (
				'88' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
					AND `teleport_zone` NOT LIKE 'same'
			)
		)
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(target_type, 'GroupV1'),
	FIELD(caster_class, 'DRUID', 'WIZARD'),
	spell_level,
	spell_name
