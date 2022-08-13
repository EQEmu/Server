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

#ifndef EQEMU_BASE_MAIL_REPOSITORY_H
#define EQEMU_BASE_MAIL_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("mail");
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

	static Mail NewEntity()
	{
		Mail e{};

		e.msgid     = 0;
		e.charid    = 0;
		e.timestamp = 0;
		e.from      = "";
		e.subject   = "";
		e.body      = "";
		e.to        = "";
		e.status    = 0;

		return e;
	}

	static Mail GetMaile(
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
		Database& db,
		int mail_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				mail_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Mail e{};

			e.msgid     = atoi(row[0]);
			e.charid    = atoi(row[1]);
			e.timestamp = atoi(row[2]);
			e.from      = row[3] ? row[3] : "";
			e.subject   = row[4] ? row[4] : "";
			e.body      = row[5] ? row[5] : "";
			e.to        = row[6] ? row[6] : "";
			e.status    = atoi(row[7]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int mail_id
	)
	{
		auto results = db.QueryDatabase(
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
		Database& db,
		Mail mail_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(mail_e.charid));
		update_values.push_back(columns[2] + " = " + std::to_string(mail_e.timestamp));
		update_values.push_back(columns[3] + " = '" + Strings::Escape(mail_e.from) + "'");
		update_values.push_back(columns[4] + " = '" + Strings::Escape(mail_e.subject) + "'");
		update_values.push_back(columns[5] + " = '" + Strings::Escape(mail_e.body) + "'");
		update_values.push_back(columns[6] + " = '" + Strings::Escape(mail_e.to) + "'");
		update_values.push_back(columns[7] + " = " + std::to_string(mail_e.status));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				mail_e.msgid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Mail InsertOne(
		Database& db,
		Mail mail_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(mail_e.msgid));
		insert_values.push_back(std::to_string(mail_e.charid));
		insert_values.push_back(std::to_string(mail_e.timestamp));
		insert_values.push_back("'" + Strings::Escape(mail_e.from) + "'");
		insert_values.push_back("'" + Strings::Escape(mail_e.subject) + "'");
		insert_values.push_back("'" + Strings::Escape(mail_e.body) + "'");
		insert_values.push_back("'" + Strings::Escape(mail_e.to) + "'");
		insert_values.push_back(std::to_string(mail_e.status));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			mail_e.msgid = results.LastInsertedID();
			return mail_e;
		}

		mail_e = NewEntity();

		return mail_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Mail> mail_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &mail_e: mail_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(mail_e.msgid));
			insert_values.push_back(std::to_string(mail_e.charid));
			insert_values.push_back(std::to_string(mail_e.timestamp));
			insert_values.push_back("'" + Strings::Escape(mail_e.from) + "'");
			insert_values.push_back("'" + Strings::Escape(mail_e.subject) + "'");
			insert_values.push_back("'" + Strings::Escape(mail_e.body) + "'");
			insert_values.push_back("'" + Strings::Escape(mail_e.to) + "'");
			insert_values.push_back(std::to_string(mail_e.status));

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

	static std::vector<Mail> All(Database& db)
	{
		std::vector<Mail> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Mail e{};

			e.msgid     = atoi(row[0]);
			e.charid    = atoi(row[1]);
			e.timestamp = atoi(row[2]);
			e.from      = row[3] ? row[3] : "";
			e.subject   = row[4] ? row[4] : "";
			e.body      = row[5] ? row[5] : "";
			e.to        = row[6] ? row[6] : "";
			e.status    = atoi(row[7]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Mail> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Mail> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Mail e{};

			e.msgid     = atoi(row[0]);
			e.charid    = atoi(row[1]);
			e.timestamp = atoi(row[2]);
			e.from      = row[3] ? row[3] : "";
			e.subject   = row[4] ? row[4] : "";
			e.body      = row[5] ? row[5] : "";
			e.to        = row[6] ? row[6] : "";
			e.status    = atoi(row[7]);

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

#endif //EQEMU_BASE_MAIL_REPOSITORY_H
