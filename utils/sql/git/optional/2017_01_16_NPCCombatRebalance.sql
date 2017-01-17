update npc_types set attack_speed=0, atk=ceil(1.7*level), accuracy=ceil(1.7*level) where (name like 'Swarm%' or name like '%skel%' or name like 'BLpet%' or name like 'Sum%') 
and id >510 and id <860;
update npc_types set attack_speed=0, atk=ceil(0.5*level), accuracy=ceil(0.5*level) where name like 'SumFire%';
update npc_types set attack_speed=0, atk=ceil(1.775*level), accuracy=ceil(1.775*level) where name like 'SumAir%';
update npc_types set attack_speed=0, atk=ceil(1.775*level), accuracy=ceil(1.775*level) where name like 'SumEarth%';
update npc_types set attack_speed=0, atk=ceil(2.26*level), accuracy=ceil(2.26*level),gender=2,size=3 where name like 'BestialAid%';
update npc_types set attack_speed=0, atk=ceil(2.26*level), accuracy=ceil(2.26*level),gender=2,size=3 where name like 'RagingServant%';


update npc_types n
set ac= 
ceil(case
when level < 3 then level*2+2 
when level < 15 and level >=3 then level*3
when id >= 200000 and id < 224000 then 200*.5+level
else level * 4.1 end 
* (case when raid_target=1 then 1.4 else 1 end) + (case when raid_target=1 then level*1.4 else 0 end))
+ 4*ifnull((select min(expansion)from zone where zoneidnumber=floor(n.id/1000)),5)
,str=ceil((level * 4.1)*.75)  + case when raid_target=1 then level else 0 end + 4*ifnull((select min(expansion)from zone where zoneidnumber=floor(n.id/1000)),5)
,sta=ceil((level * 4.1)*.75)  + case when raid_target=1 then level else 0 end + 4*ifnull((select min(expansion)from zone where zoneidnumber=floor(n.id/1000)),5)
,agi=ceil((level * 4.1)*.75)  + case when raid_target=1 then level else 0 end + 4*ifnull((select min(expansion)from zone where zoneidnumber=floor(n.id/1000)),5)
,_int=ceil((level * 4.1)*.75)  + case when raid_target=1 then level else 0 end + 4*ifnull((select min(expansion)from zone where zoneidnumber=floor(n.id/1000)),5)
,dex=ceil((level * 4.1)*.75)  + case when raid_target=1 then level else 0 end + 4*ifnull((select min(expansion)from zone where zoneidnumber=floor(n.id/1000)),5)
,wis=ceil((level * 4.1)*.75)  + case when raid_target=1 then level else 0 end + 4*ifnull((select min(expansion)from zone where zoneidnumber=floor(n.id/1000)),5)
,cha=ceil((level * 4.1)*.75)  + case when raid_target=1 then level else 0 end + 4*ifnull((select min(expansion)from zone where zoneidnumber=floor(n.id/1000)),5);
