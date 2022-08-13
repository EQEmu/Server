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

#ifndef EQEMU_BASE_CHARACTER_CORPSES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_CORPSES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterCorpsesRepository {
public:
	struct CharacterCorpses {
		int         id;
		int         charid;
		std::string charname;
		int         zone_id;
		int         instance_id;
		float       x;
		float       y;
		float       z;
		float       heading;
		time_t      time_of_death;
		int         guild_consent_id;
		int         is_rezzed;
		int         is_buried;
		int         was_at_graveyard;
		int         is_locked;
		int         exp;
		int         size;
		int         level;
		int         race;
		int         gender;
		int         class_;
		int         deity;
		int         texture;
		int         helm_texture;
		int         copper;
		int         silver;
		int         gold;
		int         platinum;
		int         hair_color;
		int         beard_color;
		int         eye_color_1;
		int         eye_color_2;
		int         hair_style;
		int         face;
		int         beard;
		int         drakkin_heritage;
		int         drakkin_tattoo;
		int         drakkin_details;
		int         wc_1;
		int         wc_2;
		int         wc_3;
		int         wc_4;
		int         wc_5;
		int         wc_6;
		int         wc_7;
		int         wc_8;
		int         wc_9;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"charid",
			"charname",
			"zone_id",
			"instance_id",
			"x",
			"y",
			"z",
			"heading",
			"time_of_death",
			"guild_consent_id",
			"is_rezzed",
			"is_buried",
			"was_at_graveyard",
			"is_locked",
			"exp",
			"size",
			"level",
			"race",
			"gender",
			"`class`",
			"deity",
			"texture",
			"helm_texture",
			"copper",
			"silver",
			"gold",
			"platinum",
			"hair_color",
			"beard_color",
			"eye_color_1",
			"eye_color_2",
			"hair_style",
			"face",
			"beard",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"wc_1",
			"wc_2",
			"wc_3",
			"wc_4",
			"wc_5",
			"wc_6",
			"wc_7",
			"wc_8",
			"wc_9",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"charid",
			"charname",
			"zone_id",
			"instance_id",
			"x",
			"y",
			"z",
			"heading",
			"UNIX_TIMESTAMP(time_of_death)",
			"guild_consent_id",
			"is_rezzed",
			"is_buried",
			"was_at_graveyard",
			"is_locked",
			"exp",
			"size",
			"level",
			"race",
			"gender",
			"`class`",
			"deity",
			"texture",
			"helm_texture",
			"copper",
			"silver",
			"gold",
			"platinum",
			"hair_color",
			"beard_color",
			"eye_color_1",
			"eye_color_2",
			"hair_style",
			"face",
			"beard",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"wc_1",
			"wc_2",
			"wc_3",
			"wc_4",
			"wc_5",
			"wc_6",
			"wc_7",
			"wc_8",
			"wc_9",
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
		return std::string("character_corpses");
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

	static CharacterCorpses NewEntity()
	{
		CharacterCorpses e{};

		e.id               = 0;
		e.charid           = 0;
		e.charname         = "";
		e.zone_id          = 0;
		e.instance_id      = 0;
		e.x                = 0;
		e.y                = 0;
		e.z                = 0;
		e.heading          = 0;
		e.time_of_death    = 0;
		e.guild_consent_id = 0;
		e.is_rezzed        = 0;
		e.is_buried        = 0;
		e.was_at_graveyard = 0;
		e.is_locked        = 0;
		e.exp              = 0;
		e.size             = 0;
		e.level            = 0;
		e.race             = 0;
		e.gender           = 0;
		e.class_           = 0;
		e.deity            = 0;
		e.texture          = 0;
		e.helm_texture     = 0;
		e.copper           = 0;
		e.silver           = 0;
		e.gold             = 0;
		e.platinum         = 0;
		e.hair_color       = 0;
		e.beard_color      = 0;
		e.eye_color_1      = 0;
		e.eye_color_2      = 0;
		e.hair_style       = 0;
		e.face             = 0;
		e.beard            = 0;
		e.drakkin_heritage = 0;
		e.drakkin_tattoo   = 0;
		e.drakkin_details  = 0;
		e.wc_1             = 0;
		e.wc_2             = 0;
		e.wc_3             = 0;
		e.wc_4             = 0;
		e.wc_5             = 0;
		e.wc_6             = 0;
		e.wc_7             = 0;
		e.wc_8             = 0;
		e.wc_9             = 0;

		return e;
	}

	static CharacterCorpses GetCharacterCorpsese(
		const std::vector<CharacterCorpses> &character_corpsess,
		int character_corpses_id
	)
	{
		for (auto &character_corpses : character_corpsess) {
			if (character_corpses.id == character_corpses_id) {
				return character_corpses;
			}
		}

		return NewEntity();
	}

	static CharacterCorpses FindOne(
		Database& db,
		int character_corpses_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_corpses_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterCorpses e{};

			e.id               = atoi(row[0]);
			e.charid           = atoi(row[1]);
			e.charname         = row[2] ? row[2] : "";
			e.zone_id          = atoi(row[3]);
			e.instance_id      = atoi(row[4]);
			e.x                = static_cast<float>(atof(row[5]));
			e.y                = static_cast<float>(atof(row[6]));
			e.z                = static_cast<float>(atof(row[7]));
			e.heading          = static_cast<float>(atof(row[8]));
			e.time_of_death    = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.guild_consent_id = atoi(row[10]);
			e.is_rezzed        = atoi(row[11]);
			e.is_buried        = atoi(row[12]);
			e.was_at_graveyard = atoi(row[13]);
			e.is_locked        = atoi(row[14]);
			e.exp              = atoi(row[15]);
			e.size             = atoi(row[16]);
			e.level            = atoi(row[17]);
			e.race             = atoi(row[18]);
			e.gender           = atoi(row[19]);
			e.class_           = atoi(row[20]);
			e.deity            = atoi(row[21]);
			e.texture          = atoi(row[22]);
			e.helm_texture     = atoi(row[23]);
			e.copper           = atoi(row[24]);
			e.silver           = atoi(row[25]);
			e.gold             = atoi(row[26]);
			e.platinum         = atoi(row[27]);
			e.hair_color       = atoi(row[28]);
			e.beard_color      = atoi(row[29]);
			e.eye_color_1      = atoi(row[30]);
			e.eye_color_2      = atoi(row[31]);
			e.hair_style       = atoi(row[32]);
			e.face             = atoi(row[33]);
			e.beard            = atoi(row[34]);
			e.drakkin_heritage = atoi(row[35]);
			e.drakkin_tattoo   = atoi(row[36]);
			e.drakkin_details  = atoi(row[37]);
			e.wc_1             = atoi(row[38]);
			e.wc_2             = atoi(row[39]);
			e.wc_3             = atoi(row[40]);
			e.wc_4             = atoi(row[41]);
			e.wc_5             = atoi(row[42]);
			e.wc_6             = atoi(row[43]);
			e.wc_7             = atoi(row[44]);
			e.wc_8             = atoi(row[45]);
			e.wc_9             = atoi(row[46]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_corpses_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_corpses_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterCorpses e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(e.charid));
		update_values.push_back(columns[2] + " = '" + Strings::Escape(e.charname) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(e.zone_id));
		update_values.push_back(columns[4] + " = " + std::to_string(e.instance_id));
		update_values.push_back(columns[5] + " = " + std::to_string(e.x));
		update_values.push_back(columns[6] + " = " + std::to_string(e.y));
		update_values.push_back(columns[7] + " = " + std::to_string(e.z));
		update_values.push_back(columns[8] + " = " + std::to_string(e.heading));
		update_values.push_back(columns[9] + " = FROM_UNIXTIME(" + (e.time_of_death > 0 ? std::to_string(e.time_of_death) : "null") + ")");
		update_values.push_back(columns[10] + " = " + std::to_string(e.guild_consent_id));
		update_values.push_back(columns[11] + " = " + std::to_string(e.is_rezzed));
		update_values.push_back(columns[12] + " = " + std::to_string(e.is_buried));
		update_values.push_back(columns[13] + " = " + std::to_string(e.was_at_graveyard));
		update_values.push_back(columns[14] + " = " + std::to_string(e.is_locked));
		update_values.push_back(columns[15] + " = " + std::to_string(e.exp));
		update_values.push_back(columns[16] + " = " + std::to_string(e.size));
		update_values.push_back(columns[17] + " = " + std::to_string(e.level));
		update_values.push_back(columns[18] + " = " + std::to_string(e.race));
		update_values.push_back(columns[19] + " = " + std::to_string(e.gender));
		update_values.push_back(columns[20] + " = " + std::to_string(e.class_));
		update_values.push_back(columns[21] + " = " + std::to_string(e.deity));
		update_values.push_back(columns[22] + " = " + std::to_string(e.texture));
		update_values.push_back(columns[23] + " = " + std::to_string(e.helm_texture));
		update_values.push_back(columns[24] + " = " + std::to_string(e.copper));
		update_values.push_back(columns[25] + " = " + std::to_string(e.silver));
		update_values.push_back(columns[26] + " = " + std::to_string(e.gold));
		update_values.push_back(columns[27] + " = " + std::to_string(e.platinum));
		update_values.push_back(columns[28] + " = " + std::to_string(e.hair_color));
		update_values.push_back(columns[29] + " = " + std::to_string(e.beard_color));
		update_values.push_back(columns[30] + " = " + std::to_string(e.eye_color_1));
		update_values.push_back(columns[31] + " = " + std::to_string(e.eye_color_2));
		update_values.push_back(columns[32] + " = " + std::to_string(e.hair_style));
		update_values.push_back(columns[33] + " = " + std::to_string(e.face));
		update_values.push_back(columns[34] + " = " + std::to_string(e.beard));
		update_values.push_back(columns[35] + " = " + std::to_string(e.drakkin_heritage));
		update_values.push_back(columns[36] + " = " + std::to_string(e.drakkin_tattoo));
		update_values.push_back(columns[37] + " = " + std::to_string(e.drakkin_details));
		update_values.push_back(columns[38] + " = " + std::to_string(e.wc_1));
		update_values.push_back(columns[39] + " = " + std::to_string(e.wc_2));
		update_values.push_back(columns[40] + " = " + std::to_string(e.wc_3));
		update_values.push_back(columns[41] + " = " + std::to_string(e.wc_4));
		update_values.push_back(columns[42] + " = " + std::to_string(e.wc_5));
		update_values.push_back(columns[43] + " = " + std::to_string(e.wc_6));
		update_values.push_back(columns[44] + " = " + std::to_string(e.wc_7));
		update_values.push_back(columns[45] + " = " + std::to_string(e.wc_8));
		update_values.push_back(columns[46] + " = " + std::to_string(e.wc_9));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterCorpses InsertOne(
		Database& db,
		CharacterCorpses e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(e.id));
		insert_values.push_back(std::to_string(e.charid));
		insert_values.push_back("'" + Strings::Escape(e.charname) + "'");
		insert_values.push_back(std::to_string(e.zone_id));
		insert_values.push_back(std::to_string(e.instance_id));
		insert_values.push_back(std::to_string(e.x));
		insert_values.push_back(std::to_string(e.y));
		insert_values.push_back(std::to_string(e.z));
		insert_values.push_back(std::to_string(e.heading));
		insert_values.push_back("FROM_UNIXTIME(" + (e.time_of_death > 0 ? std::to_string(e.time_of_death) : "null") + ")");
		insert_values.push_back(std::to_string(e.guild_consent_id));
		insert_values.push_back(std::to_string(e.is_rezzed));
		insert_values.push_back(std::to_string(e.is_buried));
		insert_values.push_back(std::to_string(e.was_at_graveyard));
		insert_values.push_back(std::to_string(e.is_locked));
		insert_values.push_back(std::to_string(e.exp));
		insert_values.push_back(std::to_string(e.size));
		insert_values.push_back(std::to_string(e.level));
		insert_values.push_back(std::to_string(e.race));
		insert_values.push_back(std::to_string(e.gender));
		insert_values.push_back(std::to_string(e.class_));
		insert_values.push_back(std::to_string(e.deity));
		insert_values.push_back(std::to_string(e.texture));
		insert_values.push_back(std::to_string(e.helm_texture));
		insert_values.push_back(std::to_string(e.copper));
		insert_values.push_back(std::to_string(e.silver));
		insert_values.push_back(std::to_string(e.gold));
		insert_values.push_back(std::to_string(e.platinum));
		insert_values.push_back(std::to_string(e.hair_color));
		insert_values.push_back(std::to_string(e.beard_color));
		insert_values.push_back(std::to_string(e.eye_color_1));
		insert_values.push_back(std::to_string(e.eye_color_2));
		insert_values.push_back(std::to_string(e.hair_style));
		insert_values.push_back(std::to_string(e.face));
		insert_values.push_back(std::to_string(e.beard));
		insert_values.push_back(std::to_string(e.drakkin_heritage));
		insert_values.push_back(std::to_string(e.drakkin_tattoo));
		insert_values.push_back(std::to_string(e.drakkin_details));
		insert_values.push_back(std::to_string(e.wc_1));
		insert_values.push_back(std::to_string(e.wc_2));
		insert_values.push_back(std::to_string(e.wc_3));
		insert_values.push_back(std::to_string(e.wc_4));
		insert_values.push_back(std::to_string(e.wc_5));
		insert_values.push_back(std::to_string(e.wc_6));
		insert_values.push_back(std::to_string(e.wc_7));
		insert_values.push_back(std::to_string(e.wc_8));
		insert_values.push_back(std::to_string(e.wc_9));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterCorpses> entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(e.id));
			insert_values.push_back(std::to_string(e.charid));
			insert_values.push_back("'" + Strings::Escape(e.charname) + "'");
			insert_values.push_back(std::to_string(e.zone_id));
			insert_values.push_back(std::to_string(e.instance_id));
			insert_values.push_back(std::to_string(e.x));
			insert_values.push_back(std::to_string(e.y));
			insert_values.push_back(std::to_string(e.z));
			insert_values.push_back(std::to_string(e.heading));
			insert_values.push_back("FROM_UNIXTIME(" + (e.time_of_death > 0 ? std::to_string(e.time_of_death) : "null") + ")");
			insert_values.push_back(std::to_string(e.guild_consent_id));
			insert_values.push_back(std::to_string(e.is_rezzed));
			insert_values.push_back(std::to_string(e.is_buried));
			insert_values.push_back(std::to_string(e.was_at_graveyard));
			insert_values.push_back(std::to_string(e.is_locked));
			insert_values.push_back(std::to_string(e.exp));
			insert_values.push_back(std::to_string(e.size));
			insert_values.push_back(std::to_string(e.level));
			insert_values.push_back(std::to_string(e.race));
			insert_values.push_back(std::to_string(e.gender));
			insert_values.push_back(std::to_string(e.class_));
			insert_values.push_back(std::to_string(e.deity));
			insert_values.push_back(std::to_string(e.texture));
			insert_values.push_back(std::to_string(e.helm_texture));
			insert_values.push_back(std::to_string(e.copper));
			insert_values.push_back(std::to_string(e.silver));
			insert_values.push_back(std::to_string(e.gold));
			insert_values.push_back(std::to_string(e.platinum));
			insert_values.push_back(std::to_string(e.hair_color));
			insert_values.push_back(std::to_string(e.beard_color));
			insert_values.push_back(std::to_string(e.eye_color_1));
			insert_values.push_back(std::to_string(e.eye_color_2));
			insert_values.push_back(std::to_string(e.hair_style));
			insert_values.push_back(std::to_string(e.face));
			insert_values.push_back(std::to_string(e.beard));
			insert_values.push_back(std::to_string(e.drakkin_heritage));
			insert_values.push_back(std::to_string(e.drakkin_tattoo));
			insert_values.push_back(std::to_string(e.drakkin_details));
			insert_values.push_back(std::to_string(e.wc_1));
			insert_values.push_back(std::to_string(e.wc_2));
			insert_values.push_back(std::to_string(e.wc_3));
			insert_values.push_back(std::to_string(e.wc_4));
			insert_values.push_back(std::to_string(e.wc_5));
			insert_values.push_back(std::to_string(e.wc_6));
			insert_values.push_back(std::to_string(e.wc_7));
			insert_values.push_back(std::to_string(e.wc_8));
			insert_values.push_back(std::to_string(e.wc_9));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<CharacterCorpses> All(Database& db)
	{
		std::vector<CharacterCorpses> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterCorpses e{};

			e.id               = atoi(row[0]);
			e.charid           = atoi(row[1]);
			e.charname         = row[2] ? row[2] : "";
			e.zone_id          = atoi(row[3]);
			e.instance_id      = atoi(row[4]);
			e.x                = static_cast<float>(atof(row[5]));
			e.y                = static_cast<float>(atof(row[6]));
			e.z                = static_cast<float>(atof(row[7]));
			e.heading          = static_cast<float>(atof(row[8]));
			e.time_of_death    = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.guild_consent_id = atoi(row[10]);
			e.is_rezzed        = atoi(row[11]);
			e.is_buried        = atoi(row[12]);
			e.was_at_graveyard = atoi(row[13]);
			e.is_locked        = atoi(row[14]);
			e.exp              = atoi(row[15]);
			e.size             = atoi(row[16]);
			e.level            = atoi(row[17]);
			e.race             = atoi(row[18]);
			e.gender           = atoi(row[19]);
			e.class_           = atoi(row[20]);
			e.deity            = atoi(row[21]);
			e.texture          = atoi(row[22]);
			e.helm_texture     = atoi(row[23]);
			e.copper           = atoi(row[24]);
			e.silver           = atoi(row[25]);
			e.gold             = atoi(row[26]);
			e.platinum         = atoi(row[27]);
			e.hair_color       = atoi(row[28]);
			e.beard_color      = atoi(row[29]);
			e.eye_color_1      = atoi(row[30]);
			e.eye_color_2      = atoi(row[31]);
			e.hair_style       = atoi(row[32]);
			e.face             = atoi(row[33]);
			e.beard            = atoi(row[34]);
			e.drakkin_heritage = atoi(row[35]);
			e.drakkin_tattoo   = atoi(row[36]);
			e.drakkin_details  = atoi(row[37]);
			e.wc_1             = atoi(row[38]);
			e.wc_2             = atoi(row[39]);
			e.wc_3             = atoi(row[40]);
			e.wc_4             = atoi(row[41]);
			e.wc_5             = atoi(row[42]);
			e.wc_6             = atoi(row[43]);
			e.wc_7             = atoi(row[44]);
			e.wc_8             = atoi(row[45]);
			e.wc_9             = atoi(row[46]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterCorpses> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterCorpses> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterCorpses e{};

			e.id               = atoi(row[0]);
			e.charid           = atoi(row[1]);
			e.charname         = row[2] ? row[2] : "";
			e.zone_id          = atoi(row[3]);
			e.instance_id      = atoi(row[4]);
			e.x                = static_cast<float>(atof(row[5]));
			e.y                = static_cast<float>(atof(row[6]));
			e.z                = static_cast<float>(atof(row[7]));
			e.heading          = static_cast<float>(atof(row[8]));
			e.time_of_death    = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.guild_consent_id = atoi(row[10]);
			e.is_rezzed        = atoi(row[11]);
			e.is_buried        = atoi(row[12]);
			e.was_at_graveyard = atoi(row[13]);
			e.is_locked        = atoi(row[14]);
			e.exp              = atoi(row[15]);
			e.size             = atoi(row[16]);
			e.level            = atoi(row[17]);
			e.race             = atoi(row[18]);
			e.gender           = atoi(row[19]);
			e.class_           = atoi(row[20]);
			e.deity            = atoi(row[21]);
			e.texture          = atoi(row[22]);
			e.helm_texture     = atoi(row[23]);
			e.copper           = atoi(row[24]);
			e.silver           = atoi(row[25]);
			e.gold             = atoi(row[26]);
			e.platinum         = atoi(row[27]);
			e.hair_color       = atoi(row[28]);
			e.beard_color      = atoi(row[29]);
			e.eye_color_1      = atoi(row[30]);
			e.eye_color_2      = atoi(row[31]);
			e.hair_style       = atoi(row[32]);
			e.face             = atoi(row[33]);
			e.beard            = atoi(row[34]);
			e.drakkin_heritage = atoi(row[35]);
			e.drakkin_tattoo   = atoi(row[36]);
			e.drakkin_details  = atoi(row[37]);
			e.wc_1             = atoi(row[38]);
			e.wc_2             = atoi(row[39]);
			e.wc_3             = atoi(row[40]);
			e.wc_4             = atoi(row[41]);
			e.wc_5             = atoi(row[42]);
			e.wc_6             = atoi(row[43]);
			e.wc_7             = atoi(row[44]);
			e.wc_8             = atoi(row[45]);
			e.wc_9             = atoi(row[46]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, std::string where_filter)
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
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_CHARACTER_CORPSES_REPOSITORY_H
