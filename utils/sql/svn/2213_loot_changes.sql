alter table loottable_entries add `droplimit` tinyint(2) unsigned NOT NULL default 0;
alter table loottable_entries add `mindrop` tinyint(2) unsigned NOT NULL default 0;
alter table lootdrop_entries change `chance` `chance` float not null default 1;
alter table lootdrop_entries add `multiplier` tinyint(2) unsigned NOT NULL default 1;
update loottable_entries set droplimit = 1 where probability != 100;
update loottable_entries set mindrop = multiplier, droplimit = multiplier, multiplier = 1 where probability = 100;
