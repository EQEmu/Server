DROP VIEW IF EXISTS `vw_bot_character_mobs`;

-- Views
CREATE VIEW `vw_bot_character_mobs` AS
SELECT
_utf8'C' AS mob_type,
c.`id`,
c.`name`,
c.`class`,
c.`level`,
c.`last_login`,
c.`zone_id`,
c.`deleted_at`
FROM `character_data` AS c
UNION ALL
SELECT _utf8'B' AS mob_type,
b.`bot_id` AS id,
b.`name`,
b.`class`,
b.`level`,
b.`last_spawn` AS last_login,
b.`zone_id`,
NULL AS `deleted_at`
FROM `bot_data` AS b;
