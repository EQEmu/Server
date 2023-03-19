ALTER TABLE npc_types ADD COLUMN hp_regen_per_second bigint DEFAULT 0 AFTER hp_regen_rate;
