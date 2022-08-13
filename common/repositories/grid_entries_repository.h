#ifndef EQEMU_GRID_ENTRIES_REPOSITORY_H
#define EQEMU_GRID_ENTRIES_REPOSITORY_H

#include "../database.h"
#include "../strings.h"

class GridEntriesRepository {
public:
	struct GridEntry {
		int   gridid;
		int   zoneid;
		int   number;
		float x;
		float y;
		float z;
		float heading;
		int   pause;
		int8  centerpoint;
	};

	static std::vector<std::string> Columns()
	{
		return {
			"gridid",
			"zoneid",
			"number",
			"x",
			"y",
			"z",
			"heading",
			"pause",
			"centerpoint",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("grid_entries");
	}

	static std::string BaseSelect()
	{
		return std::string(
			fmt::format(
				"SELECT {} FROM {}",
				ColumnsRaw(),
				TableName()
			)
		);
	}

	static GridEntry NewEntity()
	{
		GridEntry entry{};

		entry.gridid      = 0;
		entry.zoneid      = 0;
		entry.number      = 0;
		entry.x           = 0;
		entry.y           = 0;
		entry.z           = 0;
		entry.heading     = 0;
		entry.pause       = 0;
		entry.centerpoint = 0;

		return entry;
	}

	static std::vector<GridEntry> GetZoneGridEntries(int zone_id)
	{
		std::vector<GridEntry> grid_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE zoneid = {} ORDER BY gridid, number",
				BaseSelect(),
				zone_id
			)
		);

		for (auto row = results.begin(); row != results.end(); ++row) {
			GridEntry entry{};

			entry.gridid      = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.number      = atoi(row[2]);
			entry.x           = static_cast<float>(atof(row[3]));
			entry.y           = static_cast<float>(atof(row[4]));
			entry.z           = static_cast<float>(atof(row[5]));
			entry.heading     = static_cast<float>(atof(row[6]));
			entry.pause       = atoi(row[7]);
			entry.centerpoint = atoi(row[8]);

			grid_entries.push_back(entry);
		}

		return grid_entries;
	}
};

#endif
