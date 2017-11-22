alter table `traps` add column `triggered_number` tinyint(4) not null default 0;
alter table `traps` add column `group` tinyint(4) not null default 0;
alter table `traps` add column `despawn_when_triggered` tinyint(4) not null default 0;
alter table `traps` add column `undetectable` tinyint(4) not null default 0; 
