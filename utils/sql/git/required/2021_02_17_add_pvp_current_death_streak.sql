ALTER TABLE character_data ADD COLUMN pvp_current_death_streak INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER pvp_current_kill_streak;
