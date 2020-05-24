/* In July 2018 - magical poles were no longer breakable */

UPDATE items SET subtype = magic WHERE itemtype = 36;
