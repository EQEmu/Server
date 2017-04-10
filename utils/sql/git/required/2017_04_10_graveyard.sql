alter table graveyard drop index zone_id;
create index zone_id_nonunique on graveyard(zone_id);
