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

#ifndef EQEMU_ACCOUNT_IP_REPOSITORY_H
#define EQEMU_ACCOUNT_IP_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class AccountIpRepository {
public:
	struct AccountIp {
		int         accid;
		std::string ip;
		int         count;
		std::string lastused;
	};

	static std::string PrimaryKey()
	{
		return std::string("ip");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"accid",
			"ip",
			"count",
			"lastused",
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
		return std::string("account_ip");
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

	static AccountIp NewEntity()
	{
		AccountIp entry{};

		entry.accid    = 0;
		entry.ip       = "";
		entry.count    = 1;
		entry.lastused = current_timestamp();

		return entry;
	}

	static AccountIp GetAccountIpEntry(
		const std::vector<AccountIp> &account_ips,
		int account_ip_id
	)
	{
		for (auto &account_ip : account_ips) {
			if (account_ip.ip == account_ip_id) {
				return account_ip;
			}
		}

		return NewEntity();
	}

	static AccountIp FindOne(
		int account_ip_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				account_ip_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AccountIp entry{};

			entry.accid    = atoi(row[0]);
			entry.ip       = row[1];
			entry.count    = atoi(row[2]);
			entry.lastused = row[3];

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int account_ip_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				account_ip_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		AccountIp account_ip_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[2] + " = " + std::to_string(account_ip_entry.count));
		update_values.push_back(columns[3] + " = '" + EscapeString(account_ip_entry.lastused) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				account_ip_entry.ip
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AccountIp InsertOne(
		AccountIp account_ip_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(account_ip_entry.count));
		insert_values.push_back("'" + EscapeString(account_ip_entry.lastused) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			account_ip_entry.id = results.LastInsertedID();
			return account_ip_entry;
		}

		account_ip_entry = InstanceListRepository::NewEntity();

		return account_ip_entry;
	}

	static int InsertMany(
		std::vector<AccountIp> account_ip_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &account_ip_entry: account_ip_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(account_ip_entry.count));
			insert_values.push_back("'" + EscapeString(account_ip_entry.lastused) + "'");

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

	static std::vector<AccountIp> All()
	{
		std::vector<AccountIp> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountIp entry{};

			entry.accid    = atoi(row[0]);
			entry.ip       = row[1];
			entry.count    = atoi(row[2]);
			entry.lastused = row[3];

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_ACCOUNT_IP_REPOSITORY_H
