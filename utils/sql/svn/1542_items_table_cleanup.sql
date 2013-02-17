-- This renames some unknown fields to known fields according to 13th floor data
-- 
ALTER TABLE items CHANGE UNK061 elitematerial INT(11) DEFAULT 0 NOT NULL;
ALTER TABLE items CHANGE UNK098 ldonsellbackrate INT(11) DEFAULT 70 NOT NULL;
ALTER TABLE items CHANGE UNK129 scriptfileid INT(11) DEFAULT 0 NOT NULL;
ALTER TABLE items CHANGE UNK131 expendablearrow INT(11) DEFAULT 0 NOT NULL;
-- 
-- The summonedflag field simply puts the Summoned Item note on newer summoned items, it has no real effect on gameplay
-- If you have summonedflag already set for custom items, you may not want to use the line below to copy UNK109 to summonedflag.
-- 
UPDATE items SET summonedflag = UNK109;
-- 
-- This drops multiple unknown fields that are not used and never will be
-- 
ALTER TABLE items DROP UNK109;
ALTER TABLE items DROP unknown002;
ALTER TABLE items DROP unknown003;
ALTER TABLE items DROP unknown005;
ALTER TABLE items DROP unknown007;
ALTER TABLE items DROP unknown018;
ALTER TABLE items DROP unknown019;
ALTER TABLE items DROP unknown020;
ALTER TABLE items DROP unknown061;
ALTER TABLE items DROP unknown067;
ALTER TABLE items DROP unknown069;
ALTER TABLE items DROP unknown081;
ALTER TABLE items DROP unknown105;
ALTER TABLE items DROP unknown122;
ALTER TABLE items DROP unknown123;
ALTER TABLE items DROP unknown124;
ALTER TABLE items DROP unknown128;
ALTER TABLE items DROP unknown133;