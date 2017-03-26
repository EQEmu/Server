-- Fix spells_id for existing Shadowknight entries
UPDATE `bot_data` SET `spells_id` = '3005' WHERE `class` = '5';
