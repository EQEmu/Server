INSERT INTO `perl_event_export_settings` (`event_id`, `event_description`, `export_qglobals`, `export_mob`, `export_zone`, `export_item`, `export_event`) VALUES (81, 'EVENT_SPAWN_ZONE', 0, 0, 0, 0, 1);
INSERT INTO `perl_event_export_settings` (`event_id`, `event_description`, `export_qglobals`, `export_mob`, `export_zone`, `export_item`, `export_event`) VALUES (82, 'EVENT_DEATH_ZONE', 0, 0, 0, 0, 1);
ALTER TABLE `rule_values`
MODIFY COLUMN `notes`  text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL AFTER `rule_value`;