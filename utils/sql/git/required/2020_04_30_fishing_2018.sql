/* Update all non-magical fishing poles to expendable per 2018 patch */

ALTER TABLE items RENAME COLUMN unk219 TO sub_type;
UPDATE items SET unk219 = !magic WHERE itemtype = 36;
