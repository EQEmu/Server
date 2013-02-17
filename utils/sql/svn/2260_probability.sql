-- Not needed if you ran 2213_loot_changes.sql after 12/14/12

ALTER TABLE  `loottable_entries` ADD  `probability` FLOAT NOT NULL DEFAULT  '100';