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

#ifndef EQEMU_BASE_BOT_SPELL_CASTING_CHANCES_REPOSITORY_H
#define EQEMU_BASE_BOT_SPELL_CASTING_CHANCES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseBotSpellCastingChancesRepository {
public:
	struct BotSpellCastingChances {
		int32_t id;
		uint8_t spell_type_index;
		uint8_t class_id;
		uint8_t stance_index;
		uint8_t nHSND_value;
		uint8_t pH_value;
		uint8_t pS_value;
		uint8_t pHS_value;
		uint8_t pN_value;
		uint8_t pHN_value;
		uint8_t pSN_value;
		uint8_t pHSN_value;
		uint8_t pD_value;
		uint8_t pHD_value;
		uint8_t pSD_value;
		uint8_t pHSD_value;
		uint8_t pND_value;
		uint8_t pHND_value;
		uint8_t pSND_value;
		uint8_t pHSND_value;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"spell_type_index",
			"class_id",
			"stance_index",
			"nHSND_value",
			"pH_value",
			"pS_value",
			"pHS_value",
			"pN_value",
			"pHN_value",
			"pSN_value",
			"pHSN_value",
			"pD_value",
			"pHD_value",
			"pSD_value",
			"pHSD_value",
			"pND_value",
			"pHND_value",
			"pSND_value",
			"pHSND_value",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"spell_type_index",
			"class_id",
			"stance_index",
			"nHSND_value",
			"pH_value",
			"pS_value",
			"pHS_value",
			"pN_value",
			"pHN_value",
			"pSN_value",
			"pHSN_value",
			"pD_value",
			"pHD_value",
			"pSD_value",
			"pHSD_value",
			"pND_value",
			"pHND_value",
			"pSND_value",
			"pHSND_value",
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
		return std::string("bot_spell_casting_chances");
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

	static BotSpellCastingChances NewEntity()
	{
		BotSpellCastingChances e{};

		e.id               = 0;
		e.spell_type_index = 0;
		e.class_id         = 0;
		e.stance_index     = 0;
		e.nHSND_value      = 0;
		e.pH_value         = 0;
		e.pS_value         = 0;
		e.pHS_value        = 0;
		e.pN_value         = 0;
		e.pHN_value        = 0;
		e.pSN_value        = 0;
		e.pHSN_value       = 0;
		e.pD_value         = 0;
		e.pHD_value        = 0;
		e.pSD_value        = 0;
		e.pHSD_value       = 0;
		e.pND_value        = 0;
		e.pHND_value       = 0;
		e.pSND_value       = 0;
		e.pHSND_value      = 0;

		return e;
	}

	static BotSpellCastingChances GetBotSpellCastingChances(
		const std::vector<BotSpellCastingChances> &bot_spell_casting_chancess,
		int bot_spell_casting_chances_id
	)
	{
		for (auto &bot_spell_casting_chances : bot_spell_casting_chancess) {
			if (bot_spell_casting_chances.id == bot_spell_casting_chances_id) {
				return bot_spell_casting_chances;
			}
		}

		return NewEntity();
	}

	static BotSpellCastingChances FindOne(
		Database& db,
		int bot_spell_casting_chances_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				bot_spell_casting_chances_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotSpellCastingChances e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.spell_type_index = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.class_id         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.stance_index     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.nHSND_value      = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.pH_value         = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.pS_value         = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.pHS_value        = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.pN_value         = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.pHN_value        = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.pSN_value        = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.pHSN_value       = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.pD_value         = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.pHD_value        = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.pSD_value        = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.pHSD_value       = static_cast<uint8_t>(strtoul(row[15], nullptr, 10));
			e.pND_value        = static_cast<uint8_t>(strtoul(row[16], nullptr, 10));
			e.pHND_value       = static_cast<uint8_t>(strtoul(row[17], nullptr, 10));
			e.pSND_value       = static_cast<uint8_t>(strtoul(row[18], nullptr, 10));
			e.pHSND_value      = static_cast<uint8_t>(strtoul(row[19], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_spell_casting_chances_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_spell_casting_chances_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotSpellCastingChances &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.spell_type_index));
		v.push_back(columns[2] + " = " + std::to_string(e.class_id));
		v.push_back(columns[3] + " = " + std::to_string(e.stance_index));
		v.push_back(columns[4] + " = " + std::to_string(e.nHSND_value));
		v.push_back(columns[5] + " = " + std::to_string(e.pH_value));
		v.push_back(columns[6] + " = " + std::to_string(e.pS_value));
		v.push_back(columns[7] + " = " + std::to_string(e.pHS_value));
		v.push_back(columns[8] + " = " + std::to_string(e.pN_value));
		v.push_back(columns[9] + " = " + std::to_string(e.pHN_value));
		v.push_back(columns[10] + " = " + std::to_string(e.pSN_value));
		v.push_back(columns[11] + " = " + std::to_string(e.pHSN_value));
		v.push_back(columns[12] + " = " + std::to_string(e.pD_value));
		v.push_back(columns[13] + " = " + std::to_string(e.pHD_value));
		v.push_back(columns[14] + " = " + std::to_string(e.pSD_value));
		v.push_back(columns[15] + " = " + std::to_string(e.pHSD_value));
		v.push_back(columns[16] + " = " + std::to_string(e.pND_value));
		v.push_back(columns[17] + " = " + std::to_string(e.pHND_value));
		v.push_back(columns[18] + " = " + std::to_string(e.pSND_value));
		v.push_back(columns[19] + " = " + std::to_string(e.pHSND_value));

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

	static BotSpellCastingChances InsertOne(
		Database& db,
		BotSpellCastingChances e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.spell_type_index));
		v.push_back(std::to_string(e.class_id));
		v.push_back(std::to_string(e.stance_index));
		v.push_back(std::to_string(e.nHSND_value));
		v.push_back(std::to_string(e.pH_value));
		v.push_back(std::to_string(e.pS_value));
		v.push_back(std::to_string(e.pHS_value));
		v.push_back(std::to_string(e.pN_value));
		v.push_back(std::to_string(e.pHN_value));
		v.push_back(std::to_string(e.pSN_value));
		v.push_back(std::to_string(e.pHSN_value));
		v.push_back(std::to_string(e.pD_value));
		v.push_back(std::to_string(e.pHD_value));
		v.push_back(std::to_string(e.pSD_value));
		v.push_back(std::to_string(e.pHSD_value));
		v.push_back(std::to_string(e.pND_value));
		v.push_back(std::to_string(e.pHND_value));
		v.push_back(std::to_string(e.pSND_value));
		v.push_back(std::to_string(e.pHSND_value));

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
		const std::vector<BotSpellCastingChances> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.spell_type_index));
			v.push_back(std::to_string(e.class_id));
			v.push_back(std::to_string(e.stance_index));
			v.push_back(std::to_string(e.nHSND_value));
			v.push_back(std::to_string(e.pH_value));
			v.push_back(std::to_string(e.pS_value));
			v.push_back(std::to_string(e.pHS_value));
			v.push_back(std::to_string(e.pN_value));
			v.push_back(std::to_string(e.pHN_value));
			v.push_back(std::to_string(e.pSN_value));
			v.push_back(std::to_string(e.pHSN_value));
			v.push_back(std::to_string(e.pD_value));
			v.push_back(std::to_string(e.pHD_value));
			v.push_back(std::to_string(e.pSD_value));
			v.push_back(std::to_string(e.pHSD_value));
			v.push_back(std::to_string(e.pND_value));
			v.push_back(std::to_string(e.pHND_value));
			v.push_back(std::to_string(e.pSND_value));
			v.push_back(std::to_string(e.pHSND_value));

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

	static std::vector<BotSpellCastingChances> All(Database& db)
	{
		std::vector<BotSpellCastingChances> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotSpellCastingChances e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.spell_type_index = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.class_id         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.stance_index     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.nHSND_value      = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.pH_value         = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.pS_value         = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.pHS_value        = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.pN_value         = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.pHN_value        = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.pSN_value        = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.pHSN_value       = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.pD_value         = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.pHD_value        = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.pSD_value        = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.pHSD_value       = static_cast<uint8_t>(strtoul(row[15], nullptr, 10));
			e.pND_value        = static_cast<uint8_t>(strtoul(row[16], nullptr, 10));
			e.pHND_value       = static_cast<uint8_t>(strtoul(row[17], nullptr, 10));
			e.pSND_value       = static_cast<uint8_t>(strtoul(row[18], nullptr, 10));
			e.pHSND_value      = static_cast<uint8_t>(strtoul(row[19], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotSpellCastingChances> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotSpellCastingChances> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotSpellCastingChances e{};

			e.id               = static_cast<int32_t>(atoi(row[0]));
			e.spell_type_index = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.class_id         = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.stance_index     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.nHSND_value      = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.pH_value         = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.pS_value         = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.pHS_value        = static_cast<uint8_t>(strtoul(row[7], nullptr, 10));
			e.pN_value         = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));
			e.pHN_value        = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.pSN_value        = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.pHSN_value       = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.pD_value         = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.pHD_value        = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.pSD_value        = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.pHSD_value       = static_cast<uint8_t>(strtoul(row[15], nullptr, 10));
			e.pND_value        = static_cast<uint8_t>(strtoul(row[16], nullptr, 10));
			e.pHND_value       = static_cast<uint8_t>(strtoul(row[17], nullptr, 10));
			e.pSND_value       = static_cast<uint8_t>(strtoul(row[18], nullptr, 10));
			e.pHSND_value      = static_cast<uint8_t>(strtoul(row[19], nullptr, 10));

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

#endif //EQEMU_BASE_BOT_SPELL_CASTING_CHANCES_REPOSITORY_H
