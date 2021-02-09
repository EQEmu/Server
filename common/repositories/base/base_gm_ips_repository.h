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

#ifndef EQEMU_BASE_GM_IPS_REPOSITORY_H
#define EQEMU_BASE_GM_IPS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGmIpsRepository {
public:
	struct GmIps {
		std::string name;
		int         account_id;
		std::string ip_address;
	};

	static std::string PrimaryKey()
	{
		return std::string("account_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"name",
			"account_id",
			"ip_address",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("gm_ips");
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

	static GmIps NewEntity()
	{
		GmIps entry{};

		entry.name       = "";
		entry.account_id = 0;
		entry.ip_address = "";

		return entry;
	}

	static GmIps GetGmIpsEntry(
		const std::vector<GmIps> &gm_ipss,
		int gm_ips_id
	)
	{
		for (auto &gm_ips : gm_ipss) {
			if (gm_ips.account_id == gm_ips_id) {
				return gm_ips;
			}
		}

		return NewEntity();
	}

	static GmIps FindOne(
		Database& db,
		int gm_ips_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				gm_ips_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GmIps entry{};

			entry.name       = row[0] ? row[0] : "";
			entry.account_id = atoi(row[1]);
			entry.ip_address = row[2] ? row[2] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int gm_ips_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				gm_ips_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		GmIps gm_ips_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = '" + EscapeString(gm_ips_entry.name) + "'");
		update_values.push_back(columns[1] + " = " + std::to_string(gm_ips_entry.account_id));
		update_values.push_back(columns[2] + " = '" + EscapeString(gm_ips_entry.ip_address) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				gm_ips_entry.account_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GmIps InsertOne(
		Database& db,
		GmIps gm_ips_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(gm_ips_entry.name) + "'");
		insert_values.push_back(std::to_string(gm_ips_entry.account_id));
		insert_values.push_back("'" + EscapeString(gm_ips_entry.ip_address) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			gm_ips_entry.account_id = results.LastInsertedID();
			return gm_ips_entry;
		}

		gm_ips_entry = NewEntity();

		return gm_ips_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<GmIps> gm_ips_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &gm_ips_entry: gm_ips_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(gm_ips_entry.name) + "'");
			insert_values.push_back(std::to_string(gm_ips_entry.account_id));
			insert_values.push_back("'" + EscapeString(gm_ips_entry.ip_address) + "'");

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

	static std::vector<GmIps> All(Database& db)
	{
		std::vector<GmIps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GmIps entry{};

			entry.name       = row[0] ? row[0] : "";
			entry.account_id = atoi(row[1]);
			entry.ip_address = row[2] ? row[2] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GmIps> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<GmIps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GmIps entry{};

			entry.name       = row[0] ? row[0] : "";
			entry.account_id = atoi(row[1]);
			entry.ip_address = row[2] ? row[2] : "";

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

#endif //EQEMU_BASE_GM_IPS_REPOSITORY_H
