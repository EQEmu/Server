/* This rescales the old peq runspeeds which were about 80 percent too high to new values */
/* This section should only ever be run once */
UPDATE npc_types SET npc_types.runspeed = 1.050 WHERE (npc_types.runspeed > 0 and npc_types.runspeed < 1.2);
UPDATE npc_types SET npc_types.runspeed = 1.325 WHERE (npc_types.runspeed > 1.19 and npc_types.runspeed < 1.75 and race != 73 and race != 72);
UPDATE npc_types SET npc_types.runspeed = 1.575 WHERE (npc_types.runspeed > 1.69 and npc_types.runspeed < 2.2);
UPDATE npc_types SET npc_types.runspeed = 1.850 WHERE (npc_types.runspeed > 2.19 and npc_types.runspeed < 3);
UPDATE npc_types SET npc_types.runspeed = 3 WHERE npc_types.runspeed > 3;