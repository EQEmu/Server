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
		uint32_t    id;
		uint32_t    charid;
		std::string charname;
		int16_t     zone_id;
		uint16_t    instance_id;
		float       x;
		float       y;
		float       z;
		float       heading;
		time_t      time_of_death;
		uint32_t    guild_consent_id;
		uint8_t     is_rezzed;
		int8_t      is_buried;
		int8_t      was_at_graveyard;
		int8_t      is_locked;
		uint32_t    exp;
		uint32_t    size;
		uint32_t    level;
		uint32_t    race;
		uint32_t    gender;
		uint32_t    class_;
		uint32_t    deity;
		uint32_t    texture;
		uint32_t    helm_texture;
		uint32_t    copper;
		uint32_t    silver;
		uint32_t    gold;
		uint32_t    platinum;
		uint32_t    hair_color;
		uint32_t    beard_color;
		uint32_t    eye_color_1;
		uint32_t    eye_color_2;
		uint32_t    hair_style;
		uint32_t    face;
		uint32_t    beard;
		uint32_t    drakkin_heritage;
		uint32_t    drakkin_tattoo;
		uint32_t    drakkin_details;
		uint32_t    wc_1;
		uint32_t    wc_2;
		uint32_t    wc_3;
		uint32_t    wc_4;
		uint32_t    wc_5;
		uint32_t    wc_6;
		uint32_t    wc_7;
		uint32_t    wc_8;
		uint32_t    wc_9;
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

	static CharacterCorpses GetCharacterCorpses(
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

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.charid           = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.charname         = row[2] ? row[2] : "";
			e.zone_id          = static_cast<int16_t>(atoi(row[3]));
			e.instance_id      = static_cast<uint16_t>(strtoul(row[4], nullptr, 10));
			e.x                = strtof(row[5], nullptr);
			e.y                = strtof(row[6], nullptr);
			e.z                = strtof(row[7], nullptr);
			e.heading          = strtof(row[8], nullptr);
			e.time_of_death    = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.guild_consent_id = static_cast<uint32_t>(strtoul(row[10], nullptr, 10));
			e.is_rezzed        = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.is_buried        = static_cast<int8_t>(atoi(row[12]));
			e.was_at_graveyard = static_cast<int8_t>(atoi(row[13]));
			e.is_locked        = static_cast<int8_t>(atoi(row[14]));
			e.exp              = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.size             = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.level            = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.race             = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.gender           = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.class_           = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.deity            = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.texture          = static_cast<uint32_t>(strtoul(row[22], nullptr, 10));
			e.helm_texture     = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.copper           = static_cast<uint32_t>(strtoul(row[24], nullptr, 10));
			e.silver           = static_cast<uint32_t>(strtoul(row[25], nullptr, 10));
			e.gold             = static_cast<uint32_t>(strtoul(row[26], nullptr, 10));
			e.platinum         = static_cast<uint32_t>(strtoul(row[27], nullptr, 10));
			e.hair_color       = static_cast<uint32_t>(strtoul(row[28], nullptr, 10));
			e.beard_color      = static_cast<uint32_t>(strtoul(row[29], nullptr, 10));
			e.eye_color_1      = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.eye_color_2      = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.hair_style       = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.face             = static_cast<uint32_t>(strtoul(row[33], nullptr, 10));
			e.beard            = static_cast<uint32_t>(strtoul(row[34], nullptr, 10));
			e.drakkin_heritage = static_cast<uint32_t>(strtoul(row[35], nullptr, 10));
			e.drakkin_tattoo   = static_cast<uint32_t>(strtoul(row[36], nullptr, 10));
			e.drakkin_details  = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.wc_1             = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.wc_2             = static_cast<uint32_t>(strtoul(row[39], nullptr, 10));
			e.wc_3             = static_cast<uint32_t>(strtoul(row[40], nullptr, 10));
			e.wc_4             = static_cast<uint32_t>(strtoul(row[41], nullptr, 10));
			e.wc_5             = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.wc_6             = static_cast<uint32_t>(strtoul(row[43], nullptr, 10));
			e.wc_7             = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.wc_8             = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.wc_9             = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));

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
		const CharacterCorpses &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.charid));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.charname) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[4] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[5] + " = " + std::to_string(e.x));
		v.push_back(columns[6] + " = " + std::to_string(e.y));
		v.push_back(columns[7] + " = " + std::to_string(e.z));
		v.push_back(columns[8] + " = " + std::to_string(e.heading));
		v.push_back(columns[9] + " = FROM_UNIXTIME(" + (e.time_of_death > 0 ? std::to_string(e.time_of_death) : "null") + ")");
		v.push_back(columns[10] + " = " + std::to_string(e.guild_consent_id));
		v.push_back(columns[11] + " = " + std::to_string(e.is_rezzed));
		v.push_back(columns[12] + " = " + std::to_string(e.is_buried));
		v.push_back(columns[13] + " = " + std::to_string(e.was_at_graveyard));
		v.push_back(columns[14] + " = " + std::to_string(e.is_locked));
		v.push_back(columns[15] + " = " + std::to_string(e.exp));
		v.push_back(columns[16] + " = " + std::to_string(e.size));
		v.push_back(columns[17] + " = " + std::to_string(e.level));
		v.push_back(columns[18] + " = " + std::to_string(e.race));
		v.push_back(columns[19] + " = " + std::to_string(e.gender));
		v.push_back(columns[20] + " = " + std::to_string(e.class_));
		v.push_back(columns[21] + " = " + std::to_string(e.deity));
		v.push_back(columns[22] + " = " + std::to_string(e.texture));
		v.push_back(columns[23] + " = " + std::to_string(e.helm_texture));
		v.push_back(columns[24] + " = " + std::to_string(e.copper));
		v.push_back(columns[25] + " = " + std::to_string(e.silver));
		v.push_back(columns[26] + " = " + std::to_string(e.gold));
		v.push_back(columns[27] + " = " + std::to_string(e.platinum));
		v.push_back(columns[28] + " = " + std::to_string(e.hair_color));
		v.push_back(columns[29] + " = " + std::to_string(e.beard_color));
		v.push_back(columns[30] + " = " + std::to_string(e.eye_color_1));
		v.push_back(columns[31] + " = " + std::to_string(e.eye_color_2));
		v.push_back(columns[32] + " = " + std::to_string(e.hair_style));
		v.push_back(columns[33] + " = " + std::to_string(e.face));
		v.push_back(columns[34] + " = " + std::to_string(e.beard));
		v.push_back(columns[35] + " = " + std::to_string(e.drakkin_heritage));
		v.push_back(columns[36] + " = " + std::to_string(e.drakkin_tattoo));
		v.push_back(columns[37] + " = " + std::to_string(e.drakkin_details));
		v.push_back(columns[38] + " = " + std::to_string(e.wc_1));
		v.push_back(columns[39] + " = " + std::to_string(e.wc_2));
		v.push_back(columns[40] + " = " + std::to_string(e.wc_3));
		v.push_back(columns[41] + " = " + std::to_string(e.wc_4));
		v.push_back(columns[42] + " = " + std::to_string(e.wc_5));
		v.push_back(columns[43] + " = " + std::to_string(e.wc_6));
		v.push_back(columns[44] + " = " + std::to_string(e.wc_7));
		v.push_back(columns[45] + " = " + std::to_string(e.wc_8));
		v.push_back(columns[46] + " = " + std::to_string(e.wc_9));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
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
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.charid));
		v.push_back("'" + Strings::Escape(e.charname) + "'");
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back("FROM_UNIXTIME(" + (e.time_of_death > 0 ? std::to_string(e.time_of_death) : "null") + ")");
		v.push_back(std::to_string(e.guild_consent_id));
		v.push_back(std::to_string(e.is_rezzed));
		v.push_back(std::to_string(e.is_buried));
		v.push_back(std::to_string(e.was_at_graveyard));
		v.push_back(std::to_string(e.is_locked));
		v.push_back(std::to_string(e.exp));
		v.push_back(std::to_string(e.size));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.deity));
		v.push_back(std::to_string(e.texture));
		v.push_back(std::to_string(e.helm_texture));
		v.push_back(std::to_string(e.copper));
		v.push_back(std::to_string(e.silver));
		v.push_back(std::to_string(e.gold));
		v.push_back(std::to_string(e.platinum));
		v.push_back(std::to_string(e.hair_color));
		v.push_back(std::to_string(e.beard_color));
		v.push_back(std::to_string(e.eye_color_1));
		v.push_back(std::to_string(e.eye_color_2));
		v.push_back(std::to_string(e.hair_style));
		v.push_back(std::to_string(e.face));
		v.push_back(std::to_string(e.beard));
		v.push_back(std::to_string(e.drakkin_heritage));
		v.push_back(std::to_string(e.drakkin_tattoo));
		v.push_back(std::to_string(e.drakkin_details));
		v.push_back(std::to_string(e.wc_1));
		v.push_back(std::to_string(e.wc_2));
		v.push_back(std::to_string(e.wc_3));
		v.push_back(std::to_string(e.wc_4));
		v.push_back(std::to_string(e.wc_5));
		v.push_back(std::to_string(e.wc_6));
		v.push_back(std::to_string(e.wc_7));
		v.push_back(std::to_string(e.wc_8));
		v.push_back(std::to_string(e.wc_9));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
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
		const std::vector<CharacterCorpses> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.charid));
			v.push_back("'" + Strings::Escape(e.charname) + "'");
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back("FROM_UNIXTIME(" + (e.time_of_death > 0 ? std::to_string(e.time_of_death) : "null") + ")");
			v.push_back(std::to_string(e.guild_consent_id));
			v.push_back(std::to_string(e.is_rezzed));
			v.push_back(std::to_string(e.is_buried));
			v.push_back(std::to_string(e.was_at_graveyard));
			v.push_back(std::to_string(e.is_locked));
			v.push_back(std::to_string(e.exp));
			v.push_back(std::to_string(e.size));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.deity));
			v.push_back(std::to_string(e.texture));
			v.push_back(std::to_string(e.helm_texture));
			v.push_back(std::to_string(e.copper));
			v.push_back(std::to_string(e.silver));
			v.push_back(std::to_string(e.gold));
			v.push_back(std::to_string(e.platinum));
			v.push_back(std::to_string(e.hair_color));
			v.push_back(std::to_string(e.beard_color));
			v.push_back(std::to_string(e.eye_color_1));
			v.push_back(std::to_string(e.eye_color_2));
			v.push_back(std::to_string(e.hair_style));
			v.push_back(std::to_string(e.face));
			v.push_back(std::to_string(e.beard));
			v.push_back(std::to_string(e.drakkin_heritage));
			v.push_back(std::to_string(e.drakkin_tattoo));
			v.push_back(std::to_string(e.drakkin_details));
			v.push_back(std::to_string(e.wc_1));
			v.push_back(std::to_string(e.wc_2));
			v.push_back(std::to_string(e.wc_3));
			v.push_back(std::to_string(e.wc_4));
			v.push_back(std::to_string(e.wc_5));
			v.push_back(std::to_string(e.wc_6));
			v.push_back(std::to_string(e.wc_7));
			v.push_back(std::to_string(e.wc_8));
			v.push_back(std::to_string(e.wc_9));

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

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.charid           = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.charname         = row[2] ? row[2] : "";
			e.zone_id          = static_cast<int16_t>(atoi(row[3]));
			e.instance_id      = static_cast<uint16_t>(strtoul(row[4], nullptr, 10));
			e.x                = strtof(row[5], nullptr);
			e.y                = strtof(row[6], nullptr);
			e.z                = strtof(row[7], nullptr);
			e.heading          = strtof(row[8], nullptr);
			e.time_of_death    = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.guild_consent_id = static_cast<uint32_t>(strtoul(row[10], nullptr, 10));
			e.is_rezzed        = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.is_buried        = static_cast<int8_t>(atoi(row[12]));
			e.was_at_graveyard = static_cast<int8_t>(atoi(row[13]));
			e.is_locked        = static_cast<int8_t>(atoi(row[14]));
			e.exp              = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.size             = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.level            = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.race             = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.gender           = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.class_           = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.deity            = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.texture          = static_cast<uint32_t>(strtoul(row[22], nullptr, 10));
			e.helm_texture     = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.copper           = static_cast<uint32_t>(strtoul(row[24], nullptr, 10));
			e.silver           = static_cast<uint32_t>(strtoul(row[25], nullptr, 10));
			e.gold             = static_cast<uint32_t>(strtoul(row[26], nullptr, 10));
			e.platinum         = static_cast<uint32_t>(strtoul(row[27], nullptr, 10));
			e.hair_color       = static_cast<uint32_t>(strtoul(row[28], nullptr, 10));
			e.beard_color      = static_cast<uint32_t>(strtoul(row[29], nullptr, 10));
			e.eye_color_1      = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.eye_color_2      = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.hair_style       = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.face             = static_cast<uint32_t>(strtoul(row[33], nullptr, 10));
			e.beard            = static_cast<uint32_t>(strtoul(row[34], nullptr, 10));
			e.drakkin_heritage = static_cast<uint32_t>(strtoul(row[35], nullptr, 10));
			e.drakkin_tattoo   = static_cast<uint32_t>(strtoul(row[36], nullptr, 10));
			e.drakkin_details  = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.wc_1             = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.wc_2             = static_cast<uint32_t>(strtoul(row[39], nullptr, 10));
			e.wc_3             = static_cast<uint32_t>(strtoul(row[40], nullptr, 10));
			e.wc_4             = static_cast<uint32_t>(strtoul(row[41], nullptr, 10));
			e.wc_5             = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.wc_6             = static_cast<uint32_t>(strtoul(row[43], nullptr, 10));
			e.wc_7             = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.wc_8             = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.wc_9             = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterCorpses> GetWhere(Database& db, const std::string &where_filter)
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

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.charid           = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.charname         = row[2] ? row[2] : "";
			e.zone_id          = static_cast<int16_t>(atoi(row[3]));
			e.instance_id      = static_cast<uint16_t>(strtoul(row[4], nullptr, 10));
			e.x                = strtof(row[5], nullptr);
			e.y                = strtof(row[6], nullptr);
			e.z                = strtof(row[7], nullptr);
			e.heading          = strtof(row[8], nullptr);
			e.time_of_death    = strtoll(row[9] ? row[9] : "-1", nullptr, 10);
			e.guild_consent_id = static_cast<uint32_t>(strtoul(row[10], nullptr, 10));
			e.is_rezzed        = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.is_buried        = static_cast<int8_t>(atoi(row[12]));
			e.was_at_graveyard = static_cast<int8_t>(atoi(row[13]));
			e.is_locked        = static_cast<int8_t>(atoi(row[14]));
			e.exp              = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.size             = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.level            = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.race             = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.gender           = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.class_           = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.deity            = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.texture          = static_cast<uint32_t>(strtoul(row[22], nullptr, 10));
			e.helm_texture     = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.copper           = static_cast<uint32_t>(strtoul(row[24], nullptr, 10));
			e.silver           = static_cast<uint32_t>(strtoul(row[25], nullptr, 10));
			e.gold             = static_cast<uint32_t>(strtoul(row[26], nullptr, 10));
			e.platinum         = static_cast<uint32_t>(strtoul(row[27], nullptr, 10));
			e.hair_color       = static_cast<uint32_t>(strtoul(row[28], nullptr, 10));
			e.beard_color      = static_cast<uint32_t>(strtoul(row[29], nullptr, 10));
			e.eye_color_1      = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.eye_color_2      = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.hair_style       = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.face             = static_cast<uint32_t>(strtoul(row[33], nullptr, 10));
			e.beard            = static_cast<uint32_t>(strtoul(row[34], nullptr, 10));
			e.drakkin_heritage = static_cast<uint32_t>(strtoul(row[35], nullptr, 10));
			e.drakkin_tattoo   = static_cast<uint32_t>(strtoul(row[36], nullptr, 10));
			e.drakkin_details  = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.wc_1             = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.wc_2             = static_cast<uint32_t>(strtoul(row[39], nullptr, 10));
			e.wc_3             = static_cast<uint32_t>(strtoul(row[40], nullptr, 10));
			e.wc_4             = static_cast<uint32_t>(strtoul(row[41], nullptr, 10));
			e.wc_5             = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.wc_6             = static_cast<uint32_t>(strtoul(row[43], nullptr, 10));
			e.wc_7             = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.wc_8             = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.wc_9             = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));

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

#endif //EQEMU_BASE_CHARACTER_CORPSES_REPOSITORY_H
