ALTER TABLE `bot_data`
ADD COLUMN `enforce_spell_settings` TINYINT(2) UNSIGNED NOT NULL DEFAULT '0' AFTER `expansion_bitmask`;