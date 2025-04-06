#ifndef EQEMU_INVENTORY_SNAPSHOTS_REPOSITORY_H
#define EQEMU_INVENTORY_SNAPSHOTS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_inventory_snapshots_repository.h"
#include "inventory_repository.h"

class InventorySnapshotsRepository: public BaseInventorySnapshotsRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * InventorySnapshotsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * InventorySnapshotsRepository::GetWhereNeverExpires()
     * InventorySnapshotsRepository::GetWhereXAndY()
     * InventorySnapshotsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static int64 CountInventorySnapshots(Database& db)
	{
		const std::string &query =
			"SELECT COUNT(*) FROM (SELECT * FROM `inventory_snapshots` a GROUP BY `character_id`, `time_index`) b";

		auto results = db.QueryDatabase(query);
		if (!results.Success() || !results.RowCount()) {
			return -1;
		}

		auto        row   = results.begin();
		const int64 count = Strings::ToBigInt(row[0]);

		if (count > std::numeric_limits<int>::max()) {
			return -2;
		}

		if (count < 0) {
			return -3;
		}

		return count;
	}

	static int64 CountCharacterInvSnapshots(Database& db, uint32 character_id)
	{
		const std::string &query = fmt::format(
			"SELECT COUNT(*) FROM (SELECT * FROM `inventory_snapshots` a WHERE "
			"`character_id` = '{}' GROUP BY `time_index`) b",
			character_id
		);

		auto results = db.QueryDatabase(query);
		if (!results.Success() || !results.RowCount()) {
			return -1;
		}

		auto       &row   = results.begin();
		const int64 count = Strings::ToBigInt(row[0]);

		if (count > std::numeric_limits<int>::max()) {
			return -2;
		}

		if (count < 0) {
			return -3;
		}

		return count;
	}

	static void ClearCharacterInvSnapshots(Database &db, uint32 character_id, bool from_now)
	{
		uint32 del_time = time(nullptr);
		if (!from_now) {
			del_time -= RuleI(Character, InvSnapshotHistoryD) * 86400;
		}

		DeleteWhere(db, fmt::format("`character_id` = '{}' AND `time_index` <= '{}'", character_id, del_time));
	}

	static void ListCharacterInvSnapshots(Database &db, uint32 character_id, std::list<std::pair<uint32, int>> &is_list)
	{
		const std::string &query = fmt::format(
			"SELECT `time_index`, COUNT(*) FROM `inventory_snapshots` WHERE "
			"`character_id` = '{}' GROUP BY `time_index` ORDER BY `time_index` DESC",
			character_id
		);
		auto results = db.QueryDatabase(query);

		if (!results.Success())
			return;

		for (auto row: results) {
			is_list.emplace_back(std::pair<uint32, int>(Strings::ToUnsignedInt(row[0]), Strings::ToInt(row[1])));
		}
	}

	static bool ValidateCharacterInvSnapshotTimestamp(Database &db, uint32 character_id, uint32 timestamp)
	{
		if (!character_id || !timestamp) {
			return false;
		}

		const std::string &query = fmt::format(
			"SELECT * FROM `inventory_snapshots` WHERE `character_id` = '{}' "
			"AND `time_index` = '{}' LIMIT 1",
			character_id,
			timestamp
		);
		auto results = db.QueryDatabase(query);

		if (!results.Success() || results.RowCount() == 0) {
			return false;
		}

		return true;
	}

	static void ParseCharacterInvSnapshot(
		Database &db,
		uint32 character_id,
		uint32 timestamp,
		std::list<std::pair<int16, uint32>> &parse_list)
	{
		const std::string &query = fmt::format(
			"SELECT `slot_id`, `item_id` FROM `inventory_snapshots` "
			"WHERE `character_id` = '{}' AND `time_index` = '{}' ORDER BY `slot_id`",
			character_id,
			timestamp
		);
		auto results = db.QueryDatabase(query);

		if (!results.Success()) {
			return;
		}

		for (auto row: results) {
			parse_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
		}
	}

	static void TransformToInv(InventorySnapshots &out, const InventoryRepository::Inventory &in, uint32 time_index)
	{
		out.character_id        = in.character_id;
		out.item_id             = in.item_id;
		out.item_unique_id      = in.item_unique_id;
		out.augment_one         = in.augment_one;
		out.augment_two         = in.augment_two;
		out.augment_three       = in.augment_three;
		out.augment_four        = in.augment_four;
		out.augment_five        = in.augment_five;
		out.augment_six         = in.augment_six;
		out.charges             = in.charges;
		out.color               = in.color;
		out.custom_data         = in.custom_data;
		out.instnodrop          = in.instnodrop;
		out.ornament_hero_model = in.ornament_hero_model;
		out.ornament_icon       = in.ornament_icon;
		out.ornament_idfile     = in.ornament_idfile;
		out.guid                = in.guid;
		out.slot_id             = in.slot_id;
		out.time_index          = time_index;
	}

	static bool SaveCharacterInvSnapshot(Database &db, uint32 character_id)
	{
		uint32                          time_index = time(nullptr);
		std::vector<InventorySnapshots> queue{};

		auto inventory = InventoryRepository::GetWhere(db, fmt::format("`character_id` = '{}'", character_id));

		for (auto const &i: inventory) {
			auto snapshot = NewEntity();
			TransformToInv(snapshot, i, time_index);
			queue.push_back(snapshot);
		}

		if (!queue.empty()) {
			InsertMany(db, queue);
			LogInventory("Created inventory snapshot for [{}] with ([{}]) items", character_id, queue.size());
			return true;
		}

		LogInventory("Failed to created inventory snapshot for [{}]", character_id);
		return false;
	}

};

#endif //EQEMU_INVENTORY_SNAPSHOTS_REPOSITORY_H
