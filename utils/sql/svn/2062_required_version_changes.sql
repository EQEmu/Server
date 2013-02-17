ALTER TABLE `doors`
MODIFY COLUMN `version`  smallint(5) NOT NULL DEFAULT 0 AFTER `zone`;
ALTER TABLE `object`
MODIFY COLUMN `version`  smallint(5) NOT NULL DEFAULT 0 AFTER `zoneid`;
ALTER TABLE `ground_spawns`
MODIFY COLUMN `version`  smallint(5) NOT NULL DEFAULT 0 AFTER `zoneid`;

