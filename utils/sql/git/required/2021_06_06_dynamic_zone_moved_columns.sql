ALTER TABLE `dynamic_zones`
	ADD COLUMN `uuid` VARCHAR(36) NOT NULL COLLATE 'latin1_swedish_ci' AFTER `type`,
	ADD COLUMN `name` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'latin1_swedish_ci' AFTER `uuid`,
	ADD COLUMN `leader_id` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `name`,
	ADD COLUMN `min_players` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `leader_id`,
	ADD COLUMN `max_players` INT UNSIGNED NOT NULL DEFAULT '0' AFTER `min_players`;

-- migrate any currently active expeditions
UPDATE dynamic_zones
INNER JOIN expeditions ON expeditions.dynamic_zone_id = dynamic_zones.id
SET
  dynamic_zones.uuid = expeditions.uuid,
  dynamic_zones.name = expeditions.expedition_name,
  dynamic_zones.leader_id = expeditions.leader_id,
  dynamic_zones.min_players = expeditions.min_players,
  dynamic_zones.max_players = expeditions.max_players;

ALTER TABLE `expeditions`
  DROP COLUMN `uuid`,
  DROP COLUMN `expedition_name`,
  DROP COLUMN `leader_id`,
  DROP COLUMN `min_players`,
  DROP COLUMN `max_players`;
