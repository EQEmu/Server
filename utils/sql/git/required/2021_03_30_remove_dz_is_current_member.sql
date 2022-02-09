-- remove any non-current members for new behavior
DELETE FROM `dynamic_zone_members`
WHERE is_current_member = 0;

ALTER TABLE `dynamic_zone_members`
  DROP COLUMN `is_current_member`;
