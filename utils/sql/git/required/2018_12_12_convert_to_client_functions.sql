/* 
This SQL update utilizes the new raw faction data from the client

First we create a temporary table - which we will use to map any
custom factions in the eqemu db, that are either:
   - eqemu utility factions
   - obsoleted factions with no new mapping to the client

This is done so that we can keep these factions while server owners either
stay with them, or migrate.  They are moved to the 5000+ range, to not conflict
with client faction_ids.
*/

/* Create the temp table and start mappings at 5000 */
CREATE TABLE custom_faction_mappings (old_faction int, new_faction int, primary key (old_faction)) engine=INNODB;

select "Moving custom factions to safe range, well above known client values" ``;
select @startcustom:=5000;

/* Insert the custom/obsolete factions into the temp mapping table */
insert into custom_faction_mappings (select id, @startcustom := @startcustom +1 from faction_list where id not in (select serverid from client_server_faction_map) and id < 5000);

CREATE TABLE IF NOT EXISTS faction_list_mod_oldfac AS SELECT * from faction_list_mod;

/* Now we update all the tables for these custom factions */

update faction_list_mod set faction_id = (select new_faction from custom_faction_mappings where old_faction = faction_id) where faction_id < 5000 and faction_id in (select old_faction from custom_faction_mappings);

CREATE TABLE IF NOT EXISTS faction_list_oldfac AS SELECT * from faction_list;

/* The faction_list table was forcing faction name to be a key.  Client does
   not.  Also, auto increment doesnt make sense anymore */
ALTER TABLE faction_list  CHANGE COLUMN `id` `id` INT(11) NOT NULL;
ALTER TABLE faction_list  DROP INDEX `name`;

update faction_list set id =
(select new_faction from custom_faction_mappings where old_faction = id) where id < 5000 and id in (select old_faction from custom_faction_mappings);

/* At this point all faction_mods for unmapped factions will be ids 5000+ */
/* So we can delete all the old ones still under 5000 - making room for the */
/* new faction ids */

delete from faction_list_mod where faction_id < 5000;

delete from faction_list where id < 5000;

/* Make an entry for each faction */
/* No base on client factions */

insert into faction_list (id, name, base)  (select id, name, 0 from client_faction_names);

/* Create mods based on the client_faction_associations */
/* No code changes required */

insert into faction_list_mod
(select null, faction_id, `mod`, concat("r", other_faction_id-50)
from client_faction_associations a
join client_faction_names n on n.id = a.other_faction_id
where other_faction_id between 51 and 180);

insert into faction_list_mod 
(select null, faction_id, `mod`, concat("c", other_faction_id)
from client_faction_associations a
join client_faction_names n on n.id = a.other_faction_id
where other_faction_id between 1 and 50);

insert into faction_list_mod 
(select null, faction_id, `mod`, concat("d", other_faction_id)
from client_faction_associations a
join client_faction_names n on n.id = a.other_faction_id
where other_faction_id between 201 and 216);

/* And now we need to fix all the other faction tables to point to the new factions. */

CREATE TABLE IF NOT EXISTS npc_faction_oldfac AS SELECT * from npc_faction;

update npc_faction set primaryfaction = (select new_faction from custom_faction_mappings where old_faction = primaryfaction) 
where primaryfaction in (select old_faction from custom_faction_mappings);

update npc_faction set primaryfaction = (select clientid from client_server_faction_map where serverid = primaryfaction) 
where primaryfaction in (select serverid from client_server_faction_map);

update npc_faction_entries set faction_id = (select new_faction from custom_faction_mappings where old_faction = faction_id) 
where faction_id in (select old_faction from custom_faction_mappings);

CREATE TABLE IF NOT EXISTS npc_faction_entries_oldfac AS SELECT * from npc_faction_entries;

/* Move existing factions out of wat - the following replace would create key */
/* duplicates along the way, but none when complete. */

update npc_faction_entries set faction_id = faction_id + 20000 
where faction_id in (select serverid from client_server_faction_map);

update npc_faction_entries set faction_id = (select clientid from client_server_faction_map where faction_id > 20000 && serverid = (faction_id-20000))
where faction_id > 20000 && (faction_id-20000) in (select serverid from client_server_faction_map);

/* Removes any duplicates from the use of factions that are obsoleted */
/* These are entries that have no new mapping whatsoever */
delete from npc_faction_entries where faction_id > 20000;


/* 
Update the faction_values now.

*/
delete from faction_values
 where faction_id not in (select old_faction from custom_faction_mappings) and faction_id not in (select serverid from client_server_faction_map);

/* Custom faction mappings dont have to worry about range collision */

select "Updating faction_values for custom factions" ``;

update faction_values set faction_id = (select new_faction from custom_faction_mappings where old_faction = faction_id) 
where faction_id in (select old_faction from custom_faction_mappings);

/* 
There are so many of these, Im going to update in place to save time.
To do this we must remove the unique keys, as these will be violated until
the update is complete
*/

select "Updating core faction_values to use new faction ids...." ``;

alter table faction_values drop primary key;

update faction_values v
join client_server_faction_map m on v.faction_id = m.serverid
set faction_id = m.clientid;

ALTER TABLE `faction_values` ADD PRIMARY KEY `lookup` (`char_id`,`faction_id`);
