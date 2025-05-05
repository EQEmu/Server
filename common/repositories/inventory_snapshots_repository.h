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

	static void DivergeCharacterInvSnapshotFromInventory(
		Database &db,
		uint32 character_id,
		uint32 timestamp,
		std::list<std::pair<int16, uint32>> &compare_list)
	{
		const std::string &query = fmt::format(
			"SELECT slot_id, item_id FROM `inventory_snapshots` "
			"WHERE `time_index` = '{0}' AND `character_id` = '{1}' AND `slot_id` NOT IN ("
			"SELECT a.`slot_id` FROM `inventory_snapshots` a JOIN `inventory` b USING (`slot_id`, `item_id`) "
			"WHERE a.`time_index` = '{0}' AND a.`character_id` = '{1}' AND b.`character_id` = '{1}')",
			timestamp,
			character_id
		);
		auto results = db.QueryDatabase(query);

		if (!results.Success()) {
			return;
		}

		for (auto row: results) {
			compare_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
		}
	}

	static void DivergeCharacterInventoryFromInvSnapshot(
		Database &db, uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list)
	{
		const std::string &query = fmt::format(
			"SELECT `slot_id`, `item_id` FROM `inventory` WHERE "
			"`character_id` = '{0}' AND `slot_id` NOT IN ("
			"SELECT a.`slot_id` FROM `inventory` a JOIN `inventory_snapshots` b USING (`slot_id`, `item_id`) "
			"WHERE b.`time_index` = '{1}' AND b.`character_id` = '{0}' AND a.`character_id` = '{0}')",
			character_id,
			timestamp
		);

		auto results = db.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		for (auto row: results) {
			compare_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
		}
	}

	static bool SaveCharacterInvSnapshot(Database &db, uint32 character_id)
	{
		uint32                          time_index = time(nullptr);
		std::vector<InventorySnapshots> queue{};

		auto inventory = InventoryRepository::GetWhere(db, fmt::format("`character_id` = '{}'", character_id));
		if (inventory.empty()) {
			LogError("Character ID [{}] inventory is empty. Snapshot not created", character_id);
			return false;
		}

		for (auto const &i: inventory) {
			auto snapshot                = NewEntity();
			snapshot.character_id        = i.character_id;
			snapshot.item_id             = i.item_id;
			snapshot.item_unique_id      = i.item_unique_id;
			snapshot.augment_one         = i.augment_one;
			snapshot.augment_two         = i.augment_two;
			snapshot.augment_three       = i.augment_three;
			snapshot.augment_four        = i.augment_four;
			snapshot.augment_five        = i.augment_five;
			snapshot.augment_six         = i.augment_six;
			snapshot.charges             = i.charges;
			snapshot.color               = i.color;
			snapshot.custom_data         = i.custom_data;
			snapshot.instnodrop          = i.instnodrop;
			snapshot.ornament_hero_model = i.ornament_hero_model;
			snapshot.ornament_icon       = i.ornament_icon;
			snapshot.ornament_idfile     = i.ornament_idfile;
			snapshot.slot_id             = i.slot_id;
			snapshot.time_index          = time_index;
			queue.push_back(snapshot);
		}

		if (queue.empty()) {
			LogError("Character ID [{}] inventory is empty. Snapshot not created", character_id);
			return false;
		}

		if (!InsertMany(db, queue)) {
			LogError("Failed to created inventory snapshot for [{}]", character_id);
			return false;
		}

		LogInventory("Created inventory snapshot for [{}] with ([{}]) items", character_id, queue.size());
		return true;
	}

	static bool RestoreCharacterInvSnapshot(Database &db, uint32 character_id, uint32 timestamp)
	{
		InventoryRepository::DeleteWhere(db, fmt::format("`character_id` = '{}'", character_id));

		auto snapshot = GetWhere(db, fmt::format("`character_id` = '{}' AND `time_index` = '{}'", character_id, timestamp));
		if (snapshot.empty()) {
			LogError("The snapshot requested could not be found.  Restore failed for character id [{}] @ [{}] failed",
				character_id,
				timestamp
			);
			return false;
		}

		std::vector<InventoryRepository::Inventory> queue{};
		for (auto const &i: snapshot) {
			auto inventory_entry                = InventoryRepository::NewEntity();
			inventory_entry.character_id        = i.character_id;
			inventory_entry.item_id             = i.item_id;
			inventory_entry.item_unique_id      = i.item_unique_id;
			inventory_entry.augment_one         = i.augment_one;
			inventory_entry.augment_two         = i.augment_two;
			inventory_entry.augment_three       = i.augment_three;
			inventory_entry.augment_four        = i.augment_four;
			inventory_entry.augment_five        = i.augment_five;
			inventory_entry.augment_six         = i.augment_six;
			inventory_entry.charges             = i.charges;
			inventory_entry.color               = i.color;
			inventory_entry.custom_data         = i.custom_data;
			inventory_entry.instnodrop          = i.instnodrop;
			inventory_entry.ornament_hero_model = i.ornament_hero_model;
			inventory_entry.ornament_icon       = i.ornament_icon;
			inventory_entry.ornament_idfile     = i.ornament_idfile;
			inventory_entry.slot_id             = i.slot_id;
			queue.push_back(inventory_entry);
		}

		if (queue.empty()) {
			LogError("The snapshot is empty.  Restore failed for character id [{}] @ [{}] failed", character_id, timestamp);
			return false;
		}

		if (!InventoryRepository::InsertMany(db, queue)) {
			LogError("A database error occurred.  Restore failed for character id [{}] @ [{}] failed", character_id, timestamp);
			return false;
		}

		LogInventory(
			"Restore complete for character id [{}] with snapshot @ [{}] with [{}] entries",
			character_id,
			timestamp,
			queue.size()
		);
		return true;
	}
};

#endif //EQEMU_INVENTORY_SNAPSHOTS_REPOSITORY_H
