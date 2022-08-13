#ifndef EQEMU_RAID_LEADERS_REPOSITORY_H
#define EQEMU_RAID_LEADERS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"

class RaidLeadersRepository {
public:
	struct RaidLeaders {
		int         gid;
		int         rid;
		std::string marknpc;
		std::string maintank;
		std::string assist;
		std::string puller;
		std::string leadershipaa;
		std::string mentoree;
		int         mentor_percent;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"gid",
			"rid",
			"marknpc",
			"maintank",
			"assist",
			"puller",
			"leadershipaa",
			"mentoree",
			"mentor_percent",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
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

		return std::string(Strings::Implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("raid_leaders");
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

	static RaidLeaders NewEntity()
	{
		RaidLeaders entry{};

		entry.gid            = 0;
		entry.rid            = 0;
		entry.marknpc        = "";
		entry.maintank       = "";
		entry.assist         = "";
		entry.puller         = "";
		entry.leadershipaa   = 0;
		entry.mentoree       = "";
		entry.mentor_percent = 0;

		return entry;
	}

	static RaidLeaders GetRaidLeadersEntry(
		const std::vector<RaidLeaders> &raid_leaderss,
		int raid_leaders_id
	)
	{
		for (auto &raid_leaders : raid_leaderss) {
			if (raid_leaders. == raid_leaders_id) {
				return raid_leaders;
			}
		}

		return NewEntity();
	}

	static RaidLeaders FindOne(
		int raid_leaders_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				raid_leaders_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RaidLeaders entry{};

			entry.gid            = atoi(row[0]);
			entry.rid            = atoi(row[1]);
			entry.marknpc        = row[2];
			entry.maintank       = row[3];
			entry.assist         = row[4];
			entry.puller         = row[5];
			entry.leadershipaa   = row[6];
			entry.mentoree       = row[7];
			entry.mentor_percent = atoi(row[8]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int raid_leaders_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				raid_leaders_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		RaidLeaders raid_leaders_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(raid_leaders_entry.gid));
		update_values.push_back(columns[1] + " = " + std::to_string(raid_leaders_entry.rid));
		update_values.push_back(columns[2] + " = '" + Strings::Escape(raid_leaders_entry.marknpc) + "'");
		update_values.push_back(columns[3] + " = '" + Strings::Escape(raid_leaders_entry.maintank) + "'");
		update_values.push_back(columns[4] + " = '" + Strings::Escape(raid_leaders_entry.assist) + "'");
		update_values.push_back(columns[5] + " = '" + Strings::Escape(raid_leaders_entry.puller) + "'");
		update_values.push_back(columns[6] + " = '" + Strings::Escape(raid_leaders_entry.leadershipaa) + "'");
		update_values.push_back(columns[7] + " = '" + Strings::Escape(raid_leaders_entry.mentoree) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(raid_leaders_entry.mentor_percent));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				raid_leaders_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RaidLeaders InsertOne(
		RaidLeaders raid_leaders_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(raid_leaders_entry.gid));
		insert_values.push_back(std::to_string(raid_leaders_entry.rid));
		insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.marknpc) + "'");
		insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.maintank) + "'");
		insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.assist) + "'");
		insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.puller) + "'");
		insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.leadershipaa) + "'");
		insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.mentoree) + "'");
		insert_values.push_back(std::to_string(raid_leaders_entry.mentor_percent));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			raid_leaders_entry.id = results.LastInsertedID();
			return raid_leaders_entry;
		}

		raid_leaders_entry = InstanceListRepository::NewEntity();

		return raid_leaders_entry;
	}

	static int InsertMany(
		std::vector<RaidLeaders> raid_leaders_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &raid_leaders_entry: raid_leaders_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(raid_leaders_entry.gid));
			insert_values.push_back(std::to_string(raid_leaders_entry.rid));
			insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.marknpc) + "'");
			insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.maintank) + "'");
			insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.assist) + "'");
			insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.puller) + "'");
			insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.leadershipaa) + "'");
			insert_values.push_back("'" + Strings::Escape(raid_leaders_entry.mentoree) + "'");
			insert_values.push_back(std::to_string(raid_leaders_entry.mentor_percent));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<RaidLeaders> All()
	{
		std::vector<RaidLeaders> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidLeaders entry{};

			entry.gid            = atoi(row[0]);
			entry.rid            = atoi(row[1]);
			entry.marknpc        = row[2];
			entry.maintank       = row[3];
			entry.assist         = row[4];
			entry.puller         = row[5];
			entry.leadershipaa   = row[6];
			entry.mentoree       = row[7];
			entry.mentor_percent = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_RAID_LEADERS_REPOSITORY_H
