REPLACE INTO merc_spell_list_entries (merc_spell_list_id, spell_id, spell_type, stance_id, minlevel, maxlevel, slot, procChance) VALUES
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23181, 16, 0, 1, 9, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23182, 16, 0, 10, 19, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23183, 16, 0, 20, 24, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23184, 16, 0, 25, 29, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23185, 16, 0, 30, 34, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23186, 16, 0, 35, 39, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23187, 16, 0, 40, 44, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23188, 16, 0, 45, 49, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23189, 16, 0, 50, 54, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23190, 16, 0, 55, 59, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23191, 16, 0, 60, 64, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23192, 16, 0, 65, 69, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23193, 16, 0, 70, 74, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23194, 16, 0, 75, 79, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23195, 16, 0, 80, 80, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23196, 16, 0, 81, 81, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23197, 16, 0, 82, 82, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23198, 16, 0, 83, 83, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23199, 16, 0, 84, 84, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 23200, 16, 0, 85, 95, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), 37747, 16, 0, 96, 255, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Hide' ORDER BY id DESC LIMIT 1), 8, 0, 1, 255, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Sneak Attack' ORDER BY id DESC LIMIT 1), 1024, 0, 20, 51, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Thief\'s Vengeance' ORDER BY id DESC LIMIT 1), 1024, 0, 52, 62, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Assassin\'s Strike' ORDER BY id DESC LIMIT 1), 1024, 0, 63, 64, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Kyv Strike' ORDER BY id DESC LIMIT 1), 1024, 0, 65, 68, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Thief\'s eyes' ORDER BY id DESC LIMIT 1), 1024, 0, 68, 255, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerfall' ORDER BY id DESC LIMIT 1), 1024, 0, 69, 69, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Razorarc' ORDER BY id DESC LIMIT 1), 1024, 0, 70, 79, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Swiftblade' ORDER BY id DESC LIMIT 1), 1024, 0, 80, 84, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Swiftblade Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 80, 84, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Swiftblade Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 80, 84, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerlunge' ORDER BY id DESC LIMIT 1), 1024, 0, 85, 89, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerlunge Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 85, 89, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerlunge Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 85, 89, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerswipe' ORDER BY id DESC LIMIT 1), 1024, 0, 90, 94, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerswipe Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 90, 94, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerswipe Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 90, 94, 1, 0 );
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerstrike' ORDER BY id DESC LIMIT 1), 1024, 0, 95, 99, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerstrike Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 95, 9, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerstrike Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 95, 9, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerthrust' ORDER BY id DESC LIMIT 1), 1024, 0, 100, 255, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerthrust Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 100, 255, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 1), (SELECT id FROM spells_new WHERE name = 'Daggerthrust Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 100, 255, 1, 0 ),
	
	
REPLACE INTO merc_spell_list_entries (merc_spell_list_id, spell_id, spell_type, stance_id, minlevel, maxlevel, slot, procChance) VALUES
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23181, 16, 0, 1, 9, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23182, 16, 0, 10, 19, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23183, 16, 0, 20, 24, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23184, 16, 0, 25, 29, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23185, 16, 0, 30, 34, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23186, 16, 0, 35, 39, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23187, 16, 0, 40, 44, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23188, 16, 0, 45, 49, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23189, 16, 0, 50, 54, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23190, 16, 0, 55, 59, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23191, 16, 0, 60, 64, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23192, 16, 0, 65, 69, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23193, 16, 0, 70, 74, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23194, 16, 0, 75, 79, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23195, 16, 0, 80, 80, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23196, 16, 0, 81, 81, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23197, 16, 0, 82, 82, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23198, 16, 0, 83, 83, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23199, 16, 0, 84, 84, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 23200, 16, 0, 85, 95, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), 37747, 16, 0, 96, 255, 1, 0 ), -- Strike of Impaired Vision
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Hide' ORDER BY id DESC LIMIT 1), 8, 0, 1, 255, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Sneak Attack' ORDER BY id DESC LIMIT 1), 1024, 0, 20, 51, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Thief\'s Vengeance' ORDER BY id DESC LIMIT 1), 1024, 0, 52, 62, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Assassin\'s Strike' ORDER BY id DESC LIMIT 1), 1024, 0, 63, 64, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Ancient: Chaos Strike' ORDER BY id DESC LIMIT 1), 1024, 0, 65, 68, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Thief\'s eyes' ORDER BY id DESC LIMIT 1), 1024, 0, 68, 255, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerfall' ORDER BY id DESC LIMIT 1), 1024, 0, 69, 69, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Razorarc' ORDER BY id DESC LIMIT 1), 1024, 0, 70, 79, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Swiftblade' ORDER BY id DESC LIMIT 1), 1024, 0, 80, 84, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Swiftblade Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 80, 84, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Swiftblade Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 80, 84, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerlunge' ORDER BY id DESC LIMIT 1), 1024, 0, 85, 89, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerlunge Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 85, 89, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerlunge Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 85, 89, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerswipe' ORDER BY id DESC LIMIT 1), 1024, 0, 90, 94, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerswipe Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 90, 94, 1, 0 ),
	((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerswipe Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 90, 94, 1, 0 );
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerstrike' ORDER BY id DESC LIMIT 1), 1024, 0, 95, 99, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerstrike Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 95, 99, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerstrike Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 95, 99, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerthrust' ORDER BY id DESC LIMIT 1), 1024, 0, 100, 255, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerthrust Rk. II' ORDER BY id DESC LIMIT 1), 1024, 0, 100, 255, 1, 0 ),
	-- ((SELECT merc_spell_list_id FROM merc_spell_lists WHERE class_id = 9 AND proficiency_id = 2), (SELECT id FROM spells_new WHERE name = 'Daggerthrust Rk. III' ORDER BY id DESC LIMIT 1), 1024, 0, 100, 255, 1, 0 );
