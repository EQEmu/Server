ALTER TABLE zone DROP PRIMARY KEY, DROP INDEX zoneidnumber, ADD INDEX zoneidnumber (zoneidnumber), ADD INDEX zonename (short_name);
ALTER TABLE zone ADD ruleset INT UNSIGNED DEFAULT '0' NOT NULL AFTER weather;
ALTER TABLE zone ADD version TINYINT UNSIGNED DEFAULT '0' NOT NULL AFTER zoneidnumber;
ALTER TABLE adventure_template ADD graveyard_zone_id INT UNSIGNED DEFAULT '0' NOT NULL AFTER dest_h;
ALTER TABLE adventure_template ADD graveyard_x FLOAT DEFAULT '0' NOT NULL AFTER graveyard_zone_id;
ALTER TABLE adventure_template ADD graveyard_y FLOAT DEFAULT '0' NOT NULL AFTER graveyard_x;
ALTER TABLE adventure_template ADD graveyard_z FLOAT DEFAULT '0' NOT NULL AFTER graveyard_y;
ALTER TABLE adventure_template ADD graveyard_radius FLOAT UNSIGNED DEFAULT '0.0' NOT NULL AFTER graveyard_z;