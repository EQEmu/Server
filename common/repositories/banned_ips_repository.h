/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EQEMU_BANNED_IPS_REPOSITORY_H
#define EQEMU_BANNED_IPS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class BannedIpsRepository {
public:
	struct BannedIps {
		std::string ip_address;
		std::string notes;
	};

	static std::string PrimaryKey()
	{
		return std::string("ip_address");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"ip_address",
			"notes",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("banned_ips");
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
			InsertColumnsRaw()
		);
	}

	static BannedIps NewEntity()
	{
		BannedIps entry{};

		entry.ip_address = "";
		entry.notes      = "";

		return entry;
	}

	static BannedIps GetBannedIpsEntry(
		const std::vector<BannedIps> &banned_ipss,
		int banned_ips_id
	)
	{
		for (auto &banned_ips : banned_ipss) {
			if (banned_ips.ip_address == banned_ips_id) {
				return banned_ips;
			}
		}

		return NewEntity();
	}

	static BannedIps FindOne(
		int banned_ips_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				banned_ips_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BannedIps entry{};

			entry.ip_address = row[0] ? row[0] : "";
			entry.notes      = row[1] ? row[1] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int banned_ips_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				banned_ips_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		BannedIps banned_ips_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(banned_ips_entry.notes) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				banned_ips_entry.ip_address
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BannedIps InsertOne(
		BannedIps banned_ips_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(banned_ips_entry.notes) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			banned_ips_entry.id = results.LastInsertedID();
			return banned_ips_entry;
		}

		banned_ips_entry = BannedIpsRepository::NewEntity();

		return banned_ips_entry;
	}

	static int InsertMany(
		std::vector<BannedIps> banned_ips_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &banned_ips_entry: banned_ips_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(banned_ips_entry.notes) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<BannedIps> All()
	{
		std::vector<BannedIps> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BannedIps entry{};

			entry.ip_address = row[0] ? row[0] : "";
			entry.notes      = row[1] ? row[1] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<BannedIps> GetWhere(std::string where_filter)
	{
		std::vector<BannedIps> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BannedIps entry{};

			entry.ip_address = row[0] ? row[0] : "";
			entry.notes      = row[1] ? row[1] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BANNED_IPS_REPOSITORY_H
