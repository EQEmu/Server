/* some specific by name */
UPDATE npc_types SET npc_types.runspeed = 3.175 WHERE npc_types.name = 'a_shadowed_man';
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE npc_types.name = 'aviak_egret';
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE npc_types.name = 'froglok_hunter';
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE npc_types.name = 'froglok_forager';
/* rhinos */
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE npc_types.race = 135;
/* centaurs */
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE npc_types.race = 16;
/* griffins */
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE npc_types.race = 47;
/* wolves - use size, to not change cubs*/
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE (npc_types.race = 42 and npc_types.size > 5);
/* sarnaks */
UPDATE npc_types SET npc_types.runspeed = 1.325 WHERE npc_types.race = 131;
/* sabertooth tigers - use size, to not change cubs*/
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE (npc_types.race = 119 and npc_types.size > 6);
/* lions */
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE (npc_types.race = 50 and npc_types.size > 7);
/* panthers/pumas */
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE npc_types.race = 76;
/* beetles */
UPDATE npc_types SET npc_types.runspeed = 1.05 WHERE npc_types.race = 22;
/*leeches*/
UPDATE npc_types SET npc_types.runspeed = 1.05 WHERE npc_types.race = 104;
/*a_brontotherium*/
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE npc_types.race = 169;
/* raptors */
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE npc_types.race = 163;
/* vicious plants */
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE npc_types.race = 162;
/* western wastes, drakes, cragwyrms and wyvern */
UPDATE npc_types
JOIN spawnentry ON npc_types.id = spawnentry.npcID
JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID
SET npc_types.runspeed = 1.575
WHERE ((npc_types.race = 89 OR npc_types.race = 157 OR npc_types.race = 158) AND spawn2.zone = 'westwastes');
/* velium hounds/wolves */
UPDATE npc_types
JOIN spawnentry ON npc_types.id = spawnentry.npcID
JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID
SET npc_types.runspeed = 1.850
WHERE (npc_types.race = 42 AND spawn2.zone = 'westwastes');
/* Overthere Specials, goons, etc. */
UPDATE npc_types
JOIN spawnentry ON npc_types.id = spawnentry.npcID
JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID
SET npc_types.runspeed = 1.850
WHERE ((npc_types.race = 77 or npc_types.race = 147) AND spawn2.zone = 'overthere');
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE npc_types.name = 'Captain_Rottgrime';
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE npc_types.name = 'an_undead_marine';
/* Pet Speeds. */
UPDATE npc_types
JOIN pets ON npc_types.id = pets.npcID
SET npc_types.runspeed = 1.575;
/* raptors in tim are slower than other raptors in kunark */
UPDATE npc_types
JOIN spawnentry ON npc_types.id = spawnentry.npcID
JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID
SET npc_types.runspeed = 1.325
WHERE ( npc_types.race = 163 AND spawn2.zone = 'timorous' );
