-- Add new path_when_zone_idle flag to allow some spawns to path in empty zones
ALTER TABLE spawn2 ADD COLUMN path_when_zone_idle tinyint(1) NOT NULL DEFAULT 0 AFTER pathgrid;

-- Update spawns that used to path in empty zones because of their grid type
-- to behave the same using the new mechanism.  The code that checked path grid
-- types has been removed as it was coincidentally coupled to idle movement.
-- The new flag path_when_zone_idle is the new mechanism, and allows any moving
-- mob, not just those on grids, to path while the zone is idle.
UPDATE spawn2 s 
LEFT JOIN zone z ON z.short_name = s.zone 
LEFT JOIN grid g ON g.id = s.pathgrid AND g.zoneid = z.zoneidnumber 
SET path_when_zone_idle = 1 
WHERE pathgrid != 0 AND g.type IN (4, 6);
