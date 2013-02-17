delete from rule_values where rule_name like 'EQOffline%';
INSERT INTO rule_values VALUES ('0', 'EQOffline:CreateBotCount', 150);
INSERT INTO rule_values VALUES ('0', 'EQOffline:SpawnBotCount', 71);
INSERT INTO rule_values VALUES ('0', 'EQOffline:BotQuest', 'true');
delete from npc_types where name='Aediles_Thrall';
delete from npc_types where name='Aspen';
insert into npc_types (    id,             name, lastname, level, race, class, bodytype,    hp, gender,  texture, helmtexture, size, hp_regen_rate, mana_regen_rate, loottable_id, merchant_id, npc_spells_id, npc_faction_id, mindmg, maxdmg, npcspecialattks, aggroradius, face, runspeed,  MR,  CR,  DR,  FR,  PR, see_invis, see_invis_undead,  qglobal,   AC, spawn_limit,  findable, STR,  STA,  DEX,  AGI,  _INT, WIS, CHA, see_hide, see_improved_hide, trackable, isbot)
               VALUES (151070, 'Aediles_Thrall',       '',    71,    8,     1,        1, 10000,      0,        0,           0,  5.0,           500,               0,            0,           0,             0,              0,      1,      1,              '',          70,  255,      1.9,   0,   0,   0,   0,   0,         1,                1,        1, 5000,           0,         1, 192,  192,  192,  192,   192, 192, 192,        1,                 1,         1,     0);
insert into npc_types (    id,    name, lastname, level, race, class, bodytype,    hp, gender,  texture, helmtexture, size, hp_regen_rate, mana_regen_rate, loottable_id, merchant_id, npc_spells_id, npc_faction_id, mindmg, maxdmg, npcspecialattks, aggroradius, face, runspeed,  MR,  CR,  DR,  FR,  PR, see_invis, see_invis_undead,  qglobal,   AC, spawn_limit,  findable, STR,  STA,  DEX,  AGI,  _INT, WIS, CHA, see_hide, see_improved_hide, trackable, isbot)
               VALUES (151071, 'Aspen',       '',     5,    7,     1,        1, 10000,      0,        0,           0,  4.5,           500,               0,            0,           0,             0,              0,      1,      1,              '',          70,  255,      1.9,   0,   0,   0,   0,   0,         1,                1,        1, 5000,           0,         1, 192,  192,  192,  192,   192, 192, 192,        1,                 1,         1,     0);
delete from spawn2 where spawngroupid=49000 and zone='bazaar';
delete from spawn2 where spawngroupid=49001 and zone='bazaar';
insert into spawn2 (spawngroupID, zone, x, y, z, heading, respawntime) values (60000, 'bazaar', 119.385948, -402.999329, 33.257092, 224.250000, 1200);
insert into spawn2 (spawngroupID, zone, x, y, z, heading, respawntime) values (60001, 'bazaar', 116.750000, -400.375000, 33.257092, 89.875000, 1200);
delete from spawngroup where name='bazaar_Aediles_Thrall';
delete from spawngroup where name='bazaar_Aspen';
insert into spawngroup (id, name) values (60000, 'bazaar_Aediles_Thrall');
insert into spawngroup (id, name) values (60001, 'bazaar_Aspen');
delete from spawnentry where npcid=151070;
delete from spawnentry where npcid=151071;
insert into spawnentry (spawngroupID, npcID, chance) values (60000, 151070, 100);
insert into spawnentry (spawngroupID, npcID, chance) values (60001, 151071, 100);

