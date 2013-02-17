RENAME TABLE botinventory TO oldbotinventory;

ALTER TABLE bots ADD COLUMN (oldnpcid int(11) NOT NULL default 0);

SOURCE bots.sql;

INSERT INTO bots SELECT NULL, bo.botleadercharacterid, nt.npc_spells_id, nt.name, nt.lastname, nt.level, nt.race, nt.class, nt.bodytype, nt.hp, nt.gender, nt.size, nt.hp_regen_rate, nt.mana_regen_rate, nt.face, nt.luclin_hairstyle, nt.luclin_haircolor, nt.luclin_eyecolor, nt.luclin_eyecolor2, nt.luclin_beardcolor, nt.luclin_beard, nt.drakkin_heritage, nt.drakkin_tattoo, nt.drakkin_details, nt.runspeed, nt.MR, nt.CR, nt.DR, nt.FR, nt.PR, nt.AC, nt.STR, nt.STA, nt.DEX, nt.AGI, nt._INT, nt.WIS, nt.CHA, nt.ATK, nt.id FROM botsowners bo, npc_types nt WHERE nt.isbot = 1 AND nt.id = bo.botnpctypeid;

INSERT INTO botinventory SELECT NULL, b.BotID, i.botslotid, i.itemid FROM bots b, oldbotinventory i WHERE b.oldnpcid = i.npctypeid;

ALTER TABLE bots DROP COLUMN oldnpcid;

DROP TABLE IF EXISTS oldbotinventory;
DROP TABLE IF EXISTS botsleaders;

DELETE FROM npc_types WHERE isbot = 1;
