alter table bots
drop column TotalPlayTime,
add column TotalPlayTime int unsigned NOT NULL DEFAULT 0;