/* Fix any items with faction adjustments */

SET SQL_MODE='ALLOW_INVALID_DATES'; /* Some dbs have bad dates which prevents the index creation */

CREATE INDEX itemfac1 ON items (factionmod1);
CREATE INDEX itemfac2 ON items (factionmod2);
CREATE INDEX itemfac3 ON items (factionmod3);
CREATE INDEX itemfac4 ON items (factionmod4);

UPDATE items i
INNER JOIN custom_faction_mappings m ON i.factionmod1 = m.old_faction
SET i.factionmod1 = m.new_faction
WHERE i.factionmod1 > 0;

UPDATE items i
INNER JOIN custom_faction_mappings m ON i.factionmod2 = m.old_faction
SET i.factionmod2 = m.new_faction
WHERE i.factionmod2 > 0;

UPDATE items i
INNER JOIN custom_faction_mappings m ON i.factionmod3 = m.old_faction
SET i.factionmod3 = m.new_faction
WHERE i.factionmod3 > 0;

UPDATE items i
INNER JOIN custom_faction_mappings m ON i.factionmod4 = m.old_faction
SET i.factionmod4 = m.new_faction
WHERE i.factionmod4 > 0;

UPDATE items i
INNER JOIN client_server_faction_map m ON i.factionmod1 = m.serverid
SET i.factionmod1 = m.clientid
WHERE i.factionmod1 > 0;

UPDATE items i
INNER JOIN client_server_faction_map m ON i.factionmod2 = m.serverid
SET i.factionmod2 = m.clientid
WHERE i.factionmod2 > 0;

UPDATE items i
INNER JOIN client_server_faction_map m ON i.factionmod3 = m.serverid
SET i.factionmod3 = m.clientid
WHERE i.factionmod3 > 0;

UPDATE items i
INNER JOIN client_server_faction_map m ON i.factionmod4 = m.serverid
SET i.factionmod4 = m.clientid
WHERE i.factionmod4 > 0;

DROP INDEX itemfac1 ON items;
DROP INDEX itemfac2 ON items;
DROP INDEX itemfac3 ON items;
DROP INDEX itemfac4 ON items;
