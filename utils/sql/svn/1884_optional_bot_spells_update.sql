-- Shaman Spells
UPDATE npc_spells_entries SET priority = 2, maxlevel = 61 WHERE spellid = (SELECT id FROM spells_new WHERE name ='Chloroblast') AND npc_spells_id = (SELECT id FROM npc_spells WHERE name = 'Shaman Bot');

REPLACE INTO npc_spells_entries (npc_spells_id, spellid, type, minlevel, maxlevel, priority) VALUES
-- HoTs
((SELECT id FROM npc_spells WHERE name = 'Shaman Bot'), (SELECT id FROM spells_new WHERE name = 'Breath of Trushar'), 2, 65, 69, 1),
((SELECT id FROM npc_spells WHERE name = 'Shaman Bot'), (SELECT id FROM spells_new WHERE name = 'Spiritual Serenity'), 2, 70, 255, 1),
-- Heal 
((SELECT id FROM npc_spells WHERE name = 'Shaman Bot'), (SELECT id FROM spells_new WHERE name = 'Kragg\'s Mending'), 2, 58, 61, 2);


-- Druid Spells
REPLACE INTO npc_spells_entries (npc_spells_id, spellid, type, minlevel, maxlevel, priority) VALUES
-- Heals
((SELECT id FROM npc_spells WHERE name = 'Druid Bot'), (SELECT id FROM spells_new WHERE name = 'Tunare\'s Renewal'), 2, 58, 63, 2),
((SELECT id FROM npc_spells WHERE name = 'Druid Bot'), (SELECT id FROM spells_new WHERE name = 'Karana\'s Renewal'), 2, 64, 255, 2);


-- Paladin Spells
UPDATE npc_spells_entries SET minlevel = 58, maxlevel = 64 WHERE spellid = (SELECT id FROM spells_new WHERE name ='Healing Wave of Prexus') AND npc_spells_id = (SELECT id FROM npc_spells WHERE name = 'Paladin Bot');
UPDATE npc_spells_entries SET maxlevel = 61 WHERE spellid = (SELECT id FROM spells_new WHERE name ='Force of Akera') AND npc_spells_id = (SELECT id FROM npc_spells WHERE name = 'Paladin Bot');
UPDATE npc_spells_entries SET maxlevel = 64 WHERE spellid = (SELECT id FROM spells_new WHERE name ='Force of Akilae') AND npc_spells_id = (SELECT id FROM npc_spells WHERE name = 'Paladin Bot');
UPDATE npc_spells_entries SET maxlevel = 67 WHERE spellid = (SELECT id FROM spells_new WHERE name ='Quellious\' Word of Serenity') AND npc_spells_id = (SELECT id FROM npc_spells WHERE name = 'Paladin Bot');
UPDATE npc_spells_entries SET maxlevel = 69 WHERE spellid = (SELECT id FROM spells_new WHERE name ='Force of Piety') AND npc_spells_id = (SELECT id FROM npc_spells WHERE name = 'Paladin Bot');
UPDATE npc_spells_entries SET maxlevel = 255 WHERE spellid = (SELECT id FROM spells_new WHERE name ='Serene Command') AND npc_spells_id = (SELECT id FROM npc_spells WHERE name = 'Paladin Bot');

REPLACE INTO npc_spells_entries (npc_spells_id, spellid, type, minlevel, maxlevel, priority) VALUES
-- Group Heals
((SELECT id FROM npc_spells WHERE name = 'Paladin Bot'), (SELECT id FROM spells_new WHERE name = 'Wave of Life'), 2, 39, 54, 2),
((SELECT id FROM npc_spells WHERE name = 'Paladin Bot'), (SELECT id FROM spells_new WHERE name = 'Wave of Healing'), 2, 55, 57, 2),
((SELECT id FROM npc_spells WHERE name = 'Paladin Bot'), (SELECT id FROM spells_new WHERE name = 'Wave of Marr'), 2, 65, 69, 2),
((SELECT id FROM npc_spells WHERE name = 'Paladin Bot'), (SELECT id FROM spells_new WHERE name = 'Wave of Trushar'), 2, 65, 69, 2),
((SELECT id FROM npc_spells WHERE name = 'Paladin Bot'), (SELECT id FROM spells_new WHERE name = 'Wave of Piety'), 2, 70, 255, 2);


-- Cleric Spells
REPLACE INTO npc_spells_entries (npc_spells_id, spellid, type, minlevel, maxlevel, priority) VALUES
-- Fast Heals
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Remedy'), 2, 51, 60, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Supernal Remedy'), 2, 61, 65, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Pious Remedy'), 2, 66, 255, 2),
-- Regular Heals
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Divine Light'), 2, 53, 57, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Ethereal Light'), 2, 58, 62, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Supernal Light'), 2, 63, 64, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Holy Light'), 2, 65, 67, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Pious Light'), 2, 68, 69, 2),
-- Group Heals
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Word of Health'), 2, 30, 44, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Word of Healing'), 2, 45, 51, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Word of Vigor'), 2, 52, 56, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Word of Restoration'), 2, 57, 63, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Word of Redemption'), 2, 60, 255, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Word of Replenishment'), 2, 64, 68, 2),
((SELECT id FROM npc_spells WHERE name = 'Cleric Bot'), (SELECT id FROM spells_new WHERE name = 'Word of Vivification'), 2, 69, 255, 2);