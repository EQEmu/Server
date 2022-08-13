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

#ifndef EQEMU_BASE_GUILDS_REPOSITORY_H
#define EQEMU_BASE_GUILDS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseGuildsRepository {
public:
	struct Guilds {
		int         id;
		std::string name;
		int         leader;
		int         minstatus;
		std::string motd;
		int         tribute;
		std::string motd_setter;
		std::string channel;
		std::string url;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"leader",
			"minstatus",
			"motd",
			"tribute",
			"motd_setter",
			"channel",
			"url",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"leader",
			"minstatus",
			"motd",
			"tribute",
			"motd_setter",
			"channel",
			"url",
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
		return std::string("guilds");
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

	static Guilds NewEntity()
	{
		Guilds e{};

		e.id          = 0;
		e.name        = "";
		e.leader      = 0;
		e.minstatus   = 0;
		e.motd        = "";
		e.tribute     = 0;
		e.motd_setter = "";
		e.channel     = "";
		e.url         = "";

		return e;
	}

	static Guilds GetGuildse(
		const std::vector<Guilds> &guildss,
		int guilds_id
	)
	{
		for (auto &guilds : guildss) {
			if (guilds.id == guilds_id) {
				return guilds;
			}
		}

		return NewEntity();
	}

	static Guilds FindOne(
		Database& db,
		int guilds_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				guilds_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Guilds e{};

			e.id          = atoi(row[0]);
			e.name        = row[1] ? row[1] : "";
			e.leader      = atoi(row[2]);
			e.minstatus   = atoi(row[3]);
			e.motd        = row[4] ? row[4] : "";
			e.tribute     = atoi(row[5]);
			e.motd_setter = row[6] ? row[6] : "";
			e.channel     = row[7] ? row[7] : "";
			e.url         = row[8] ? row[8] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guilds_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guilds_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Guilds guilds_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + Strings::Escape(guilds_e.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(guilds_e.leader));
		update_values.push_back(columns[3] + " = " + std::to_string(guilds_e.minstatus));
		update_values.push_back(columns[4] + " = '" + Strings::Escape(guilds_e.motd) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(guilds_e.tribute));
		update_values.push_back(columns[6] + " = '" + Strings::Escape(guilds_e.motd_setter) + "'");
		update_values.push_back(columns[7] + " = '" + Strings::Escape(guilds_e.channel) + "'");
		update_values.push_back(columns[8] + " = '" + Strings::Escape(guilds_e.url) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				guilds_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Guilds InsertOne(
		Database& db,
		Guilds guilds_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(guilds_e.id));
		insert_values.push_back("'" + Strings::Escape(guilds_e.name) + "'");
		insert_values.push_back(std::to_string(guilds_e.leader));
		insert_values.push_back(std::to_string(guilds_e.minstatus));
		insert_values.push_back("'" + Strings::Escape(guilds_e.motd) + "'");
		insert_values.push_back(std::to_string(guilds_e.tribute));
		insert_values.push_back("'" + Strings::Escape(guilds_e.motd_setter) + "'");
		insert_values.push_back("'" + Strings::Escape(guilds_e.channel) + "'");
		insert_values.push_back("'" + Strings::Escape(guilds_e.url) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			guilds_e.id = results.LastInsertedID();
			return guilds_e;
		}

		guilds_e = NewEntity();

		return guilds_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Guilds> guilds_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &guilds_e: guilds_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(guilds_e.id));
			insert_values.push_back("'" + Strings::Escape(guilds_e.name) + "'");
			insert_values.push_back(std::to_string(guilds_e.leader));
			insert_values.push_back(std::to_string(guilds_e.minstatus));
			insert_values.push_back("'" + Strings::Escape(guilds_e.motd) + "'");
			insert_values.push_back(std::to_string(guilds_e.tribute));
			insert_values.push_back("'" + Strings::Escape(guilds_e.motd_setter) + "'");
			insert_values.push_back("'" + Strings::Escape(guilds_e.channel) + "'");
			insert_values.push_back("'" + Strings::Escape(guilds_e.url) + "'");

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

	static std::vector<Guilds> All(Database& db)
	{
		std::vector<Guilds> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Guilds e{};

			e.id          = atoi(row[0]);
			e.name        = row[1] ? row[1] : "";
			e.leader      = atoi(row[2]);
			e.minstatus   = atoi(row[3]);
			e.motd        = row[4] ? row[4] : "";
			e.tribute     = atoi(row[5]);
			e.motd_setter = row[6] ? row[6] : "";
			e.channel     = row[7] ? row[7] : "";
			e.url         = row[8] ? row[8] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Guilds> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Guilds> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Guilds e{};

			e.id          = atoi(row[0]);
			e.name        = row[1] ? row[1] : "";
			e.leader      = atoi(row[2]);
			e.minstatus   = atoi(row[3]);
			e.motd        = row[4] ? row[4] : "";
			e.tribute     = atoi(row[5]);
			e.motd_setter = row[6] ? row[6] : "";
			e.channel     = row[7] ? row[7] : "";
			e.url         = row[8] ? row[8] : "";

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

#endif //EQEMU_BASE_GUILDS_REPOSITORY_H
