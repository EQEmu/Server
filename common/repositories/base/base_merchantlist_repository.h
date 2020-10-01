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
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_MERCHANTLIST_REPOSITORY_H
#define EQEMU_BASE_MERCHANTLIST_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseMerchantlistRepository {
public:
	struct Merchantlist {
		int         merchantid;
		int         slot;
		int         item;
		int         faction_required;
		int         level_required;
		int         alt_currency_cost;
		int         classes_required;
		int         probability;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("merchantid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merchantid",
			"slot",
			"item",
			"faction_required",
			"level_required",
			"alt_currency_cost",
			"classes_required",
			"probability",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("merchantlist");
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

	static Merchantlist NewEntity()
	{
		Merchantlist entry{};

		entry.merchantid             = 0;
		entry.slot                   = 0;
		entry.item                   = 0;
		entry.faction_required       = -100;
		entry.level_required         = 0;
		entry.alt_currency_cost      = 0;
		entry.classes_required       = 65535;
		entry.probability            = 100;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Merchantlist GetMerchantlistEntry(
		const std::vector<Merchantlist> &merchantlists,
		int merchantlist_id
	)
	{
		for (auto &merchantlist : merchantlists) {
			if (merchantlist.merchantid == merchantlist_id) {
				return merchantlist;
			}
		}

		return NewEntity();
	}

	static Merchantlist FindOne(
		int merchantlist_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				merchantlist_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Merchantlist entry{};

			entry.merchantid             = atoi(row[0]);
			entry.slot                   = atoi(row[1]);
			entry.item                   = atoi(row[2]);
			entry.faction_required       = atoi(row[3]);
			entry.level_required         = atoi(row[4]);
			entry.alt_currency_cost      = atoi(row[5]);
			entry.classes_required       = atoi(row[6]);
			entry.probability            = atoi(row[7]);
			entry.min_expansion          = atoi(row[8]);
			entry.max_expansion          = atoi(row[9]);
			entry.content_flags          = row[10] ? row[10] : "";
			entry.content_flags_disabled = row[11] ? row[11] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int merchantlist_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merchantlist_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Merchantlist merchantlist_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(merchantlist_entry.merchantid));
		update_values.push_back(columns[1] + " = " + std::to_string(merchantlist_entry.slot));
		update_values.push_back(columns[2] + " = " + std::to_string(merchantlist_entry.item));
		update_values.push_back(columns[3] + " = " + std::to_string(merchantlist_entry.faction_required));
		update_values.push_back(columns[4] + " = " + std::to_string(merchantlist_entry.level_required));
		update_values.push_back(columns[5] + " = " + std::to_string(merchantlist_entry.alt_currency_cost));
		update_values.push_back(columns[6] + " = " + std::to_string(merchantlist_entry.classes_required));
		update_values.push_back(columns[7] + " = " + std::to_string(merchantlist_entry.probability));
		update_values.push_back(columns[8] + " = " + std::to_string(merchantlist_entry.min_expansion));
		update_values.push_back(columns[9] + " = " + std::to_string(merchantlist_entry.max_expansion));
		update_values.push_back(columns[10] + " = '" + EscapeString(merchantlist_entry.content_flags) + "'");
		update_values.push_back(columns[11] + " = '" + EscapeString(merchantlist_entry.content_flags_disabled) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				merchantlist_entry.merchantid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Merchantlist InsertOne(
		Merchantlist merchantlist_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(merchantlist_entry.merchantid));
		insert_values.push_back(std::to_string(merchantlist_entry.slot));
		insert_values.push_back(std::to_string(merchantlist_entry.item));
		insert_values.push_back(std::to_string(merchantlist_entry.faction_required));
		insert_values.push_back(std::to_string(merchantlist_entry.level_required));
		insert_values.push_back(std::to_string(merchantlist_entry.alt_currency_cost));
		insert_values.push_back(std::to_string(merchantlist_entry.classes_required));
		insert_values.push_back(std::to_string(merchantlist_entry.probability));
		insert_values.push_back(std::to_string(merchantlist_entry.min_expansion));
		insert_values.push_back(std::to_string(merchantlist_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(merchantlist_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(merchantlist_entry.content_flags_disabled) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			merchantlist_entry.merchantid = results.LastInsertedID();
			return merchantlist_entry;
		}

		merchantlist_entry = NewEntity();

		return merchantlist_entry;
	}

	static int InsertMany(
		std::vector<Merchantlist> merchantlist_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &merchantlist_entry: merchantlist_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(merchantlist_entry.merchantid));
			insert_values.push_back(std::to_string(merchantlist_entry.slot));
			insert_values.push_back(std::to_string(merchantlist_entry.item));
			insert_values.push_back(std::to_string(merchantlist_entry.faction_required));
			insert_values.push_back(std::to_string(merchantlist_entry.level_required));
			insert_values.push_back(std::to_string(merchantlist_entry.alt_currency_cost));
			insert_values.push_back(std::to_string(merchantlist_entry.classes_required));
			insert_values.push_back(std::to_string(merchantlist_entry.probability));
			insert_values.push_back(std::to_string(merchantlist_entry.min_expansion));
			insert_values.push_back(std::to_string(merchantlist_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(merchantlist_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(merchantlist_entry.content_flags_disabled) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Merchantlist> All()
	{
		std::vector<Merchantlist> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Merchantlist entry{};

			entry.merchantid             = atoi(row[0]);
			entry.slot                   = atoi(row[1]);
			entry.item                   = atoi(row[2]);
			entry.faction_required       = atoi(row[3]);
			entry.level_required         = atoi(row[4]);
			entry.alt_currency_cost      = atoi(row[5]);
			entry.classes_required       = atoi(row[6]);
			entry.probability            = atoi(row[7]);
			entry.min_expansion          = atoi(row[8]);
			entry.max_expansion          = atoi(row[9]);
			entry.content_flags          = row[10] ? row[10] : "";
			entry.content_flags_disabled = row[11] ? row[11] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Merchantlist> GetWhere(std::string where_filter)
	{
		std::vector<Merchantlist> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Merchantlist entry{};

			entry.merchantid             = atoi(row[0]);
			entry.slot                   = atoi(row[1]);
			entry.item                   = atoi(row[2]);
			entry.faction_required       = atoi(row[3]);
			entry.level_required         = atoi(row[4]);
			entry.alt_currency_cost      = atoi(row[5]);
			entry.classes_required       = atoi(row[6]);
			entry.probability            = atoi(row[7]);
			entry.min_expansion          = atoi(row[8]);
			entry.max_expansion          = atoi(row[9]);
			entry.content_flags          = row[10] ? row[10] : "";
			entry.content_flags_disabled = row[11] ? row[11] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_MERCHANTLIST_REPOSITORY_H
