/* Drop the current Merc View */
DROP VIEW vwMercNpcTypes;

/* Rename fields to match the source changes */
ALTER TABLE `merc_weaponinfo` CHANGE `d_meele_texture1` `d_melee_texture1` INT(11) NOT NULL DEFAULT 0;
ALTER TABLE `merc_weaponinfo` CHANGE `d_meele_texture2` `d_melee_texture2` INT(11) NOT NULL DEFAULT 0;

/* Re-Create the Merc View with new field names */
CREATE VIEW vwMercNpcTypes AS
SELECT
	ms.merc_npc_type_id,
	'' AS name,
	ms.clientlevel,
	ms.level,
	mtyp.race_id,
	mstyp.class_id,
	ms.hp,
	ms.mana,
	0 AS gender,
	mai.texture,
	mai.helmtexture,
	ms.attack_speed,
	ms.STR,
	ms.STA,
	ms.DEX,
	ms.AGI,
	ms._INT,
	ms.WIS,
	ms.CHA,
	ms.MR,
	ms.CR,
	ms.DR,
	ms.FR,
	ms.PR,
	ms.Corrup,
	ms.mindmg,
	ms.maxdmg,
	ms.attack_count,
	ms.special_abilities AS special_abilities,
	mwi.d_melee_texture1,
	mwi.d_melee_texture2,
	mwi.prim_melee_type,
	mwi.sec_melee_type,
	ms.runspeed,
	ms.hp_regen_rate,
	ms.mana_regen_rate,
	1 AS bodytype,
	mai.armortint_id,
	mai.armortint_red,
	mai.armortint_green,
	mai.armortint_blue,
	ms.AC,
	ms.ATK,
	ms.Accuracy,
	ms.spellscale,
	ms.healscale
FROM merc_stats ms
INNER JOIN merc_armorinfo mai
ON ms.merc_npc_type_id = mai.merc_npc_type_id
AND mai.minlevel <= ms.level AND mai.maxlevel >= ms.level
INNER JOIN merc_weaponinfo mwi
ON ms.merc_npc_type_id = mwi.merc_npc_type_id
AND mwi.minlevel <= ms.level AND mwi.maxlevel >= ms.level
INNER JOIN merc_templates mtem
ON mtem.merc_npc_type_id = ms.merc_npc_type_id
INNER JOIN merc_types mtyp
ON mtem.merc_type_id = mtyp.merc_type_id
INNER JOIN merc_subtypes mstyp
ON mtem.merc_subtype_id = mstyp.merc_subtype_id;