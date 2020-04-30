/* Update all non-magical fishing poles to expendable per 2018 patch */

UPDATE items SET expendablearrow = 1 where id in (9969, 9666, 16863, 13100, 84004, 25154, 22857, 46996);
