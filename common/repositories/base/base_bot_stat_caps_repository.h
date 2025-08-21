/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_BOT_STAT_CAPS_REPOSITORY_H
#define EQEMU_BASE_BOT_STAT_CAPS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseBotStatCapsRepository {
public:
	struct BotStatCaps {
		uint32_t bot_id;
		uint8_t  stat_id;
		int32_t  stat_cap;
	};

	static std::string PrimaryKey()
	{
		return std::string("bot_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"bot_id",
			"stat_id",
			"stat_cap",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"bot_id",
			"stat_id",
			"stat_cap",
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
		return std::string("bot_stat_caps");
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

	static BotStatCaps NewEntity()
	{
		BotStatCaps e{};

		e.bot_id   = 0;
		e.stat_id  = 0;
		e.stat_cap = -1;

		return e;
	}

	static BotStatCaps GetBotStatCaps(
		const std::vector<BotStatCaps> &bot_stat_capss,
		int bot_stat_caps_id
	)
	{
		for (auto &bot_stat_caps : bot_stat_capss) {
			if (bot_stat_caps.bot_id == bot_stat_caps_id) {
				return bot_stat_caps;
			}
		}

		return NewEntity();
	}

	static BotStatCaps FindOne(
		Database& db,
		int bot_stat_caps_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_stat_caps_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotStatCaps e{};

			e.bot_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.stat_id  = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.stat_cap = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_stat_caps_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_stat_caps_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotStatCaps &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.bot_id));
		v.push_back(columns[1] + " = " + std::to_string(e.stat_id));
		v.push_back(columns[2] + " = " + std::to_string(e.stat_cap));

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

	static BotStatCaps InsertOne(
		Database& db,
		BotStatCaps e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.stat_id));
		v.push_back(std::to_string(e.stat_cap));

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
		const std::vector<BotStatCaps> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.stat_id));
			v.push_back(std::to_string(e.stat_cap));

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

	static std::vector<BotStatCaps> All(Database& db)
	{
		std::vector<BotStatCaps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotStatCaps e{};

			e.bot_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.stat_id  = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.stat_cap = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotStatCaps> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotStatCaps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotStatCaps e{};

			e.bot_id   = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.stat_id  = row[1] ? static_cast<uint8_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.stat_cap = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;

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

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const BotStatCaps &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.bot_id));
		v.push_back(std::to_string(e.stat_id));
		v.push_back(std::to_string(e.stat_cap));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<BotStatCaps> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.bot_id));
			v.push_back(std::to_string(e.stat_id));
			v.push_back(std::to_string(e.stat_cap));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_BASE_BOT_STAT_CAPS_REPOSITORY_H
