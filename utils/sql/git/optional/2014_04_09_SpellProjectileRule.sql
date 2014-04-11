-- Recommend enabling if your server uses an UF+ spell file and your players use UF+ client. This will give the proper graphics for all spell projectiles.
INSERT INTO `rule_values` (`ruleset_id`, `rule_name`, `rule_value`, `notes`) VALUES (1, 'Spells:UseLiveSpellProjectileGFX', false, ' Use spell projectile graphics set in the spells_new table (player_1). Server must be using UF+ spell file.');

-- Use this query to check if your spell file is compatible
-- If it returns in the player_1 field IT##### it will work.
SELECT id,name,player_1 from spells_new WHERE targettype = 1;

