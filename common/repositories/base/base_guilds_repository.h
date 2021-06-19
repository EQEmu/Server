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
#include "../../string_util.h"

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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("guilds");
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

	static Guilds NewEntity()
	{
		Guilds entry{};

		entry.id          = 0;
		entry.name        = "";
		entry.leader      = 0;
		entry.minstatus   = 0;
		entry.motd        = "";
		entry.tribute     = 0;
		entry.motd_setter = "";
		entry.channel     = "";
		entry.url         = "";

		return entry;
	}

	static Guilds GetGuildsEntry(
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
			Guilds entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.leader      = atoi(row[2]);
			entry.minstatus   = atoi(row[3]);
			entry.motd        = row[4] ? row[4] : "";
			entry.tribute     = atoi(row[5]);
			entry.motd_setter = row[6] ? row[6] : "";
			entry.channel     = row[7] ? row[7] : "";
			entry.url         = row[8] ? row[8] : "";

			return entry;
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
		Guilds guilds_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(guilds_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(guilds_entry.leader));
		update_values.push_back(columns[3] + " = " + std::to_string(guilds_entry.minstatus));
		update_values.push_back(columns[4] + " = '" + EscapeString(guilds_entry.motd) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(guilds_entry.tribute));
		update_values.push_back(columns[6] + " = '" + EscapeString(guilds_entry.motd_setter) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(guilds_entry.channel) + "'");
		update_values.push_back(columns[8] + " = '" + EscapeString(guilds_entry.url) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				guilds_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Guilds InsertOne(
		Database& db,
		Guilds guilds_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(guilds_entry.id));
		insert_values.push_back("'" + EscapeString(guilds_entry.name) + "'");
		insert_values.push_back(std::to_string(guilds_entry.leader));
		insert_values.push_back(std::to_string(guilds_entry.minstatus));
		insert_values.push_back("'" + EscapeString(guilds_entry.motd) + "'");
		insert_values.push_back(std::to_string(guilds_entry.tribute));
		insert_values.push_back("'" + EscapeString(guilds_entry.motd_setter) + "'");
		insert_values.push_back("'" + EscapeString(guilds_entry.channel) + "'");
		insert_values.push_back("'" + EscapeString(guilds_entry.url) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			guilds_entry.id = results.LastInsertedID();
			return guilds_entry;
		}

		guilds_entry = NewEntity();

		return guilds_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Guilds> guilds_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &guilds_entry: guilds_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(guilds_entry.id));
			insert_values.push_back("'" + EscapeString(guilds_entry.name) + "'");
			insert_values.push_back(std::to_string(guilds_entry.leader));
			insert_values.push_back(std::to_string(guilds_entry.minstatus));
			insert_values.push_back("'" + EscapeString(guilds_entry.motd) + "'");
			insert_values.push_back(std::to_string(guilds_entry.tribute));
			insert_values.push_back("'" + EscapeString(guilds_entry.motd_setter) + "'");
			insert_values.push_back("'" + EscapeString(guilds_entry.channel) + "'");
			insert_values.push_back("'" + EscapeString(guilds_entry.url) + "'");

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
			Guilds entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.leader      = atoi(row[2]);
			entry.minstatus   = atoi(row[3]);
			entry.motd        = row[4] ? row[4] : "";
			entry.tribute     = atoi(row[5]);
			entry.motd_setter = row[6] ? row[6] : "";
			entry.channel     = row[7] ? row[7] : "";
			entry.url         = row[8] ? row[8] : "";

			all_entries.push_back(entry);
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
			Guilds entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.leader      = atoi(row[2]);
			entry.minstatus   = atoi(row[3]);
			entry.motd        = row[4] ? row[4] : "";
			entry.tribute     = atoi(row[5]);
			entry.motd_setter = row[6] ? row[6] : "";
			entry.channel     = row[7] ? row[7] : "";
			entry.url         = row[8] ? row[8] : "";

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

#endif //EQEMU_BASE_GUILDS_REPOSITORY_H
