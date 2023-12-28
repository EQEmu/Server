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

#ifndef EQEMU_BASE_GUILD_MEMBERS_REPOSITORY_H
#define EQEMU_BASE_GUILD_MEMBERS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>


class BaseGuildMembersRepository {
public:
	struct GuildMembers {
		int32_t     char_id;
		uint32_t    guild_id;
		uint8_t     rank;
		uint8_t     tribute_enable;
		uint32_t    total_tribute;
		uint32_t    last_tribute;
		uint8_t     banker;
		std::string public_note;
		uint8_t     alt;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"guild_id",
			"rank",
			"tribute_enable",
			"total_tribute",
			"last_tribute",
			"banker",
			"public_note",
			"alt",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"char_id",
			"guild_id",
			"rank",
			"tribute_enable",
			"total_tribute",
			"last_tribute",
			"banker",
			"public_note",
			"alt",
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
		return std::string("guild_members");
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

	static GuildMembers NewEntity()
	{
		GuildMembers e{};

		e.char_id        = 0;
		e.guild_id       = 0;
		e.rank           = 0;
		e.tribute_enable = 0;
		e.total_tribute  = 0;
		e.last_tribute   = 0;
		e.banker         = 0;
		e.public_note    = "";
		e.alt            = 0;

		return e;
	}

	static GuildMembers GetGuildMembers(
		const std::vector<GuildMembers> &guild_memberss,
		int guild_members_id
	)
	{
		for (auto &guild_members : guild_memberss) {
			if (guild_members.char_id == guild_members_id) {
				return guild_members;
			}
		}

		return NewEntity();
	}

	static GuildMembers FindOne(
		Database& db,
		int guild_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				guild_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GuildMembers e{};

			e.char_id        = static_cast<int32_t>(atoi(row[0]));
			e.guild_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.rank           = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.tribute_enable = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.total_tribute  = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.last_tribute   = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.banker         = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.public_note    = row[7] ? row[7] : "";
			e.alt            = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guild_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guild_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GuildMembers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.char_id));
		v.push_back(columns[1] + " = " + std::to_string(e.guild_id));
		v.push_back(columns[2] + " = " + std::to_string(e.rank));
		v.push_back(columns[3] + " = " + std::to_string(e.tribute_enable));
		v.push_back(columns[4] + " = " + std::to_string(e.total_tribute));
		v.push_back(columns[5] + " = " + std::to_string(e.last_tribute));
		v.push_back(columns[6] + " = " + std::to_string(e.banker));
		v.push_back(columns[7] + " = '" + Strings::Escape(e.public_note) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.alt));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GuildMembers InsertOne(
		Database& db,
		GuildMembers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.rank));
		v.push_back(std::to_string(e.tribute_enable));
		v.push_back(std::to_string(e.total_tribute));
		v.push_back(std::to_string(e.last_tribute));
		v.push_back(std::to_string(e.banker));
		v.push_back("'" + Strings::Escape(e.public_note) + "'");
		v.push_back(std::to_string(e.alt));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.char_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<GuildMembers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.rank));
			v.push_back(std::to_string(e.tribute_enable));
			v.push_back(std::to_string(e.total_tribute));
			v.push_back(std::to_string(e.last_tribute));
			v.push_back(std::to_string(e.banker));
			v.push_back("'" + Strings::Escape(e.public_note) + "'");
			v.push_back(std::to_string(e.alt));

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

	static std::vector<GuildMembers> All(Database& db)
	{
		std::vector<GuildMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildMembers e{};

			e.char_id        = static_cast<int32_t>(atoi(row[0]));
			e.guild_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.rank           = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.tribute_enable = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.total_tribute  = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.last_tribute   = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.banker         = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.public_note    = row[7] ? row[7] : "";
			e.alt            = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GuildMembers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GuildMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildMembers e{};

			e.char_id        = static_cast<int32_t>(atoi(row[0]));
			e.guild_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.rank           = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.tribute_enable = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.total_tribute  = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.last_tribute   = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.banker         = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.public_note    = row[7] ? row[7] : "";
			e.alt            = static_cast<uint8_t>(strtoul(row[8], nullptr, 10));

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

#endif //EQEMU_BASE_GUILD_MEMBERS_REPOSITORY_H
