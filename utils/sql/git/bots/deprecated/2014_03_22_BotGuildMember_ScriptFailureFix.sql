ALTER TABLE `botguildmembers` ADD `alt` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `public_note`; 

DROP VIEW IF EXISTS `vwGuildMembers`;
CREATE VIEW `vwGuildMembers` AS
  select 'C' as mobtype,
cm.char_id,
cm.guild_id,
cm.rank,
cm.tribute_enable,
cm.total_tribute,
cm.last_tribute,
cm.banker,
cm.public_note,
cm.alt
from guild_members as cm
union all
select 'B' as mobtype,
bm.char_id,
bm.guild_id,
bm.rank,
bm.tribute_enable,
bm.total_tribute,
bm.last_tribute,
bm.banker,
bm.public_note,
bm.alt
from botguildmembers as bm;