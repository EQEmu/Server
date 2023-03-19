/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://eqemu.gitbook.io/server/in-development/developer-area/repositories
 */

#ifndef EQEMU_BASE_BOT_DATA_REPOSITORY_H
#define EQEMU_BASE_BOT_DATA_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseBotDataRepository {
public:
	struct BotData {
		uint32_t    bot_id;
		uint32_t    owner_id;
		uint32_t    spells_id;
		std::string name;
		std::string last_name;
		std::string title;
		std::string suffix;
		int16_t     zone_id;
		int8_t      gender;
		int16_t     race;
		int8_t      class_;
		uint8_t     level;
		uint32_t    deity;
		uint32_t    creation_day;
		uint32_t    last_spawn;
		uint32_t    time_spawned;
		float       size;
		int32_t     face;
		int32_t     hair_color;
		int32_t     hair_style;
		int32_t     beard;
		int32_t     beard_color;
		int32_t     eye_color_1;
		int32_t     eye_color_2;
		int32_t     drakkin_heritage;
		int32_t     drakkin_tattoo;
		int32_t     drakkin_details;
		int16_t     ac;
		int32_t     atk;
		int32_t     hp;
		int32_t     mana;
		int32_t     str;
		int32_t     sta;
		int32_t     cha;
		int32_t     dex;
		int32_t     int_;
		int32_t     agi;
		int32_t     wis;
		int16_t     fire;
		int16_t     cold;
		int16_t     magic;
		int16_t     poison;
		int16_t     disease;
		int16_t     corruption;
		uint32_t    show_helm;
		uint32_t    follow_distance;
		uint8_t     stop_melee_level;
		int32_t     expansion_bitmask;
		uint8_t     enforce_spell_settings;
		uint8_t     archery_setting;
	};

	static std::string PrimaryKey()
	{
		return std::string("bot_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"bot_id",
			"owner_id",
			"spells_id",
			"name",
			"last_name",
			"title",
			"suffix",
			"zone_id",
			"gender",
			"race",
			"`class`",
			"level",
			"deity",
			"creation_day",
			"last_spawn",
			"time_spawned",
			"size",
			"face",
			"hair_color",
			"hair_style",
			"beard",
			"beard_color",
			"eye_color_1",
			"eye_color_2",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"ac",
			"atk",
			"hp",
			"mana",
			"str",
			"sta",
			"cha",
			"dex",
			"`int`",
			"agi",
			"wis",
			"fire",
			"cold",
			"magic",
			"poison",
			"disease",
			"corruption",
			"show_helm",
			"follow_distance",
			"stop_melee_level",
			"expansion_bitmask",
			"enforce_spell_settings",
			"archery_setting",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"bot_id",
			"owner_id",
			"spells_id",
			"name",
			"last_name",
			"title",
			"suffix",
			"zone_id",
			"gender",
			"race",
			"`class`",
			"level",
			"deity",
			"creation_day",
			"last_spawn",
			"time_spawned",
			"size",
			"face",
			"hair_color",
			"hair_style",
			"beard",
			"beard_color",
			"eye_color_1",
			"eye_color_2",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"ac",
			"atk",
			"hp",
			"mana",
			"str",
			"sta",
			"cha",
			"dex",
			"`int`",
			"agi",
			"wis",
			"fire",
			"cold",
			"magic",
			"poison",
			"disease",
			"corruption",
			"show_helm",
			"follow_distance",
			"stop_melee_level",
			"expansion_bitmask",
			"enforce_spell_settings",
			"archery_setting",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("bot_data");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static BotData NewEntity()
	{
		BotData e{};

		e.bot_id                 = 0;
		e.owner_id               = 0;
		e.spells_id              = 0;
		e.name                   = "";
		e.last_name              = "";
		e.title                  = "";
		e.suffix                 = "";
		e.zone_id                = 0;
		e.gender                 = 0;
		e.race                   = 0;
		e.class_                 = 0;
		e.level                  = 0;
		e.deity                  = 0;
		e.creation_day           = 0;
		e.last_spawn             = 0;
		e.time_spawned           = 0;
		e.size                   = 0;
		e.face                   = 1;
		e.hair_color             = 1;
		e.hair_style             = 1;
		e.beard                  = 0;
		e.beard_color            = 1;
		e.eye_color_1            = 1;
		e.eye_color_2            = 1;
		e.drakkin_heritage       = 0;
		e.drakkin_tattoo         = 0;
		e.drakkin_details        = 0;
		e.ac                     = 0;
		e.atk                    = 0;
		e.hp                     = 0;
		e.mana                   = 0;
		e.str                    = 75;
		e.sta                    = 75;
		e.cha                    = 75;
		e.dex                    = 75;
		e.int_                   = 75;
		e.agi                    = 75;
		e.wis                    = 75;
		e.fire                   = 0;
		e.cold                   = 0;
		e.magic                  = 0;
		e.poison                 = 0;
		e.disease                = 0;
		e.corruption             = 0;
		e.show_helm              = 0;
		e.follow_distance        = 200;
		e.stop_melee_level       = 255;
		e.expansion_bitmask      = -1;
		e.enforce_spell_settings = 0;
		e.archery_setting        = 0;

		return e;
	}

	static BotData GetBotData(
		const std::vector<BotData> &bot_datas,
		int bot_data_id
	)
	{
		for (auto &bot_data : bot_datas) {
			if (bot_data.bot_id == bot_data_id) {
				return bot_data;
			}
		}

		return NewEntity();
	}

	static BotData FindOne(
		Database& db,
		int bot_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotData e{};

			e.bot_id                 = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.owner_id               = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spells_id              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.name                   = row[3] ? row[3] : "";
			e.last_name              = row[4] ? row[4] : "";
			e.title                  = row[5] ? row[5] : "";
			e.suffix                 = row[6] ? row[6] : "";
			e.zone_id                = static_cast<int16_t>(atoi(row[7]));
			e.gender                 = static_cast<int8_t>(atoi(row[8]));
			e.race                   = static_cast<int16_t>(atoi(row[9]));
			e.class_                 = static_cast<int8_t>(atoi(row[10]));
			e.level                  = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.deity                  = static_cast<uint32_t>(strtoul(row[12], nullptr, 10));
			e.creation_day           = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.last_spawn             = static_cast<uint32_t>(strtoul(row[14], nullptr, 10));
			e.time_spawned           = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.size                   = strtof(row[16], nullptr);
			e.face                   = static_cast<int32_t>(atoi(row[17]));
			e.hair_color             = static_cast<int32_t>(atoi(row[18]));
			e.hair_style             = static_cast<int32_t>(atoi(row[19]));
			e.beard                  = static_cast<int32_t>(atoi(row[20]));
			e.beard_color            = static_cast<int32_t>(atoi(row[21]));
			e.eye_color_1            = static_cast<int32_t>(atoi(row[22]));
			e.eye_color_2            = static_cast<int32_t>(atoi(row[23]));
			e.drakkin_heritage       = static_cast<int32_t>(atoi(row[24]));
			e.drakkin_tattoo         = static_cast<int32_t>(atoi(row[25]));
			e.drakkin_details        = static_cast<int32_t>(atoi(row[26]));
			e.ac                     = static_cast<int16_t>(atoi(row[27]));
			e.atk                    = static_cast<int32_t>(atoi(row[28]));
			e.hp                     = static_cast<int32_t>(atoi(row[29]));
			e.mana                   = static_cast<int32_t>(atoi(row[30]));
			e.str                    = static_cast<int32_t>(atoi(row[31]));
			e.sta                    = static_cast<int32_t>(atoi(row[32]));
			e.cha                    = static_cast<int32_t>(atoi(row[33]));
			e.dex                    = static_cast<int32_t>(atoi(row[34]));
			e.int_                   = static_cast<int32_t>(atoi(row[35]));
			e.agi                    = static_cast<int32_t>(atoi(row[36]));
			e.wis                    = static_cast<int32_t>(atoi(row[37]));
			e.fire                   = static_cast<int16_t>(atoi(row[38]));
			e.cold                   = static_cast<int16_t>(atoi(row[39]));
			e.magic                  = static_cast<int16_t>(atoi(row[40]));
			e.poison                 = static_cast<int16_t>(atoi(row[41]));
			e.disease                = static_cast<int16_t>(atoi(row[42]));
			e.corruption             = static_cast<int16_t>(atoi(row[43]));
			e.show_helm              = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.follow_distance        = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.stop_melee_level       = static_cast<uint8_t>(strtoul(row[46], nullptr, 10));
			e.expansion_bitmask      = static_cast<int32_t>(atoi(row[47]));
			e.enforce_spell_settings = static_cast<uint8_t>(strtoul(row[48], nullptr, 10));
			e.archery_setting        = static_cast<uint8_t>(strtoul(row[49], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotData &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.owner_id));
		v.push_back(columns[2] + " = " + std::to_string(e.spells_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.last_name) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.title) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.suffix) + "'");
		v.push_back(columns[7] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[8] + " = " + std::to_string(e.gender));
		v.push_back(columns[9] + " = " + std::to_string(e.race));
		v.push_back(columns[10] + " = " + std::to_string(e.class_));
		v.push_back(columns[11] + " = " + std::to_string(e.level));
		v.push_back(columns[12] + " = " + std::to_string(e.deity));
		v.push_back(columns[13] + " = " + std::to_string(e.creation_day));
		v.push_back(columns[14] + " = " + std::to_string(e.last_spawn));
		v.push_back(columns[15] + " = " + std::to_string(e.time_spawned));
		v.push_back(columns[16] + " = " + std::to_string(e.size));
		v.push_back(columns[17] + " = " + std::to_string(e.face));
		v.push_back(columns[18] + " = " + std::to_string(e.hair_color));
		v.push_back(columns[19] + " = " + std::to_string(e.hair_style));
		v.push_back(columns[20] + " = " + std::to_string(e.beard));
		v.push_back(columns[21] + " = " + std::to_string(e.beard_color));
		v.push_back(columns[22] + " = " + std::to_string(e.eye_color_1));
		v.push_back(columns[23] + " = " + std::to_string(e.eye_color_2));
		v.push_back(columns[24] + " = " + std::to_string(e.drakkin_heritage));
		v.push_back(columns[25] + " = " + std::to_string(e.drakkin_tattoo));
		v.push_back(columns[26] + " = " + std::to_string(e.drakkin_details));
		v.push_back(columns[27] + " = " + std::to_string(e.ac));
		v.push_back(columns[28] + " = " + std::to_string(e.atk));
		v.push_back(columns[29] + " = " + std::to_string(e.hp));
		v.push_back(columns[30] + " = " + std::to_string(e.mana));
		v.push_back(columns[31] + " = " + std::to_string(e.str));
		v.push_back(columns[32] + " = " + std::to_string(e.sta));
		v.push_back(columns[33] + " = " + std::to_string(e.cha));
		v.push_back(columns[34] + " = " + std::to_string(e.dex));
		v.push_back(columns[35] + " = " + std::to_string(e.int_));
		v.push_back(columns[36] + " = " + std::to_string(e.agi));
		v.push_back(columns[37] + " = " + std::to_string(e.wis));
		v.push_back(columns[38] + " = " + std::to_string(e.fire));
		v.push_back(columns[39] + " = " + std::to_string(e.cold));
		v.push_back(columns[40] + " = " + std::to_string(e.magic));
		v.push_back(columns[41] + " = " + std::to_string(e.poison));
		v.push_back(columns[42] + " = " + std::to_string(e.disease));
		v.push_back(columns[43] + " = " + std::to_string(e.corruption));
		v.push_back(columns[44] + " = " + std::to_string(e.show_helm));
		v.push_back(columns[45] + " = " + std::to_string(e.follow_distance));
		v.push_back(columns[46] + " = " + std::to_string(e.stop_melee_level));
		v.push_back(columns[47] + " = " + std::to_string(e.expansion_bitmask));
		v.push_back(columns[48] + " = " + std::to_string(e.enforce_spell_settings));
		v.push_back(columns[49] + " = " + std::to_string(e.archery_setting));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.bot_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BotData InsertOne(
		Database& db,
		BotData e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.owner_id));
		v.push_back(std::to_string(e.spells_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.last_name) + "'");
		v.push_back("'" + Strings::Escape(e.title) + "'");
		v.push_back("'" + Strings::Escape(e.suffix) + "'");
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.deity));
		v.push_back(std::to_string(e.creation_day));
		v.push_back(std::to_string(e.last_spawn));
		v.push_back(std::to_string(e.time_spawned));
		v.push_back(std::to_string(e.size));
		v.push_back(std::to_string(e.face));
		v.push_back(std::to_string(e.hair_color));
		v.push_back(std::to_string(e.hair_style));
		v.push_back(std::to_string(e.beard));
		v.push_back(std::to_string(e.beard_color));
		v.push_back(std::to_string(e.eye_color_1));
		v.push_back(std::to_string(e.eye_color_2));
		v.push_back(std::to_string(e.drakkin_heritage));
		v.push_back(std::to_string(e.drakkin_tattoo));
		v.push_back(std::to_string(e.drakkin_details));
		v.push_back(std::to_string(e.ac));
		v.push_back(std::to_string(e.atk));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.str));
		v.push_back(std::to_string(e.sta));
		v.push_back(std::to_string(e.cha));
		v.push_back(std::to_string(e.dex));
		v.push_back(std::to_string(e.int_));
		v.push_back(std::to_string(e.agi));
		v.push_back(std::to_string(e.wis));
		v.push_back(std::to_string(e.fire));
		v.push_back(std::to_string(e.cold));
		v.push_back(std::to_string(e.magic));
		v.push_back(std::to_string(e.poison));
		v.push_back(std::to_string(e.disease));
		v.push_back(std::to_string(e.corruption));
		v.push_back(std::to_string(e.show_helm));
		v.push_back(std::to_string(e.follow_distance));
		v.push_back(std::to_string(e.stop_melee_level));
		v.push_back(std::to_string(e.expansion_bitmask));
		v.push_back(std::to_string(e.enforce_spell_settings));
		v.push_back(std::to_string(e.archery_setting));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.bot_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<BotData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.owner_id));
			v.push_back(std::to_string(e.spells_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.last_name) + "'");
			v.push_back("'" + Strings::Escape(e.title) + "'");
			v.push_back("'" + Strings::Escape(e.suffix) + "'");
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.deity));
			v.push_back(std::to_string(e.creation_day));
			v.push_back(std::to_string(e.last_spawn));
			v.push_back(std::to_string(e.time_spawned));
			v.push_back(std::to_string(e.size));
			v.push_back(std::to_string(e.face));
			v.push_back(std::to_string(e.hair_color));
			v.push_back(std::to_string(e.hair_style));
			v.push_back(std::to_string(e.beard));
			v.push_back(std::to_string(e.beard_color));
			v.push_back(std::to_string(e.eye_color_1));
			v.push_back(std::to_string(e.eye_color_2));
			v.push_back(std::to_string(e.drakkin_heritage));
			v.push_back(std::to_string(e.drakkin_tattoo));
			v.push_back(std::to_string(e.drakkin_details));
			v.push_back(std::to_string(e.ac));
			v.push_back(std::to_string(e.atk));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.str));
			v.push_back(std::to_string(e.sta));
			v.push_back(std::to_string(e.cha));
			v.push_back(std::to_string(e.dex));
			v.push_back(std::to_string(e.int_));
			v.push_back(std::to_string(e.agi));
			v.push_back(std::to_string(e.wis));
			v.push_back(std::to_string(e.fire));
			v.push_back(std::to_string(e.cold));
			v.push_back(std::to_string(e.magic));
			v.push_back(std::to_string(e.poison));
			v.push_back(std::to_string(e.disease));
			v.push_back(std::to_string(e.corruption));
			v.push_back(std::to_string(e.show_helm));
			v.push_back(std::to_string(e.follow_distance));
			v.push_back(std::to_string(e.stop_melee_level));
			v.push_back(std::to_string(e.expansion_bitmask));
			v.push_back(std::to_string(e.enforce_spell_settings));
			v.push_back(std::to_string(e.archery_setting));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<BotData> All(Database& db)
	{
		std::vector<BotData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotData e{};

			e.bot_id                 = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.owner_id               = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spells_id              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.name                   = row[3] ? row[3] : "";
			e.last_name              = row[4] ? row[4] : "";
			e.title                  = row[5] ? row[5] : "";
			e.suffix                 = row[6] ? row[6] : "";
			e.zone_id                = static_cast<int16_t>(atoi(row[7]));
			e.gender                 = static_cast<int8_t>(atoi(row[8]));
			e.race                   = static_cast<int16_t>(atoi(row[9]));
			e.class_                 = static_cast<int8_t>(atoi(row[10]));
			e.level                  = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.deity                  = static_cast<uint32_t>(strtoul(row[12], nullptr, 10));
			e.creation_day           = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.last_spawn             = static_cast<uint32_t>(strtoul(row[14], nullptr, 10));
			e.time_spawned           = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.size                   = strtof(row[16], nullptr);
			e.face                   = static_cast<int32_t>(atoi(row[17]));
			e.hair_color             = static_cast<int32_t>(atoi(row[18]));
			e.hair_style             = static_cast<int32_t>(atoi(row[19]));
			e.beard                  = static_cast<int32_t>(atoi(row[20]));
			e.beard_color            = static_cast<int32_t>(atoi(row[21]));
			e.eye_color_1            = static_cast<int32_t>(atoi(row[22]));
			e.eye_color_2            = static_cast<int32_t>(atoi(row[23]));
			e.drakkin_heritage       = static_cast<int32_t>(atoi(row[24]));
			e.drakkin_tattoo         = static_cast<int32_t>(atoi(row[25]));
			e.drakkin_details        = static_cast<int32_t>(atoi(row[26]));
			e.ac                     = static_cast<int16_t>(atoi(row[27]));
			e.atk                    = static_cast<int32_t>(atoi(row[28]));
			e.hp                     = static_cast<int32_t>(atoi(row[29]));
			e.mana                   = static_cast<int32_t>(atoi(row[30]));
			e.str                    = static_cast<int32_t>(atoi(row[31]));
			e.sta                    = static_cast<int32_t>(atoi(row[32]));
			e.cha                    = static_cast<int32_t>(atoi(row[33]));
			e.dex                    = static_cast<int32_t>(atoi(row[34]));
			e.int_                   = static_cast<int32_t>(atoi(row[35]));
			e.agi                    = static_cast<int32_t>(atoi(row[36]));
			e.wis                    = static_cast<int32_t>(atoi(row[37]));
			e.fire                   = static_cast<int16_t>(atoi(row[38]));
			e.cold                   = static_cast<int16_t>(atoi(row[39]));
			e.magic                  = static_cast<int16_t>(atoi(row[40]));
			e.poison                 = static_cast<int16_t>(atoi(row[41]));
			e.disease                = static_cast<int16_t>(atoi(row[42]));
			e.corruption             = static_cast<int16_t>(atoi(row[43]));
			e.show_helm              = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.follow_distance        = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.stop_melee_level       = static_cast<uint8_t>(strtoul(row[46], nullptr, 10));
			e.expansion_bitmask      = static_cast<int32_t>(atoi(row[47]));
			e.enforce_spell_settings = static_cast<uint8_t>(strtoul(row[48], nullptr, 10));
			e.archery_setting        = static_cast<uint8_t>(strtoul(row[49], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotData> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotData e{};

			e.bot_id                 = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.owner_id               = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.spells_id              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.name                   = row[3] ? row[3] : "";
			e.last_name              = row[4] ? row[4] : "";
			e.title                  = row[5] ? row[5] : "";
			e.suffix                 = row[6] ? row[6] : "";
			e.zone_id                = static_cast<int16_t>(atoi(row[7]));
			e.gender                 = static_cast<int8_t>(atoi(row[8]));
			e.race                   = static_cast<int16_t>(atoi(row[9]));
			e.class_                 = static_cast<int8_t>(atoi(row[10]));
			e.level                  = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.deity                  = static_cast<uint32_t>(strtoul(row[12], nullptr, 10));
			e.creation_day           = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.last_spawn             = static_cast<uint32_t>(strtoul(row[14], nullptr, 10));
			e.time_spawned           = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.size                   = strtof(row[16], nullptr);
			e.face                   = static_cast<int32_t>(atoi(row[17]));
			e.hair_color             = static_cast<int32_t>(atoi(row[18]));
			e.hair_style             = static_cast<int32_t>(atoi(row[19]));
			e.beard                  = static_cast<int32_t>(atoi(row[20]));
			e.beard_color            = static_cast<int32_t>(atoi(row[21]));
			e.eye_color_1            = static_cast<int32_t>(atoi(row[22]));
			e.eye_color_2            = static_cast<int32_t>(atoi(row[23]));
			e.drakkin_heritage       = static_cast<int32_t>(atoi(row[24]));
			e.drakkin_tattoo         = static_cast<int32_t>(atoi(row[25]));
			e.drakkin_details        = static_cast<int32_t>(atoi(row[26]));
			e.ac                     = static_cast<int16_t>(atoi(row[27]));
			e.atk                    = static_cast<int32_t>(atoi(row[28]));
			e.hp                     = static_cast<int32_t>(atoi(row[29]));
			e.mana                   = static_cast<int32_t>(atoi(row[30]));
			e.str                    = static_cast<int32_t>(atoi(row[31]));
			e.sta                    = static_cast<int32_t>(atoi(row[32]));
			e.cha                    = static_cast<int32_t>(atoi(row[33]));
			e.dex                    = static_cast<int32_t>(atoi(row[34]));
			e.int_                   = static_cast<int32_t>(atoi(row[35]));
			e.agi                    = static_cast<int32_t>(atoi(row[36]));
			e.wis                    = static_cast<int32_t>(atoi(row[37]));
			e.fire                   = static_cast<int16_t>(atoi(row[38]));
			e.cold                   = static_cast<int16_t>(atoi(row[39]));
			e.magic                  = static_cast<int16_t>(atoi(row[40]));
			e.poison                 = static_cast<int16_t>(atoi(row[41]));
			e.disease                = static_cast<int16_t>(atoi(row[42]));
			e.corruption             = static_cast<int16_t>(atoi(row[43]));
			e.show_helm              = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.follow_distance        = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.stop_melee_level       = static_cast<uint8_t>(strtoul(row[46], nullptr, 10));
			e.expansion_bitmask      = static_cast<int32_t>(atoi(row[47]));
			e.enforce_spell_settings = static_cast<uint8_t>(strtoul(row[48], nullptr, 10));
			e.archery_setting        = static_cast<uint8_t>(strtoul(row[49], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_BOT_DATA_REPOSITORY_H
