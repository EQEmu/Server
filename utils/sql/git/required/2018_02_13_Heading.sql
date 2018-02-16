UPDATE spawn2 SET heading = heading * 8.0 / 4.0;
UPDATE grid_entries SET heading = heading * 8.0 / 4.0 WHERE heading <> -1;
INSERT INTO variables (varname, value) VALUES ('fixed_heading', 1); -- hack
