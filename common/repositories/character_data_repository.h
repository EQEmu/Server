#ifndef EQEMU_CHARACTER_DATA_REPOSITORY_H
#define EQEMU_CHARACTER_DATA_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_character_data_repository.h"



class CharacterDataRepository: public BaseCharacterDataRepository {
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
     * CharacterDataRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterDataRepository::GetWhereNeverExpires()
     * CharacterDataRepository::GetWhereXAndY()
     * CharacterDataRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static uint32 GetSecondsSinceLastLogin(Database &db, const std::string& name)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT (UNIX_TIMESTAMP(NOW()) - last_login) FROM {} WHERE name = '{}'",
				TableName(),
				Strings::Escape(name)
			)
		);

		if (!results.RowCount() || !results.Success()) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToUnsignedInt(row[0]);
	}

	static CharacterData FindByName(
		Database& db,
		const std::string& character_name
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE `name` = '{}' LIMIT 1",
				BaseSelect(),
				Strings::Escape(character_name)
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterData e{};

			e.id                      = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.account_id              = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.zone_instance           = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.y                       = row[8] ? strtof(row[8], nullptr) : 0;
			e.x                       = row[9] ? strtof(row[9], nullptr) : 0;
			e.z                       = row[10] ? strtof(row[10], nullptr) : 0;
			e.heading                 = row[11] ? strtof(row[11], nullptr) : 0;
			e.gender                  = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.race                    = row[13] ? static_cast<uint16_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.class_                  = row[14] ? static_cast<uint8_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.level                   = row[15] ? static_cast<uint32_t>(strtoul(row[15], nullptr, 10)) : 0;
			e.deity                   = row[16] ? static_cast<uint32_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.birthday                = row[17] ? static_cast<uint32_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.last_login              = row[18] ? static_cast<uint32_t>(strtoul(row[18], nullptr, 10)) : 0;
			e.time_played             = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.level2                  = row[20] ? static_cast<uint8_t>(strtoul(row[20], nullptr, 10)) : 0;
			e.anon                    = row[21] ? static_cast<uint8_t>(strtoul(row[21], nullptr, 10)) : 0;
			e.gm                      = row[22] ? static_cast<uint8_t>(strtoul(row[22], nullptr, 10)) : 0;
			e.face                    = row[23] ? static_cast<uint32_t>(strtoul(row[23], nullptr, 10)) : 0;
			e.hair_color              = row[24] ? static_cast<uint8_t>(strtoul(row[24], nullptr, 10)) : 0;
			e.hair_style              = row[25] ? static_cast<uint8_t>(strtoul(row[25], nullptr, 10)) : 0;
			e.beard                   = row[26] ? static_cast<uint8_t>(strtoul(row[26], nullptr, 10)) : 0;
			e.beard_color             = row[27] ? static_cast<uint8_t>(strtoul(row[27], nullptr, 10)) : 0;
			e.eye_color_1             = row[28] ? static_cast<uint8_t>(strtoul(row[28], nullptr, 10)) : 0;
			e.eye_color_2             = row[29] ? static_cast<uint8_t>(strtoul(row[29], nullptr, 10)) : 0;
			e.drakkin_heritage        = row[30] ? static_cast<uint32_t>(strtoul(row[30], nullptr, 10)) : 0;
			e.drakkin_tattoo          = row[31] ? static_cast<uint32_t>(strtoul(row[31], nullptr, 10)) : 0;
			e.drakkin_details         = row[32] ? static_cast<uint32_t>(strtoul(row[32], nullptr, 10)) : 0;
			e.ability_time_seconds    = row[33] ? static_cast<uint8_t>(strtoul(row[33], nullptr, 10)) : 0;
			e.ability_number          = row[34] ? static_cast<uint8_t>(strtoul(row[34], nullptr, 10)) : 0;
			e.ability_time_minutes    = row[35] ? static_cast<uint8_t>(strtoul(row[35], nullptr, 10)) : 0;
			e.ability_time_hours      = row[36] ? static_cast<uint8_t>(strtoul(row[36], nullptr, 10)) : 0;
			e.exp                     = row[37] ? static_cast<uint32_t>(strtoul(row[37], nullptr, 10)) : 0;
			e.exp_enabled             = row[38] ? static_cast<uint8_t>(strtoul(row[38], nullptr, 10)) : 1;
			e.aa_points_spent         = row[39] ? static_cast<uint32_t>(strtoul(row[39], nullptr, 10)) : 0;
			e.aa_exp                  = row[40] ? static_cast<uint32_t>(strtoul(row[40], nullptr, 10)) : 0;
			e.aa_points               = row[41] ? static_cast<uint32_t>(strtoul(row[41], nullptr, 10)) : 0;
			e.group_leadership_exp    = row[42] ? static_cast<uint32_t>(strtoul(row[42], nullptr, 10)) : 0;
			e.raid_leadership_exp     = row[43] ? static_cast<uint32_t>(strtoul(row[43], nullptr, 10)) : 0;
			e.group_leadership_points = row[44] ? static_cast<uint32_t>(strtoul(row[44], nullptr, 10)) : 0;
			e.raid_leadership_points  = row[45] ? static_cast<uint32_t>(strtoul(row[45], nullptr, 10)) : 0;
			e.points                  = row[46] ? static_cast<uint32_t>(strtoul(row[46], nullptr, 10)) : 0;
			e.cur_hp                  = row[47] ? static_cast<uint32_t>(strtoul(row[47], nullptr, 10)) : 0;
			e.mana                    = row[48] ? static_cast<uint32_t>(strtoul(row[48], nullptr, 10)) : 0;
			e.endurance               = row[49] ? static_cast<uint32_t>(strtoul(row[49], nullptr, 10)) : 0;
			e.intoxication            = row[50] ? static_cast<uint32_t>(strtoul(row[50], nullptr, 10)) : 0;
			e.str                     = row[51] ? static_cast<uint32_t>(strtoul(row[51], nullptr, 10)) : 0;
			e.sta                     = row[52] ? static_cast<uint32_t>(strtoul(row[52], nullptr, 10)) : 0;
			e.cha                     = row[53] ? static_cast<uint32_t>(strtoul(row[53], nullptr, 10)) : 0;
			e.dex                     = row[54] ? static_cast<uint32_t>(strtoul(row[54], nullptr, 10)) : 0;
			e.int_                    = row[55] ? static_cast<uint32_t>(strtoul(row[55], nullptr, 10)) : 0;
			e.agi                     = row[56] ? static_cast<uint32_t>(strtoul(row[56], nullptr, 10)) : 0;
			e.wis                     = row[57] ? static_cast<uint32_t>(strtoul(row[57], nullptr, 10)) : 0;
			e.extra_haste             = row[58] ? static_cast<int32_t>(atoi(row[58])) : 0;
			e.zone_change_count       = row[59] ? static_cast<uint32_t>(strtoul(row[59], nullptr, 10)) : 0;
			e.toxicity                = row[60] ? static_cast<uint32_t>(strtoul(row[60], nullptr, 10)) : 0;
			e.hunger_level            = row[61] ? static_cast<uint32_t>(strtoul(row[61], nullptr, 10)) : 0;
			e.thirst_level            = row[62] ? static_cast<uint32_t>(strtoul(row[62], nullptr, 10)) : 0;
			e.ability_up              = row[63] ? static_cast<uint32_t>(strtoul(row[63], nullptr, 10)) : 0;
			e.ldon_points_guk         = row[64] ? static_cast<uint32_t>(strtoul(row[64], nullptr, 10)) : 0;
			e.ldon_points_mir         = row[65] ? static_cast<uint32_t>(strtoul(row[65], nullptr, 10)) : 0;
			e.ldon_points_mmc         = row[66] ? static_cast<uint32_t>(strtoul(row[66], nullptr, 10)) : 0;
			e.ldon_points_ruj         = row[67] ? static_cast<uint32_t>(strtoul(row[67], nullptr, 10)) : 0;
			e.ldon_points_tak         = row[68] ? static_cast<uint32_t>(strtoul(row[68], nullptr, 10)) : 0;
			e.ldon_points_available   = row[69] ? static_cast<uint32_t>(strtoul(row[69], nullptr, 10)) : 0;
			e.tribute_time_remaining  = row[70] ? static_cast<uint32_t>(strtoul(row[70], nullptr, 10)) : 0;
			e.career_tribute_points   = row[71] ? static_cast<uint32_t>(strtoul(row[71], nullptr, 10)) : 0;
			e.tribute_points          = row[72] ? static_cast<uint32_t>(strtoul(row[72], nullptr, 10)) : 0;
			e.tribute_active          = row[73] ? static_cast<uint32_t>(strtoul(row[73], nullptr, 10)) : 0;
			e.pvp_status              = row[74] ? static_cast<uint8_t>(strtoul(row[74], nullptr, 10)) : 0;
			e.pvp_kills               = row[75] ? static_cast<uint32_t>(strtoul(row[75], nullptr, 10)) : 0;
			e.pvp_deaths              = row[76] ? static_cast<uint32_t>(strtoul(row[76], nullptr, 10)) : 0;
			e.pvp_current_points      = row[77] ? static_cast<uint32_t>(strtoul(row[77], nullptr, 10)) : 0;
			e.pvp_career_points       = row[78] ? static_cast<uint32_t>(strtoul(row[78], nullptr, 10)) : 0;
			e.pvp_best_kill_streak    = row[79] ? static_cast<uint32_t>(strtoul(row[79], nullptr, 10)) : 0;
			e.pvp_worst_death_streak  = row[80] ? static_cast<uint32_t>(strtoul(row[80], nullptr, 10)) : 0;
			e.pvp_current_kill_streak = row[81] ? static_cast<uint32_t>(strtoul(row[81], nullptr, 10)) : 0;
			e.pvp2                    = row[82] ? static_cast<uint32_t>(strtoul(row[82], nullptr, 10)) : 0;
			e.pvp_type                = row[83] ? static_cast<uint32_t>(strtoul(row[83], nullptr, 10)) : 0;
			e.show_helm               = row[84] ? static_cast<uint32_t>(strtoul(row[84], nullptr, 10)) : 0;
			e.group_auto_consent      = row[85] ? static_cast<uint8_t>(strtoul(row[85], nullptr, 10)) : 0;
			e.raid_auto_consent       = row[86] ? static_cast<uint8_t>(strtoul(row[86], nullptr, 10)) : 0;
			e.guild_auto_consent      = row[87] ? static_cast<uint8_t>(strtoul(row[87], nullptr, 10)) : 0;
			e.leadership_exp_on       = row[88] ? static_cast<uint8_t>(strtoul(row[88], nullptr, 10)) : 0;
			e.RestTimer               = row[89] ? static_cast<uint32_t>(strtoul(row[89], nullptr, 10)) : 0;
			e.air_remaining           = row[90] ? static_cast<uint32_t>(strtoul(row[90], nullptr, 10)) : 0;
			e.autosplit_enabled       = row[91] ? static_cast<uint32_t>(strtoul(row[91], nullptr, 10)) : 0;
			e.lfp                     = row[92] ? static_cast<uint8_t>(strtoul(row[92], nullptr, 10)) : 0;
			e.lfg                     = row[93] ? static_cast<uint8_t>(strtoul(row[93], nullptr, 10)) : 0;
			e.mailkey                 = row[94] ? row[94] : "";
			e.xtargets                = row[95] ? static_cast<uint8_t>(strtoul(row[95], nullptr, 10)) : 5;
			e.firstlogon              = row[96] ? static_cast<int8_t>(atoi(row[96])) : 0;
			e.e_aa_effects            = row[97] ? static_cast<uint32_t>(strtoul(row[97], nullptr, 10)) : 0;
			e.e_percent_to_aa         = row[98] ? static_cast<uint32_t>(strtoul(row[98], nullptr, 10)) : 0;
			e.e_expended_aa_spent     = row[99] ? static_cast<uint32_t>(strtoul(row[99], nullptr, 10)) : 0;
			e.aa_points_spent_old     = row[100] ? static_cast<uint32_t>(strtoul(row[100], nullptr, 10)) : 0;
			e.aa_points_old           = row[101] ? static_cast<uint32_t>(strtoul(row[101], nullptr, 10)) : 0;
			e.e_last_invsnapshot      = row[102] ? static_cast<uint32_t>(strtoul(row[102], nullptr, 10)) : 0;
			e.deleted_at              = strtoll(row[103] ? row[103] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}
};

#endif //EQEMU_CHARACTER_DATA_REPOSITORY_H
