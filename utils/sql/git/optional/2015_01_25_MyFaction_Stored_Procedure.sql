DELIMITER $$

CREATE PROCEDURE `MyFaction`(charname text)
BEGIN
declare class_mod text default "";
declare race_mod text default "";
declare deity_mod text default "";

select class into class_mod from character_data where name = charname;
select race into race_mod from character_data where name = charname;
select deity into deity_mod from character_data where name = charname;
select concat("c", class_mod) into class_mod;
select concat("r", race_mod) into race_mod;
select concat("d", deity_mod) into deity_mod;

set @class_bump := 0;
set @race_bump := 0;
set @deity_bump := 0;

SELECT race_mod as R, class_mod as C, deity_mod as D, f.name as faction, f.id, v.current_value, f.base as "START",
@class_bump := IFNULL((select m.mod from faction_list_mod m where faction_id = f.id && class_mod != "" && mod_name = class_mod),0) as class_bump,
@race_bump := IFNULL((select m.mod from faction_list_mod m where faction_id = f.id && race_mod != "" && mod_name = race_mod),0) as race_bump,
@deity_bump := IFNULL((select m.mod from faction_list_mod m where faction_id = f.id && race_mod != "" && mod_name = deity_mod),0) as deity_bump,
v.current_value + f.base + @class_bump + @race_bump + @deity_bump as TOTAL
FROM faction_values v
inner join faction_list f on f.id = v.faction_id
where v.char_id = (select id from character_data where name=charname) ;
END $$

DELIMITER ;
