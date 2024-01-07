#ifndef EQEMU_CHARACTER_EXP_MODIFIERS_REPOSITORY_H
#define EQEMU_CHARACTER_EXP_MODIFIERS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_character_exp_modifiers_repository.h"

class CharacterExpModifiersRepository: public BaseCharacterExpModifiersRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * CharacterExpModifiersRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterExpModifiersRepository::GetWhereNeverExpires()
     * CharacterExpModifiersRepository::GetWhereXAndY()
     * CharacterExpModifiersRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static float GetEXPModifier(
		Database& db,
		uint32 character_id,
		uint32 zone_id,
		int16 instance_version
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				SQL(
					SELECT `exp_modifier` WHERE
					`character_id` = {}
					AND
					(`zone_id` = {} OR `zone_id` = 0) AND
					(`instance_version` = {} OR `instance_version` = -1)
					ORDER BY `zone_id`, `instance_version` DESC
					LIMIT 1
				),
				character_id,
				zone_id,
				instance_version
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 1.0f;
		}

		auto row = results.begin();

		return Strings::ToFloat(row[0]);
	}

	static float GetAAEXPModifier(
		Database& db,
		uint32 character_id,
		uint32 zone_id,
		int16 instance_version
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				SQL(
					SELECT `aa_modifier` WHERE
					`character_id` = {}
					AND
					(`zone_id` = {} OR `zone_id` = 0) AND
					(`instance_version` = {} OR `instance_version` = -1)
					ORDER BY `zone_id`, `instance_version` DESC
					LIMIT 1
				),
				character_id,
				zone_id,
				instance_version
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 1.0f;
		}

		auto row = results.begin();

		return Strings::ToFloat(row[0]);
	}

	static void SetAAEXPModifier(
		Database& db,
		uint32 character_id,
		uint32 zone_id,
		float aa_modifier,
		int16 instance_version
	)
	{
		const float exp_modifier = GetEXPModifier(
			db,
			character_id,
			zone_id,
			instance_version
		);

		auto e = NewEntity();

		e.character_id     = character_id;
		e.zone_id          = zone_id;
		e.instance_version = instance_version;
		e.aa_modifier      = aa_modifier;
		e.exp_modifier     = exp_modifier;

		CharacterExpModifiersRepository::ReplaceOne(db, e);
	}

	static void SetEXPModifier(
		Database& db,
		uint32 character_id,
		uint32 zone_id,
		float exp_modifier,
		int16 instance_version
	)
	{
		const float aa_modifier = GetAAEXPModifier(
			db,
			character_id,
			zone_id,
			instance_version
		);

		auto e = NewEntity();

		e.character_id     = character_id;
		e.zone_id          = zone_id;
		e.instance_version = instance_version;
		e.aa_modifier      = aa_modifier;
		e.exp_modifier     = exp_modifier;

		CharacterExpModifiersRepository::ReplaceOne(db, e);
	}
};

#endif //EQEMU_CHARACTER_EXP_MODIFIERS_REPOSITORY_H
