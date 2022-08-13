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

#ifndef EQEMU_BASE_PROXIMITIES_REPOSITORY_H
#define EQEMU_BASE_PROXIMITIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseProximitiesRepository {
public:
	struct Proximities {
		int   zoneid;
		int   exploreid;
		float minx;
		float maxx;
		float miny;
		float maxy;
		float minz;
		float maxz;
	};

	static std::string PrimaryKey()
	{
		return std::string("zoneid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"zoneid",
			"exploreid",
			"minx",
			"maxx",
			"miny",
			"maxy",
			"minz",
			"maxz",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"zoneid",
			"exploreid",
			"minx",
			"maxx",
			"miny",
			"maxy",
			"minz",
			"maxz",
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
		return std::string("proximities");
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

	static Proximities NewEntity()
	{
		Proximities e{};

		e.zoneid    = 0;
		e.exploreid = 0;
		e.minx      = 0.000000;
		e.maxx      = 0.000000;
		e.miny      = 0.000000;
		e.maxy      = 0.000000;
		e.minz      = 0.000000;
		e.maxz      = 0.000000;

		return e;
	}

	static Proximities GetProximities(
		const std::vector<Proximities> &proximitiess,
		int proximities_id
	)
	{
		for (auto &proximities : proximitiess) {
			if (proximities.zoneid == proximities_id) {
				return proximities;
			}
		}

		return NewEntity();
	}

	static Proximities FindOne(
		Database& db,
		int proximities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				proximities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Proximities e{};

			e.zoneid    = atoi(row[0]);
			e.exploreid = atoi(row[1]);
			e.minx      = static_cast<float>(atof(row[2]));
			e.maxx      = static_cast<float>(atof(row[3]));
			e.miny      = static_cast<float>(atof(row[4]));
			e.maxy      = static_cast<float>(atof(row[5]));
			e.minz      = static_cast<float>(atof(row[6]));
			e.maxz      = static_cast<float>(atof(row[7]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int proximities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				proximities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Proximities &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.zoneid));
		v.push_back(columns[1] + " = " + std::to_string(e.exploreid));
		v.push_back(columns[2] + " = " + std::to_string(e.minx));
		v.push_back(columns[3] + " = " + std::to_string(e.maxx));
		v.push_back(columns[4] + " = " + std::to_string(e.miny));
		v.push_back(columns[5] + " = " + std::to_string(e.maxy));
		v.push_back(columns[6] + " = " + std::to_string(e.minz));
		v.push_back(columns[7] + " = " + std::to_string(e.maxz));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.zoneid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Proximities InsertOne(
		Database& db,
		Proximities e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.exploreid));
		v.push_back(std::to_string(e.minx));
		v.push_back(std::to_string(e.maxx));
		v.push_back(std::to_string(e.miny));
		v.push_back(std::to_string(e.maxy));
		v.push_back(std::to_string(e.minz));
		v.push_back(std::to_string(e.maxz));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.zoneid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Proximities> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.exploreid));
			v.push_back(std::to_string(e.minx));
			v.push_back(std::to_string(e.maxx));
			v.push_back(std::to_string(e.miny));
			v.push_back(std::to_string(e.maxy));
			v.push_back(std::to_string(e.minz));
			v.push_back(std::to_string(e.maxz));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Proximities> All(Database& db)
	{
		std::vector<Proximities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Proximities e{};

			e.zoneid    = atoi(row[0]);
			e.exploreid = atoi(row[1]);
			e.minx      = static_cast<float>(atof(row[2]));
			e.maxx      = static_cast<float>(atof(row[3]));
			e.miny      = static_cast<float>(atof(row[4]));
			e.maxy      = static_cast<float>(atof(row[5]));
			e.minz      = static_cast<float>(atof(row[6]));
			e.maxz      = static_cast<float>(atof(row[7]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Proximities> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Proximities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Proximities e{};

			e.zoneid    = atoi(row[0]);
			e.exploreid = atoi(row[1]);
			e.minx      = static_cast<float>(atof(row[2]));
			e.maxx      = static_cast<float>(atof(row[3]));
			e.miny      = static_cast<float>(atof(row[4]));
			e.maxy      = static_cast<float>(atof(row[5]));
			e.minz      = static_cast<float>(atof(row[6]));
			e.maxz      = static_cast<float>(atof(row[7]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_PROXIMITIES_REPOSITORY_H
