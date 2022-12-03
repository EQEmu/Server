ALTER TABLE `bot_data`
ADD COLUMN `enforce_spell_settings` TINYINT(2) UNSIGNED NOT NULL DEFAULT '0' AFTER `expansion_bitmask`;

ALTER TABLE `bot_spell_settings` DROP `min_level`;
ALTER TABLE `bot_spell_settings` DROP `max_level`;
