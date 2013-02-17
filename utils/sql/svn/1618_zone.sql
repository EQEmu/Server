ALTER TABLE zone ADD suspendbuffs tinyint(1) unsigned NOT NULL DEFAULT 0;
UPDATE zone SET suspendbuffs = 1 WHERE short_name IN ('guildlobby', 'guildhall');
