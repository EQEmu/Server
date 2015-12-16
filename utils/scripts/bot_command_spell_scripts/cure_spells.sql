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
	(
		(IF(('20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)), '1', '0')) |
		(IF(('35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)), '2', '0')) |
		(IF(('36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)), '4', '0')) |
		(IF(('116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)), '8', '0')) |
		(IF(('369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)), '16', '0'))
	) cure_mask
--	extra <end>

FROM (
	SELECT 'CLERIC' caster_class, `classes2` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes2` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
UNION ALL
	SELECT 'PALADIN' caster_class, `classes3` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes3` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
UNION ALL
	SELECT 'RANGER' caster_class, `classes4` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes4` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
UNION ALL
	SELECT 'SHADOWKNIGHT' caster_class, `classes5` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes5` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
UNION ALL
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
UNION ALL
	SELECT 'BARD' caster_class, `classes8` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes8` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
UNION ALL
	SELECT 'SHAMAN' caster_class, `classes10` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes10` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
UNION ALL
	SELECT 'NECROMANCER' caster_class, `classes11` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes11` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
UNION ALL
	SELECT 'BEASTLORD' caster_class, `classes15` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`,
		`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`
	FROM `spells_new`
	WHERE `classes15` NOT IN ('254', '255')
		AND `SpellAffectIndex` = '1'
--		6-self, 10-undead, 14-pet, 45-ring
		AND `targettype` NOT IN ('6', '10', '14', '45')
		AND (
--			20-blindness, 35-disease, 36-poison, 116-curse, 369-corruption
			'20' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '35' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '36' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '116' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
			OR '369' IN (`effectid1`, `effectid2`, `effectid3`, `effectid4`, `effectid5`, `effectid6`, `effectid7`, `effectid8`, `effectid9`, `effectid10`, `effectid11`, `effectid12`)
		)
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(target_type, 'Single', 'GroupV1', 'GroupV2'),
	spell_level DESC,
	cure_mask DESC,
	FIELD(caster_class, 'CLERIC', 'PALADIN', 'RANGER', 'SHADOWKNIGHT', 'DRUID', 'BARD', 'SHAMAN', 'NECROMANCER', 'BEASTLORD')
