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

#ifndef EQEMU_BASE_MAIL_REPOSITORY_H
#define EQEMU_BASE_MAIL_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseMailRepository {
public:
	struct Mail {
		int         msgid;
		int         charid;
		int         timestamp;
		std::string from;
		std::string subject;
		std::string body;
		std::string to;
		int         status;
	};

	static std::string PrimaryKey()
	{
		return std::string("msgid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"msgid",
			"charid",
			"timestamp",
			"from",
			"subject",
			"body",
			"to",
			"status",
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
		return std::string("mail");
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

	static Mail NewEntity()
	{
		Mail entry{};

		entry.msgid     = 0;
		entry.charid    = 0;
		entry.timestamp = 0;
		entry.from      = "";
		entry.subject   = "";
		entry.body      = "";
		entry.to        = "";
		entry.status    = 0;

		return entry;
	}

	static Mail GetMailEntry(
		const std::vector<Mail> &mails,
		int mail_id
	)
	{
		for (auto &mail : mails) {
			if (mail.msgid == mail_id) {
				return mail;
			}
		}

		return NewEntity();
	}

	static Mail FindOne(
		int mail_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				mail_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Mail entry{};

			entry.msgid     = atoi(row[0]);
			entry.charid    = atoi(row[1]);
			entry.timestamp = atoi(row[2]);
			entry.from      = row[3] ? row[3] : "";
			entry.subject   = row[4] ? row[4] : "";
			entry.body      = row[5] ? row[5] : "";
			entry.to        = row[6] ? row[6] : "";
			entry.status    = atoi(row[7]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int mail_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				mail_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Mail mail_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(mail_entry.charid));
		update_values.push_back(columns[2] + " = " + std::to_string(mail_entry.timestamp));
		update_values.push_back(columns[3] + " = '" + EscapeString(mail_entry.from) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(mail_entry.subject) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(mail_entry.body) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(mail_entry.to) + "'");
		update_values.push_back(columns[7] + " = " + std::to_string(mail_entry.status));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				mail_entry.msgid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Mail InsertOne(
		Mail mail_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(mail_entry.charid));
		insert_values.push_back(std::to_string(mail_entry.timestamp));
		insert_values.push_back("'" + EscapeString(mail_entry.from) + "'");
		insert_values.push_back("'" + EscapeString(mail_entry.subject) + "'");
		insert_values.push_back("'" + EscapeString(mail_entry.body) + "'");
		insert_values.push_back("'" + EscapeString(mail_entry.to) + "'");
		insert_values.push_back(std::to_string(mail_entry.status));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			mail_entry.msgid = results.LastInsertedID();
			return mail_entry;
		}

		mail_entry = NewEntity();

		return mail_entry;
	}

	static int InsertMany(
		std::vector<Mail> mail_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &mail_entry: mail_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(mail_entry.charid));
			insert_values.push_back(std::to_string(mail_entry.timestamp));
			insert_values.push_back("'" + EscapeString(mail_entry.from) + "'");
			insert_values.push_back("'" + EscapeString(mail_entry.subject) + "'");
			insert_values.push_back("'" + EscapeString(mail_entry.body) + "'");
			insert_values.push_back("'" + EscapeString(mail_entry.to) + "'");
			insert_values.push_back(std::to_string(mail_entry.status));

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

	static std::vector<Mail> All()
	{
		std::vector<Mail> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Mail entry{};

			entry.msgid     = atoi(row[0]);
			entry.charid    = atoi(row[1]);
			entry.timestamp = atoi(row[2]);
			entry.from      = row[3] ? row[3] : "";
			entry.subject   = row[4] ? row[4] : "";
			entry.body      = row[5] ? row[5] : "";
			entry.to        = row[6] ? row[6] : "";
			entry.status    = atoi(row[7]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Mail> GetWhere(std::string where_filter)
	{
		std::vector<Mail> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Mail entry{};

			entry.msgid     = atoi(row[0]);
			entry.charid    = atoi(row[1]);
			entry.timestamp = atoi(row[2]);
			entry.from      = row[3] ? row[3] : "";
			entry.subject   = row[4] ? row[4] : "";
			entry.body      = row[5] ? row[5] : "";
			entry.to        = row[6] ? row[6] : "";
			entry.status    = atoi(row[7]);

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
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_MAIL_REPOSITORY_H
