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
#include "../../string_util.h"

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
		std::string time_of_death;
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
		int         class;
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
			"class",
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
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_corpses");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			ColumnsRaw(),
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
		CharacterCorpses entry{};

		entry.id               = 0;
		entry.charid           = 0;
		entry.charname         = "";
		entry.zone_id          = 0;
		entry.instance_id      = 0;
		entry.x                = 0;
		entry.y                = 0;
		entry.z                = 0;
		entry.heading          = 0;
		entry.time_of_death    = "0000-00-00 00:00:00";
		entry.guild_consent_id = 0;
		entry.is_rezzed        = 0;
		entry.is_buried        = 0;
		entry.was_at_graveyard = 0;
		entry.is_locked        = 0;
		entry.exp              = 0;
		entry.size             = 0;
		entry.level            = 0;
		entry.race             = 0;
		entry.gender           = 0;
		entry.class            = 0;
		entry.deity            = 0;
		entry.texture          = 0;
		entry.helm_texture     = 0;
		entry.copper           = 0;
		entry.silver           = 0;
		entry.gold             = 0;
		entry.platinum         = 0;
		entry.hair_color       = 0;
		entry.beard_color      = 0;
		entry.eye_color_1      = 0;
		entry.eye_color_2      = 0;
		entry.hair_style       = 0;
		entry.face             = 0;
		entry.beard            = 0;
		entry.drakkin_heritage = 0;
		entry.drakkin_tattoo   = 0;
		entry.drakkin_details  = 0;
		entry.wc_1             = 0;
		entry.wc_2             = 0;
		entry.wc_3             = 0;
		entry.wc_4             = 0;
		entry.wc_5             = 0;
		entry.wc_6             = 0;
		entry.wc_7             = 0;
		entry.wc_8             = 0;
		entry.wc_9             = 0;

		return entry;
	}

	static CharacterCorpses GetCharacterCorpsesEntry(
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
			CharacterCorpses entry{};

			entry.id               = atoi(row[0]);
			entry.charid           = atoi(row[1]);
			entry.charname         = row[2] ? row[2] : "";
			entry.zone_id          = atoi(row[3]);
			entry.instance_id      = atoi(row[4]);
			entry.x                = static_cast<float>(atof(row[5]));
			entry.y                = static_cast<float>(atof(row[6]));
			entry.z                = static_cast<float>(atof(row[7]));
			entry.heading          = static_cast<float>(atof(row[8]));
			entry.time_of_death    = row[9] ? row[9] : "";
			entry.guild_consent_id = atoi(row[10]);
			entry.is_rezzed        = atoi(row[11]);
			entry.is_buried        = atoi(row[12]);
			entry.was_at_graveyard = atoi(row[13]);
			entry.is_locked        = atoi(row[14]);
			entry.exp              = atoi(row[15]);
			entry.size             = atoi(row[16]);
			entry.level            = atoi(row[17]);
			entry.race             = atoi(row[18]);
			entry.gender           = atoi(row[19]);
			entry.class            = atoi(row[20]);
			entry.deity            = atoi(row[21]);
			entry.texture          = atoi(row[22]);
			entry.helm_texture     = atoi(row[23]);
			entry.copper           = atoi(row[24]);
			entry.silver           = atoi(row[25]);
			entry.gold             = atoi(row[26]);
			entry.platinum         = atoi(row[27]);
			entry.hair_color       = atoi(row[28]);
			entry.beard_color      = atoi(row[29]);
			entry.eye_color_1      = atoi(row[30]);
			entry.eye_color_2      = atoi(row[31]);
			entry.hair_style       = atoi(row[32]);
			entry.face             = atoi(row[33]);
			entry.beard            = atoi(row[34]);
			entry.drakkin_heritage = atoi(row[35]);
			entry.drakkin_tattoo   = atoi(row[36]);
			entry.drakkin_details  = atoi(row[37]);
			entry.wc_1             = atoi(row[38]);
			entry.wc_2             = atoi(row[39]);
			entry.wc_3             = atoi(row[40]);
			entry.wc_4             = atoi(row[41]);
			entry.wc_5             = atoi(row[42]);
			entry.wc_6             = atoi(row[43]);
			entry.wc_7             = atoi(row[44]);
			entry.wc_8             = atoi(row[45]);
			entry.wc_9             = atoi(row[46]);

			return entry;
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
		CharacterCorpses character_corpses_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(character_corpses_entry.charid));
		update_values.push_back(columns[2] + " = '" + EscapeString(character_corpses_entry.charname) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(character_corpses_entry.zone_id));
		update_values.push_back(columns[4] + " = " + std::to_string(character_corpses_entry.instance_id));
		update_values.push_back(columns[5] + " = " + std::to_string(character_corpses_entry.x));
		update_values.push_back(columns[6] + " = " + std::to_string(character_corpses_entry.y));
		update_values.push_back(columns[7] + " = " + std::to_string(character_corpses_entry.z));
		update_values.push_back(columns[8] + " = " + std::to_string(character_corpses_entry.heading));
		update_values.push_back(columns[9] + " = '" + EscapeString(character_corpses_entry.time_of_death) + "'");
		update_values.push_back(columns[10] + " = " + std::to_string(character_corpses_entry.guild_consent_id));
		update_values.push_back(columns[11] + " = " + std::to_string(character_corpses_entry.is_rezzed));
		update_values.push_back(columns[12] + " = " + std::to_string(character_corpses_entry.is_buried));
		update_values.push_back(columns[13] + " = " + std::to_string(character_corpses_entry.was_at_graveyard));
		update_values.push_back(columns[14] + " = " + std::to_string(character_corpses_entry.is_locked));
		update_values.push_back(columns[15] + " = " + std::to_string(character_corpses_entry.exp));
		update_values.push_back(columns[16] + " = " + std::to_string(character_corpses_entry.size));
		update_values.push_back(columns[17] + " = " + std::to_string(character_corpses_entry.level));
		update_values.push_back(columns[18] + " = " + std::to_string(character_corpses_entry.race));
		update_values.push_back(columns[19] + " = " + std::to_string(character_corpses_entry.gender));
		update_values.push_back(columns[20] + " = " + std::to_string(character_corpses_entry.class));
		update_values.push_back(columns[21] + " = " + std::to_string(character_corpses_entry.deity));
		update_values.push_back(columns[22] + " = " + std::to_string(character_corpses_entry.texture));
		update_values.push_back(columns[23] + " = " + std::to_string(character_corpses_entry.helm_texture));
		update_values.push_back(columns[24] + " = " + std::to_string(character_corpses_entry.copper));
		update_values.push_back(columns[25] + " = " + std::to_string(character_corpses_entry.silver));
		update_values.push_back(columns[26] + " = " + std::to_string(character_corpses_entry.gold));
		update_values.push_back(columns[27] + " = " + std::to_string(character_corpses_entry.platinum));
		update_values.push_back(columns[28] + " = " + std::to_string(character_corpses_entry.hair_color));
		update_values.push_back(columns[29] + " = " + std::to_string(character_corpses_entry.beard_color));
		update_values.push_back(columns[30] + " = " + std::to_string(character_corpses_entry.eye_color_1));
		update_values.push_back(columns[31] + " = " + std::to_string(character_corpses_entry.eye_color_2));
		update_values.push_back(columns[32] + " = " + std::to_string(character_corpses_entry.hair_style));
		update_values.push_back(columns[33] + " = " + std::to_string(character_corpses_entry.face));
		update_values.push_back(columns[34] + " = " + std::to_string(character_corpses_entry.beard));
		update_values.push_back(columns[35] + " = " + std::to_string(character_corpses_entry.drakkin_heritage));
		update_values.push_back(columns[36] + " = " + std::to_string(character_corpses_entry.drakkin_tattoo));
		update_values.push_back(columns[37] + " = " + std::to_string(character_corpses_entry.drakkin_details));
		update_values.push_back(columns[38] + " = " + std::to_string(character_corpses_entry.wc_1));
		update_values.push_back(columns[39] + " = " + std::to_string(character_corpses_entry.wc_2));
		update_values.push_back(columns[40] + " = " + std::to_string(character_corpses_entry.wc_3));
		update_values.push_back(columns[41] + " = " + std::to_string(character_corpses_entry.wc_4));
		update_values.push_back(columns[42] + " = " + std::to_string(character_corpses_entry.wc_5));
		update_values.push_back(columns[43] + " = " + std::to_string(character_corpses_entry.wc_6));
		update_values.push_back(columns[44] + " = " + std::to_string(character_corpses_entry.wc_7));
		update_values.push_back(columns[45] + " = " + std::to_string(character_corpses_entry.wc_8));
		update_values.push_back(columns[46] + " = " + std::to_string(character_corpses_entry.wc_9));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_corpses_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterCorpses InsertOne(
		Database& db,
		CharacterCorpses character_corpses_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_corpses_entry.id));
		insert_values.push_back(std::to_string(character_corpses_entry.charid));
		insert_values.push_back("'" + EscapeString(character_corpses_entry.charname) + "'");
		insert_values.push_back(std::to_string(character_corpses_entry.zone_id));
		insert_values.push_back(std::to_string(character_corpses_entry.instance_id));
		insert_values.push_back(std::to_string(character_corpses_entry.x));
		insert_values.push_back(std::to_string(character_corpses_entry.y));
		insert_values.push_back(std::to_string(character_corpses_entry.z));
		insert_values.push_back(std::to_string(character_corpses_entry.heading));
		insert_values.push_back("'" + EscapeString(character_corpses_entry.time_of_death) + "'");
		insert_values.push_back(std::to_string(character_corpses_entry.guild_consent_id));
		insert_values.push_back(std::to_string(character_corpses_entry.is_rezzed));
		insert_values.push_back(std::to_string(character_corpses_entry.is_buried));
		insert_values.push_back(std::to_string(character_corpses_entry.was_at_graveyard));
		insert_values.push_back(std::to_string(character_corpses_entry.is_locked));
		insert_values.push_back(std::to_string(character_corpses_entry.exp));
		insert_values.push_back(std::to_string(character_corpses_entry.size));
		insert_values.push_back(std::to_string(character_corpses_entry.level));
		insert_values.push_back(std::to_string(character_corpses_entry.race));
		insert_values.push_back(std::to_string(character_corpses_entry.gender));
		insert_values.push_back(std::to_string(character_corpses_entry.class));
		insert_values.push_back(std::to_string(character_corpses_entry.deity));
		insert_values.push_back(std::to_string(character_corpses_entry.texture));
		insert_values.push_back(std::to_string(character_corpses_entry.helm_texture));
		insert_values.push_back(std::to_string(character_corpses_entry.copper));
		insert_values.push_back(std::to_string(character_corpses_entry.silver));
		insert_values.push_back(std::to_string(character_corpses_entry.gold));
		insert_values.push_back(std::to_string(character_corpses_entry.platinum));
		insert_values.push_back(std::to_string(character_corpses_entry.hair_color));
		insert_values.push_back(std::to_string(character_corpses_entry.beard_color));
		insert_values.push_back(std::to_string(character_corpses_entry.eye_color_1));
		insert_values.push_back(std::to_string(character_corpses_entry.eye_color_2));
		insert_values.push_back(std::to_string(character_corpses_entry.hair_style));
		insert_values.push_back(std::to_string(character_corpses_entry.face));
		insert_values.push_back(std::to_string(character_corpses_entry.beard));
		insert_values.push_back(std::to_string(character_corpses_entry.drakkin_heritage));
		insert_values.push_back(std::to_string(character_corpses_entry.drakkin_tattoo));
		insert_values.push_back(std::to_string(character_corpses_entry.drakkin_details));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_1));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_2));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_3));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_4));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_5));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_6));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_7));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_8));
		insert_values.push_back(std::to_string(character_corpses_entry.wc_9));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_corpses_entry.id = results.LastInsertedID();
			return character_corpses_entry;
		}

		character_corpses_entry = NewEntity();

		return character_corpses_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterCorpses> character_corpses_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_corpses_entry: character_corpses_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_corpses_entry.id));
			insert_values.push_back(std::to_string(character_corpses_entry.charid));
			insert_values.push_back("'" + EscapeString(character_corpses_entry.charname) + "'");
			insert_values.push_back(std::to_string(character_corpses_entry.zone_id));
			insert_values.push_back(std::to_string(character_corpses_entry.instance_id));
			insert_values.push_back(std::to_string(character_corpses_entry.x));
			insert_values.push_back(std::to_string(character_corpses_entry.y));
			insert_values.push_back(std::to_string(character_corpses_entry.z));
			insert_values.push_back(std::to_string(character_corpses_entry.heading));
			insert_values.push_back("'" + EscapeString(character_corpses_entry.time_of_death) + "'");
			insert_values.push_back(std::to_string(character_corpses_entry.guild_consent_id));
			insert_values.push_back(std::to_string(character_corpses_entry.is_rezzed));
			insert_values.push_back(std::to_string(character_corpses_entry.is_buried));
			insert_values.push_back(std::to_string(character_corpses_entry.was_at_graveyard));
			insert_values.push_back(std::to_string(character_corpses_entry.is_locked));
			insert_values.push_back(std::to_string(character_corpses_entry.exp));
			insert_values.push_back(std::to_string(character_corpses_entry.size));
			insert_values.push_back(std::to_string(character_corpses_entry.level));
			insert_values.push_back(std::to_string(character_corpses_entry.race));
			insert_values.push_back(std::to_string(character_corpses_entry.gender));
			insert_values.push_back(std::to_string(character_corpses_entry.class));
			insert_values.push_back(std::to_string(character_corpses_entry.deity));
			insert_values.push_back(std::to_string(character_corpses_entry.texture));
			insert_values.push_back(std::to_string(character_corpses_entry.helm_texture));
			insert_values.push_back(std::to_string(character_corpses_entry.copper));
			insert_values.push_back(std::to_string(character_corpses_entry.silver));
			insert_values.push_back(std::to_string(character_corpses_entry.gold));
			insert_values.push_back(std::to_string(character_corpses_entry.platinum));
			insert_values.push_back(std::to_string(character_corpses_entry.hair_color));
			insert_values.push_back(std::to_string(character_corpses_entry.beard_color));
			insert_values.push_back(std::to_string(character_corpses_entry.eye_color_1));
			insert_values.push_back(std::to_string(character_corpses_entry.eye_color_2));
			insert_values.push_back(std::to_string(character_corpses_entry.hair_style));
			insert_values.push_back(std::to_string(character_corpses_entry.face));
			insert_values.push_back(std::to_string(character_corpses_entry.beard));
			insert_values.push_back(std::to_string(character_corpses_entry.drakkin_heritage));
			insert_values.push_back(std::to_string(character_corpses_entry.drakkin_tattoo));
			insert_values.push_back(std::to_string(character_corpses_entry.drakkin_details));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_1));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_2));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_3));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_4));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_5));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_6));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_7));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_8));
			insert_values.push_back(std::to_string(character_corpses_entry.wc_9));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
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
			CharacterCorpses entry{};

			entry.id               = atoi(row[0]);
			entry.charid           = atoi(row[1]);
			entry.charname         = row[2] ? row[2] : "";
			entry.zone_id          = atoi(row[3]);
			entry.instance_id      = atoi(row[4]);
			entry.x                = static_cast<float>(atof(row[5]));
			entry.y                = static_cast<float>(atof(row[6]));
			entry.z                = static_cast<float>(atof(row[7]));
			entry.heading          = static_cast<float>(atof(row[8]));
			entry.time_of_death    = row[9] ? row[9] : "";
			entry.guild_consent_id = atoi(row[10]);
			entry.is_rezzed        = atoi(row[11]);
			entry.is_buried        = atoi(row[12]);
			entry.was_at_graveyard = atoi(row[13]);
			entry.is_locked        = atoi(row[14]);
			entry.exp              = atoi(row[15]);
			entry.size             = atoi(row[16]);
			entry.level            = atoi(row[17]);
			entry.race             = atoi(row[18]);
			entry.gender           = atoi(row[19]);
			entry.class            = atoi(row[20]);
			entry.deity            = atoi(row[21]);
			entry.texture          = atoi(row[22]);
			entry.helm_texture     = atoi(row[23]);
			entry.copper           = atoi(row[24]);
			entry.silver           = atoi(row[25]);
			entry.gold             = atoi(row[26]);
			entry.platinum         = atoi(row[27]);
			entry.hair_color       = atoi(row[28]);
			entry.beard_color      = atoi(row[29]);
			entry.eye_color_1      = atoi(row[30]);
			entry.eye_color_2      = atoi(row[31]);
			entry.hair_style       = atoi(row[32]);
			entry.face             = atoi(row[33]);
			entry.beard            = atoi(row[34]);
			entry.drakkin_heritage = atoi(row[35]);
			entry.drakkin_tattoo   = atoi(row[36]);
			entry.drakkin_details  = atoi(row[37]);
			entry.wc_1             = atoi(row[38]);
			entry.wc_2             = atoi(row[39]);
			entry.wc_3             = atoi(row[40]);
			entry.wc_4             = atoi(row[41]);
			entry.wc_5             = atoi(row[42]);
			entry.wc_6             = atoi(row[43]);
			entry.wc_7             = atoi(row[44]);
			entry.wc_8             = atoi(row[45]);
			entry.wc_9             = atoi(row[46]);

			all_entries.push_back(entry);
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
			CharacterCorpses entry{};

			entry.id               = atoi(row[0]);
			entry.charid           = atoi(row[1]);
			entry.charname         = row[2] ? row[2] : "";
			entry.zone_id          = atoi(row[3]);
			entry.instance_id      = atoi(row[4]);
			entry.x                = static_cast<float>(atof(row[5]));
			entry.y                = static_cast<float>(atof(row[6]));
			entry.z                = static_cast<float>(atof(row[7]));
			entry.heading          = static_cast<float>(atof(row[8]));
			entry.time_of_death    = row[9] ? row[9] : "";
			entry.guild_consent_id = atoi(row[10]);
			entry.is_rezzed        = atoi(row[11]);
			entry.is_buried        = atoi(row[12]);
			entry.was_at_graveyard = atoi(row[13]);
			entry.is_locked        = atoi(row[14]);
			entry.exp              = atoi(row[15]);
			entry.size             = atoi(row[16]);
			entry.level            = atoi(row[17]);
			entry.race             = atoi(row[18]);
			entry.gender           = atoi(row[19]);
			entry.class            = atoi(row[20]);
			entry.deity            = atoi(row[21]);
			entry.texture          = atoi(row[22]);
			entry.helm_texture     = atoi(row[23]);
			entry.copper           = atoi(row[24]);
			entry.silver           = atoi(row[25]);
			entry.gold             = atoi(row[26]);
			entry.platinum         = atoi(row[27]);
			entry.hair_color       = atoi(row[28]);
			entry.beard_color      = atoi(row[29]);
			entry.eye_color_1      = atoi(row[30]);
			entry.eye_color_2      = atoi(row[31]);
			entry.hair_style       = atoi(row[32]);
			entry.face             = atoi(row[33]);
			entry.beard            = atoi(row[34]);
			entry.drakkin_heritage = atoi(row[35]);
			entry.drakkin_tattoo   = atoi(row[36]);
			entry.drakkin_details  = atoi(row[37]);
			entry.wc_1             = atoi(row[38]);
			entry.wc_2             = atoi(row[39]);
			entry.wc_3             = atoi(row[40]);
			entry.wc_4             = atoi(row[41]);
			entry.wc_5             = atoi(row[42]);
			entry.wc_6             = atoi(row[43]);
			entry.wc_7             = atoi(row[44]);
			entry.wc_8             = atoi(row[45]);
			entry.wc_9             = atoi(row[46]);

			all_entries.push_back(entry);
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

};

#endif //EQEMU_BASE_CHARACTER_CORPSES_REPOSITORY_H
