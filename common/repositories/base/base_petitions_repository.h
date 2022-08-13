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

#ifndef EQEMU_BASE_PETITIONS_REPOSITORY_H
#define EQEMU_BASE_PETITIONS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BasePetitionsRepository {
public:
	struct Petitions {
		int         dib;
		int         petid;
		std::string charname;
		std::string accountname;
		std::string lastgm;
		std::string petitiontext;
		std::string gmtext;
		std::string zone;
		int         urgency;
		int         charclass;
		int         charrace;
		int         charlevel;
		int         checkouts;
		int         unavailables;
		int         ischeckedout;
		int64       senttime;
	};

	static std::string PrimaryKey()
	{
		return std::string("dib");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"dib",
			"petid",
			"charname",
			"accountname",
			"lastgm",
			"petitiontext",
			"gmtext",
			"zone",
			"urgency",
			"charclass",
			"charrace",
			"charlevel",
			"checkouts",
			"unavailables",
			"ischeckedout",
			"senttime",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"dib",
			"petid",
			"charname",
			"accountname",
			"lastgm",
			"petitiontext",
			"gmtext",
			"zone",
			"urgency",
			"charclass",
			"charrace",
			"charlevel",
			"checkouts",
			"unavailables",
			"ischeckedout",
			"senttime",
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
		return std::string("petitions");
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

	static Petitions NewEntity()
	{
		Petitions e{};

		e.dib          = 0;
		e.petid        = 0;
		e.charname     = "";
		e.accountname  = "";
		e.lastgm       = "";
		e.petitiontext = "";
		e.gmtext       = "";
		e.zone         = "";
		e.urgency      = 0;
		e.charclass    = 0;
		e.charrace     = 0;
		e.charlevel    = 0;
		e.checkouts    = 0;
		e.unavailables = 0;
		e.ischeckedout = 0;
		e.senttime     = 0;

		return e;
	}

	static Petitions GetPetitionse(
		const std::vector<Petitions> &petitionss,
		int petitions_id
	)
	{
		for (auto &petitions : petitionss) {
			if (petitions.dib == petitions_id) {
				return petitions;
			}
		}

		return NewEntity();
	}

	static Petitions FindOne(
		Database& db,
		int petitions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				petitions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Petitions e{};

			e.dib          = atoi(row[0]);
			e.petid        = atoi(row[1]);
			e.charname     = row[2] ? row[2] : "";
			e.accountname  = row[3] ? row[3] : "";
			e.lastgm       = row[4] ? row[4] : "";
			e.petitiontext = row[5] ? row[5] : "";
			e.gmtext       = row[6] ? row[6] : "";
			e.zone         = row[7] ? row[7] : "";
			e.urgency      = atoi(row[8]);
			e.charclass    = atoi(row[9]);
			e.charrace     = atoi(row[10]);
			e.charlevel    = atoi(row[11]);
			e.checkouts    = atoi(row[12]);
			e.unavailables = atoi(row[13]);
			e.ischeckedout = atoi(row[14]);
			e.senttime     = strtoll(row[15], nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int petitions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				petitions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Petitions e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(e.petid));
		update_values.push_back(columns[2] + " = '" + Strings::Escape(e.charname) + "'");
		update_values.push_back(columns[3] + " = '" + Strings::Escape(e.accountname) + "'");
		update_values.push_back(columns[4] + " = '" + Strings::Escape(e.lastgm) + "'");
		update_values.push_back(columns[5] + " = '" + Strings::Escape(e.petitiontext) + "'");
		update_values.push_back(columns[6] + " = '" + Strings::Escape(e.gmtext) + "'");
		update_values.push_back(columns[7] + " = '" + Strings::Escape(e.zone) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(e.urgency));
		update_values.push_back(columns[9] + " = " + std::to_string(e.charclass));
		update_values.push_back(columns[10] + " = " + std::to_string(e.charrace));
		update_values.push_back(columns[11] + " = " + std::to_string(e.charlevel));
		update_values.push_back(columns[12] + " = " + std::to_string(e.checkouts));
		update_values.push_back(columns[13] + " = " + std::to_string(e.unavailables));
		update_values.push_back(columns[14] + " = " + std::to_string(e.ischeckedout));
		update_values.push_back(columns[15] + " = " + std::to_string(e.senttime));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				e.dib
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Petitions InsertOne(
		Database& db,
		Petitions e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(e.dib));
		insert_values.push_back(std::to_string(e.petid));
		insert_values.push_back("'" + Strings::Escape(e.charname) + "'");
		insert_values.push_back("'" + Strings::Escape(e.accountname) + "'");
		insert_values.push_back("'" + Strings::Escape(e.lastgm) + "'");
		insert_values.push_back("'" + Strings::Escape(e.petitiontext) + "'");
		insert_values.push_back("'" + Strings::Escape(e.gmtext) + "'");
		insert_values.push_back("'" + Strings::Escape(e.zone) + "'");
		insert_values.push_back(std::to_string(e.urgency));
		insert_values.push_back(std::to_string(e.charclass));
		insert_values.push_back(std::to_string(e.charrace));
		insert_values.push_back(std::to_string(e.charlevel));
		insert_values.push_back(std::to_string(e.checkouts));
		insert_values.push_back(std::to_string(e.unavailables));
		insert_values.push_back(std::to_string(e.ischeckedout));
		insert_values.push_back(std::to_string(e.senttime));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			e.dib = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Petitions> entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(e.dib));
			insert_values.push_back(std::to_string(e.petid));
			insert_values.push_back("'" + Strings::Escape(e.charname) + "'");
			insert_values.push_back("'" + Strings::Escape(e.accountname) + "'");
			insert_values.push_back("'" + Strings::Escape(e.lastgm) + "'");
			insert_values.push_back("'" + Strings::Escape(e.petitiontext) + "'");
			insert_values.push_back("'" + Strings::Escape(e.gmtext) + "'");
			insert_values.push_back("'" + Strings::Escape(e.zone) + "'");
			insert_values.push_back(std::to_string(e.urgency));
			insert_values.push_back(std::to_string(e.charclass));
			insert_values.push_back(std::to_string(e.charrace));
			insert_values.push_back(std::to_string(e.charlevel));
			insert_values.push_back(std::to_string(e.checkouts));
			insert_values.push_back(std::to_string(e.unavailables));
			insert_values.push_back(std::to_string(e.ischeckedout));
			insert_values.push_back(std::to_string(e.senttime));

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

	static std::vector<Petitions> All(Database& db)
	{
		std::vector<Petitions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Petitions e{};

			e.dib          = atoi(row[0]);
			e.petid        = atoi(row[1]);
			e.charname     = row[2] ? row[2] : "";
			e.accountname  = row[3] ? row[3] : "";
			e.lastgm       = row[4] ? row[4] : "";
			e.petitiontext = row[5] ? row[5] : "";
			e.gmtext       = row[6] ? row[6] : "";
			e.zone         = row[7] ? row[7] : "";
			e.urgency      = atoi(row[8]);
			e.charclass    = atoi(row[9]);
			e.charrace     = atoi(row[10]);
			e.charlevel    = atoi(row[11]);
			e.checkouts    = atoi(row[12]);
			e.unavailables = atoi(row[13]);
			e.ischeckedout = atoi(row[14]);
			e.senttime     = strtoll(row[15], nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Petitions> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Petitions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Petitions e{};

			e.dib          = atoi(row[0]);
			e.petid        = atoi(row[1]);
			e.charname     = row[2] ? row[2] : "";
			e.accountname  = row[3] ? row[3] : "";
			e.lastgm       = row[4] ? row[4] : "";
			e.petitiontext = row[5] ? row[5] : "";
			e.gmtext       = row[6] ? row[6] : "";
			e.zone         = row[7] ? row[7] : "";
			e.urgency      = atoi(row[8]);
			e.charclass    = atoi(row[9]);
			e.charrace     = atoi(row[10]);
			e.charlevel    = atoi(row[11]);
			e.checkouts    = atoi(row[12]);
			e.unavailables = atoi(row[13]);
			e.ischeckedout = atoi(row[14]);
			e.senttime     = strtoll(row[15], nullptr, 10);

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

#endif //EQEMU_BASE_PETITIONS_REPOSITORY_H
