-- Required
ALTER TABLE botinventory ADD COLUMN 
   (charges tinyint(3) unsigned DEFAULT 0,
    color INTEGER unsigned NOT NULL DEFAULT 0,
    augslot1 mediumint(7) unsigned NOT NULL DEFAULT 0,
    augslot2 mediumint(7) unsigned NOT NULL DEFAULT 0,
    augslot3 mediumint(7) unsigned NOT NULL DEFAULT 0,
    augslot4 mediumint(7) unsigned NOT NULL DEFAULT 0,
    augslot5 mediumint(7) unsigned DEFAULT 0,
    instnodrop tinyint(1) unsigned NOT NULL DEFAULT 0);

-- only required for updating existing bots
UPDATE botinventory SET charges=(select maxcharges from items WHERE id=itemid) WHERE (select maxcharges from items WHERE id=itemid)>=0;
UPDATE botinventory SET charges=255 WHERE (select maxcharges from items WHERE id=itemid)=(-1);
UPDATE botinventory SET charges=1 WHERE charges=0;
UPDATE botinventory SET color=(select color from items WHERE id=itemid) WHERE itemid=(SELECT id FROM items WHERE id=itemid);
UPDATE botinventory SET instnodrop=(select attuneable from items WHERE id=itemid) WHERE itemid=(SELECT id FROM items WHERE id=itemid);

