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

#ifndef EQEMU_BASE_IP_EXEMPTIONS_REPOSITORY_H
#define EQEMU_BASE_IP_EXEMPTIONS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseIpExemptionsRepository {
public:
	struct IpExemptions {
		int         exemption_id;
		std::string exemption_ip;
		int         exemption_amount;
	};

	static std::string PrimaryKey()
	{
		return std::string("exemption_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"exemption_id",
			"exemption_ip",
			"exemption_amount",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("ip_exemptions");
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

	static IpExemptions NewEntity()
	{
		IpExemptions entry{};

		entry.exemption_id     = 0;
		entry.exemption_ip     = "";
		entry.exemption_amount = 0;

		return entry;
	}

	static IpExemptions GetIpExemptionsEntry(
		const std::vector<IpExemptions> &ip_exemptionss,
		int ip_exemptions_id
	)
	{
		for (auto &ip_exemptions : ip_exemptionss) {
			if (ip_exemptions.exemption_id == ip_exemptions_id) {
				return ip_exemptions;
			}
		}

		return NewEntity();
	}

	static IpExemptions FindOne(
		Database& db,
		int ip_exemptions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				ip_exemptions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			IpExemptions entry{};

			entry.exemption_id     = atoi(row[0]);
			entry.exemption_ip     = row[1] ? row[1] : "";
			entry.exemption_amount = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int ip_exemptions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				ip_exemptions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		IpExemptions ip_exemptions_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(ip_exemptions_entry.exemption_ip) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(ip_exemptions_entry.exemption_amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				ip_exemptions_entry.exemption_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static IpExemptions InsertOne(
		Database& db,
		IpExemptions ip_exemptions_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(ip_exemptions_entry.exemption_id));
		insert_values.push_back("'" + EscapeString(ip_exemptions_entry.exemption_ip) + "'");
		insert_values.push_back(std::to_string(ip_exemptions_entry.exemption_amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			ip_exemptions_entry.exemption_id = results.LastInsertedID();
			return ip_exemptions_entry;
		}

		ip_exemptions_entry = NewEntity();

		return ip_exemptions_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<IpExemptions> ip_exemptions_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &ip_exemptions_entry: ip_exemptions_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(ip_exemptions_entry.exemption_id));
			insert_values.push_back("'" + EscapeString(ip_exemptions_entry.exemption_ip) + "'");
			insert_values.push_back(std::to_string(ip_exemptions_entry.exemption_amount));

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

	static std::vector<IpExemptions> All(Database& db)
	{
		std::vector<IpExemptions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			IpExemptions entry{};

			entry.exemption_id     = atoi(row[0]);
			entry.exemption_ip     = row[1] ? row[1] : "";
			entry.exemption_amount = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<IpExemptions> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<IpExemptions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			IpExemptions entry{};

			entry.exemption_id     = atoi(row[0]);
			entry.exemption_ip     = row[1] ? row[1] : "";
			entry.exemption_amount = atoi(row[2]);

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

#endif //EQEMU_BASE_IP_EXEMPTIONS_REPOSITORY_H
