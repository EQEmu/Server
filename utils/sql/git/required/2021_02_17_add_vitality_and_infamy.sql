ALTER TABLE character_data ADD COLUMN pvp_infamy INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER pvp_current_death_streak;
ALTER TABLE character_data ADD COLUMN pvp_vitality INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER pvp_infamy;
