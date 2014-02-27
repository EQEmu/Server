alter table `spawngroup` add column `mindelay` int(11) not null default 15000 AFTER delay;
alter table `spawngroup` change `delay` `delay` int(11) not null default 45000;