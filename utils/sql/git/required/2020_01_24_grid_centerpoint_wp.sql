alter table grid_entries add column `centerpoint` tinyint(4) not null default 0;
alter table spawngroup add column `wp_spawns` tinyint(1) unsigned not null default 0;