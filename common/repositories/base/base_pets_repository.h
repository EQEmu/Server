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

#ifndef EQEMU_BASE_PETS_REPOSITORY_H
#define EQEMU_BASE_PETS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BasePetsRepository {
public:
	struct Pets {
		int32_t     id;
		std::string type;
		int32_t     petpower;
		int32_t     npcID;
		int8_t      temp;
		int8_t      petcontrol;
		int8_t      petnaming;
		int8_t      monsterflag;
		int32_t     equipmentset;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"type",
			"petpower",
			"npcID",
			"temp",
			"petcontrol",
			"petnaming",
			"monsterflag",
			"equipmentset",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"type",
			"petpower",
			"npcID",
			"temp",
			"petcontrol",
			"petnaming",
			"monsterflag",
			"equipmentset",
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
		return std::string("pets");
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

	static Pets NewEntity()
	{
		Pets e{};

		e.id           = 0;
		e.type         = "";
		e.petpower     = 0;
		e.npcID        = 0;
		e.temp         = 0;
		e.petcontrol   = 0;
		e.petnaming    = 0;
		e.monsterflag  = 0;
		e.equipmentset = -1;

		return e;
	}

	static Pets GetPets(
		const std::vector<Pets> &petss,
		int pets_id
	)
	{
		for (auto &pets : petss) {
			if (pets.id == pets_id) {
				return pets;
			}
		}

		return NewEntity();
	}

	static Pets FindOne(
		Database& db,
		int pets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				pets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Pets e{};

			e.id           = static_cast<int32_t>(atoi(row[0]));
			e.type         = row[1] ? row[1] : "";
			e.petpower     = static_cast<int32_t>(atoi(row[2]));
			e.npcID        = static_cast<int32_t>(atoi(row[3]));
			e.temp         = static_cast<int8_t>(atoi(row[4]));
			e.petcontrol   = static_cast<int8_t>(atoi(row[5]));
			e.petnaming    = static_cast<int8_t>(atoi(row[6]));
			e.monsterflag  = static_cast<int8_t>(atoi(row[7]));
			e.equipmentset = static_cast<int32_t>(atoi(row[8]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int pets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				pets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Pets &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.type) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.petpower));
		v.push_back(columns[3] + " = " + std::to_string(e.npcID));
		v.push_back(columns[4] + " = " + std::to_string(e.temp));
		v.push_back(columns[5] + " = " + std::to_string(e.petcontrol));
		v.push_back(columns[6] + " = " + std::to_string(e.petnaming));
		v.push_back(columns[7] + " = " + std::to_string(e.monsterflag));
		v.push_back(columns[8] + " = " + std::to_string(e.equipmentset));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Pets InsertOne(
		Database& db,
		Pets e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.type) + "'");
		v.push_back(std::to_string(e.petpower));
		v.push_back(std::to_string(e.npcID));
		v.push_back(std::to_string(e.temp));
		v.push_back(std::to_string(e.petcontrol));
		v.push_back(std::to_string(e.petnaming));
		v.push_back(std::to_string(e.monsterflag));
		v.push_back(std::to_string(e.equipmentset));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Pets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.type) + "'");
			v.push_back(std::to_string(e.petpower));
			v.push_back(std::to_string(e.npcID));
			v.push_back(std::to_string(e.temp));
			v.push_back(std::to_string(e.petcontrol));
			v.push_back(std::to_string(e.petnaming));
			v.push_back(std::to_string(e.monsterflag));
			v.push_back(std::to_string(e.equipmentset));

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

	static std::vector<Pets> All(Database& db)
	{
		std::vector<Pets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Pets e{};

			e.id           = static_cast<int32_t>(atoi(row[0]));
			e.type         = row[1] ? row[1] : "";
			e.petpower     = static_cast<int32_t>(atoi(row[2]));
			e.npcID        = static_cast<int32_t>(atoi(row[3]));
			e.temp         = static_cast<int8_t>(atoi(row[4]));
			e.petcontrol   = static_cast<int8_t>(atoi(row[5]));
			e.petnaming    = static_cast<int8_t>(atoi(row[6]));
			e.monsterflag  = static_cast<int8_t>(atoi(row[7]));
			e.equipmentset = static_cast<int32_t>(atoi(row[8]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Pets> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Pets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Pets e{};

			e.id           = static_cast<int32_t>(atoi(row[0]));
			e.type         = row[1] ? row[1] : "";
			e.petpower     = static_cast<int32_t>(atoi(row[2]));
			e.npcID        = static_cast<int32_t>(atoi(row[3]));
			e.temp         = static_cast<int8_t>(atoi(row[4]));
			e.petcontrol   = static_cast<int8_t>(atoi(row[5]));
			e.petnaming    = static_cast<int8_t>(atoi(row[6]));
			e.monsterflag  = static_cast<int8_t>(atoi(row[7]));
			e.equipmentset = static_cast<int32_t>(atoi(row[8]));

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

#endif //EQEMU_BASE_PETS_REPOSITORY_H
