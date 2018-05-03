ALTER TABLE `bot_data` ADD COLUMN `stop_melee_level` TINYINT(3) UNSIGNED NOT NULL DEFAULT '255' AFTER `follow_distance`;

INSERT INTO `bot_command_settings`(`bot_command`, `access`, `aliases`) VALUES ('botstopmeleelevel', '0', 'sml');

SELECT @csml_raw := (SELECT `rule_value` FROM `rule_values` WHERE `rule_name` LIKE 'Bots:CasterStopMeleeLevel' LIMIT 1);
SELECT @csml_value := IF((SELECT @csml_raw REGEXP '^[0-9]+$') = '1', @csml_raw, '13');

UPDATE `bot_data` SET `stop_melee_level` = @csml_value WHERE `class` IN ('2', '6', '10', '11', '12', '13', '14');
