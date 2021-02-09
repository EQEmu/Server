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
#include "../../string_util.h"

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
		int         senttime;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("petitions");
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

	static Petitions NewEntity()
	{
		Petitions entry{};

		entry.dib          = 0;
		entry.petid        = 0;
		entry.charname     = "";
		entry.accountname  = "";
		entry.lastgm       = "";
		entry.petitiontext = "";
		entry.gmtext       = "";
		entry.zone         = "";
		entry.urgency      = 0;
		entry.charclass    = 0;
		entry.charrace     = 0;
		entry.charlevel    = 0;
		entry.checkouts    = 0;
		entry.unavailables = 0;
		entry.ischeckedout = 0;
		entry.senttime     = 0;

		return entry;
	}

	static Petitions GetPetitionsEntry(
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
			Petitions entry{};

			entry.dib          = atoi(row[0]);
			entry.petid        = atoi(row[1]);
			entry.charname     = row[2] ? row[2] : "";
			entry.accountname  = row[3] ? row[3] : "";
			entry.lastgm       = row[4] ? row[4] : "";
			entry.petitiontext = row[5] ? row[5] : "";
			entry.gmtext       = row[6] ? row[6] : "";
			entry.zone         = row[7] ? row[7] : "";
			entry.urgency      = atoi(row[8]);
			entry.charclass    = atoi(row[9]);
			entry.charrace     = atoi(row[10]);
			entry.charlevel    = atoi(row[11]);
			entry.checkouts    = atoi(row[12]);
			entry.unavailables = atoi(row[13]);
			entry.ischeckedout = atoi(row[14]);
			entry.senttime     = atoi(row[15]);

			return entry;
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
		Petitions petitions_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(petitions_entry.petid));
		update_values.push_back(columns[2] + " = '" + EscapeString(petitions_entry.charname) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(petitions_entry.accountname) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(petitions_entry.lastgm) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(petitions_entry.petitiontext) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(petitions_entry.gmtext) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(petitions_entry.zone) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(petitions_entry.urgency));
		update_values.push_back(columns[9] + " = " + std::to_string(petitions_entry.charclass));
		update_values.push_back(columns[10] + " = " + std::to_string(petitions_entry.charrace));
		update_values.push_back(columns[11] + " = " + std::to_string(petitions_entry.charlevel));
		update_values.push_back(columns[12] + " = " + std::to_string(petitions_entry.checkouts));
		update_values.push_back(columns[13] + " = " + std::to_string(petitions_entry.unavailables));
		update_values.push_back(columns[14] + " = " + std::to_string(petitions_entry.ischeckedout));
		update_values.push_back(columns[15] + " = " + std::to_string(petitions_entry.senttime));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				petitions_entry.dib
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Petitions InsertOne(
		Database& db,
		Petitions petitions_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(petitions_entry.dib));
		insert_values.push_back(std::to_string(petitions_entry.petid));
		insert_values.push_back("'" + EscapeString(petitions_entry.charname) + "'");
		insert_values.push_back("'" + EscapeString(petitions_entry.accountname) + "'");
		insert_values.push_back("'" + EscapeString(petitions_entry.lastgm) + "'");
		insert_values.push_back("'" + EscapeString(petitions_entry.petitiontext) + "'");
		insert_values.push_back("'" + EscapeString(petitions_entry.gmtext) + "'");
		insert_values.push_back("'" + EscapeString(petitions_entry.zone) + "'");
		insert_values.push_back(std::to_string(petitions_entry.urgency));
		insert_values.push_back(std::to_string(petitions_entry.charclass));
		insert_values.push_back(std::to_string(petitions_entry.charrace));
		insert_values.push_back(std::to_string(petitions_entry.charlevel));
		insert_values.push_back(std::to_string(petitions_entry.checkouts));
		insert_values.push_back(std::to_string(petitions_entry.unavailables));
		insert_values.push_back(std::to_string(petitions_entry.ischeckedout));
		insert_values.push_back(std::to_string(petitions_entry.senttime));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			petitions_entry.dib = results.LastInsertedID();
			return petitions_entry;
		}

		petitions_entry = NewEntity();

		return petitions_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Petitions> petitions_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &petitions_entry: petitions_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(petitions_entry.dib));
			insert_values.push_back(std::to_string(petitions_entry.petid));
			insert_values.push_back("'" + EscapeString(petitions_entry.charname) + "'");
			insert_values.push_back("'" + EscapeString(petitions_entry.accountname) + "'");
			insert_values.push_back("'" + EscapeString(petitions_entry.lastgm) + "'");
			insert_values.push_back("'" + EscapeString(petitions_entry.petitiontext) + "'");
			insert_values.push_back("'" + EscapeString(petitions_entry.gmtext) + "'");
			insert_values.push_back("'" + EscapeString(petitions_entry.zone) + "'");
			insert_values.push_back(std::to_string(petitions_entry.urgency));
			insert_values.push_back(std::to_string(petitions_entry.charclass));
			insert_values.push_back(std::to_string(petitions_entry.charrace));
			insert_values.push_back(std::to_string(petitions_entry.charlevel));
			insert_values.push_back(std::to_string(petitions_entry.checkouts));
			insert_values.push_back(std::to_string(petitions_entry.unavailables));
			insert_values.push_back(std::to_string(petitions_entry.ischeckedout));
			insert_values.push_back(std::to_string(petitions_entry.senttime));

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
			Petitions entry{};

			entry.dib          = atoi(row[0]);
			entry.petid        = atoi(row[1]);
			entry.charname     = row[2] ? row[2] : "";
			entry.accountname  = row[3] ? row[3] : "";
			entry.lastgm       = row[4] ? row[4] : "";
			entry.petitiontext = row[5] ? row[5] : "";
			entry.gmtext       = row[6] ? row[6] : "";
			entry.zone         = row[7] ? row[7] : "";
			entry.urgency      = atoi(row[8]);
			entry.charclass    = atoi(row[9]);
			entry.charrace     = atoi(row[10]);
			entry.charlevel    = atoi(row[11]);
			entry.checkouts    = atoi(row[12]);
			entry.unavailables = atoi(row[13]);
			entry.ischeckedout = atoi(row[14]);
			entry.senttime     = atoi(row[15]);

			all_entries.push_back(entry);
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
			Petitions entry{};

			entry.dib          = atoi(row[0]);
			entry.petid        = atoi(row[1]);
			entry.charname     = row[2] ? row[2] : "";
			entry.accountname  = row[3] ? row[3] : "";
			entry.lastgm       = row[4] ? row[4] : "";
			entry.petitiontext = row[5] ? row[5] : "";
			entry.gmtext       = row[6] ? row[6] : "";
			entry.zone         = row[7] ? row[7] : "";
			entry.urgency      = atoi(row[8]);
			entry.charclass    = atoi(row[9]);
			entry.charrace     = atoi(row[10]);
			entry.charlevel    = atoi(row[11]);
			entry.checkouts    = atoi(row[12]);
			entry.unavailables = atoi(row[13]);
			entry.ischeckedout = atoi(row[14]);
			entry.senttime     = atoi(row[15]);

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

#endif //EQEMU_BASE_PETITIONS_REPOSITORY_H
