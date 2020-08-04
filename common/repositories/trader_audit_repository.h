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

#ifndef EQEMU_TRADER_AUDIT_REPOSITORY_H
#define EQEMU_TRADER_AUDIT_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class TraderAuditRepository {
public:
	struct TraderAudit {
		std::string time;
		std::string seller;
		std::string buyer;
		std::string itemname;
		int         quantity;
		int         totalcost;
		int8        trantype;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"time",
			"seller",
			"buyer",
			"itemname",
			"quantity",
			"totalcost",
			"trantype",
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
		return std::string("trader_audit");
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

	static TraderAudit NewEntity()
	{
		TraderAudit entry{};

		entry.time      = '0000-00-00 00:00:00';
		entry.seller    = "";
		entry.buyer     = "";
		entry.itemname  = "";
		entry.quantity  = 0;
		entry.totalcost = 0;
		entry.trantype  = 0;

		return entry;
	}

	static TraderAudit GetTraderAuditEntry(
		const std::vector<TraderAudit> &trader_audits,
		int trader_audit_id
	)
	{
		for (auto &trader_audit : trader_audits) {
			if (trader_audit. == trader_audit_id) {
				return trader_audit;
			}
		}

		return NewEntity();
	}

	static TraderAudit FindOne(
		int trader_audit_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				trader_audit_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			TraderAudit entry{};

			entry.time      = row[0];
			entry.seller    = row[1];
			entry.buyer     = row[2];
			entry.itemname  = row[3];
			entry.quantity  = atoi(row[4]);
			entry.totalcost = atoi(row[5]);
			entry.trantype  = atoi(row[6]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int trader_audit_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				trader_audit_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		TraderAudit trader_audit_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = '" + EscapeString(trader_audit_entry.time) + "'");
		update_values.push_back(columns[1] + " = '" + EscapeString(trader_audit_entry.seller) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(trader_audit_entry.buyer) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(trader_audit_entry.itemname) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(trader_audit_entry.quantity));
		update_values.push_back(columns[5] + " = " + std::to_string(trader_audit_entry.totalcost));
		update_values.push_back(columns[6] + " = " + std::to_string(trader_audit_entry.trantype));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				trader_audit_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static TraderAudit InsertOne(
		TraderAudit trader_audit_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(trader_audit_entry.time) + "'");
		insert_values.push_back("'" + EscapeString(trader_audit_entry.seller) + "'");
		insert_values.push_back("'" + EscapeString(trader_audit_entry.buyer) + "'");
		insert_values.push_back("'" + EscapeString(trader_audit_entry.itemname) + "'");
		insert_values.push_back(std::to_string(trader_audit_entry.quantity));
		insert_values.push_back(std::to_string(trader_audit_entry.totalcost));
		insert_values.push_back(std::to_string(trader_audit_entry.trantype));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			trader_audit_entry.id = results.LastInsertedID();
			return trader_audit_entry;
		}

		trader_audit_entry = InstanceListRepository::NewEntity();

		return trader_audit_entry;
	}

	static int InsertMany(
		std::vector<TraderAudit> trader_audit_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &trader_audit_entry: trader_audit_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(trader_audit_entry.time) + "'");
			insert_values.push_back("'" + EscapeString(trader_audit_entry.seller) + "'");
			insert_values.push_back("'" + EscapeString(trader_audit_entry.buyer) + "'");
			insert_values.push_back("'" + EscapeString(trader_audit_entry.itemname) + "'");
			insert_values.push_back(std::to_string(trader_audit_entry.quantity));
			insert_values.push_back(std::to_string(trader_audit_entry.totalcost));
			insert_values.push_back(std::to_string(trader_audit_entry.trantype));

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

	static std::vector<TraderAudit> All()
	{
		std::vector<TraderAudit> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TraderAudit entry{};

			entry.time      = row[0];
			entry.seller    = row[1];
			entry.buyer     = row[2];
			entry.itemname  = row[3];
			entry.quantity  = atoi(row[4]);
			entry.totalcost = atoi(row[5]);
			entry.trantype  = atoi(row[6]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_TRADER_AUDIT_REPOSITORY_H
