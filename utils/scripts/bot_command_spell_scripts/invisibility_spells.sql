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
	`mana` mana_cost,
--	base <end>
--	extra <begin>
	CASE
		WHEN `effectid1` = '12' THEN 'Living'
		WHEN `effectid1` = '13' THEN 'See'
		WHEN `effectid1` = '28' THEN 'Undead'
		WHEN `effectid1` = '29' THEN 'Animal'
		ELSE `effectid1` -- 'UNDEFINED'
	END invis_type
--	extra <end>

FROM (
	SELECT 'CLERIC' caster_class, `classes2` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes2` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'PALADIN' caster_class, `classes3` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes3` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'RANGER' caster_class, `classes4` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes4` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'SHADOWKNIGHT' caster_class, `classes5` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes5` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'DRUID' caster_class, `classes6` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes6` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'SHAMAN' caster_class, `classes10` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes10` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'NECROMANCER' caster_class, `classes11` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes11` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'WIZARD' caster_class, `classes12` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes12` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'MAGICIAN' caster_class, `classes13` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes13` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'ENCHANTER' caster_class, `classes14` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes14` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
UNION ALL
	SELECT 'BEASTLORD' caster_class, `classes15` spell_level,
		`targettype`, `CastRestriction`, `zonetype`, `id`, `name`, `mana`, `effectid1`
	FROM `spells_new`
	WHERE `classes15` NOT IN ('254', '255')
		AND `SpellAffectIndex` IN ('5', '9')
--		6-self
		AND `targettype` NOT IN ('6')
--		12-living, 13-see, 28-undead, 29-animal
		AND `effectid1` IN ('12', '13', '28', '29') -- implementation restricted to `effectid1`
) spells

--	WHERE `name` NOT LIKE '%II'
--	---
--	WHERE `name` NOT LIKE '%Rk. II%'
--	AND `name` NOT LIKE '%Rk.II%'
--	AND `name` NOT LIKE '%Rk. III%'
--	AND `name` NOT LIKE '%Rk.III%'
ORDER BY FIELD(invis_type, 'Animal', 'Undead', 'Living', 'See'),
	FIELD(target_type, 'Single', 'GroupV2'),
	zone_type,
	spell_level DESC,
	spell_name,
	FIELD(caster_class, 'CLERIC', 'PALADIN', 'RANGER', 'SHADOWKNIGHT', 'DRUID', 'SHAMAN', 'NECROMANCER', 'WIZARD', 'MAGICIAN', 'ENCHANTER', 'BEASTLORD')
