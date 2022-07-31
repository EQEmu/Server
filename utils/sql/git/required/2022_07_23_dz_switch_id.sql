ALTER TABLE `doors`
  ADD COLUMN `dz_switch_id` INT NOT NULL DEFAULT '0' AFTER `is_ldon_door`;

ALTER TABLE `dynamic_zones`
  ADD COLUMN `dz_switch_id` INT NOT NULL DEFAULT '0' AFTER `max_players`;
