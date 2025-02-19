#include "data_bucket.h"
#include "zonedb.h"
#include "mob.h"
#include "worldserver.h"
#include <ctime>
#include <cctype>
#include "../common/json/json.hpp"

using json = nlohmann::json;

extern WorldServer worldserver;
const std::string  NESTED_KEY_DELIMITER = ".";

std::vector<DataBucketsRepository::DataBuckets> g_data_bucket_cache = {};

void DataBucket::SetData(const std::string &bucket_key, const std::string &bucket_value, std::string expires_time)
{
	auto k = DataBucketKey{
		.key = bucket_key,
		.value = bucket_value,
		.expires = expires_time,
	};

	DataBucket::SetData(k);
}

void DataBucket::SetData(const DataBucketKey &k_)
{
	DataBucketKey k = k_; // copy the key so we can modify it
	if (k.key.find(NESTED_KEY_DELIMITER) != std::string::npos) {
		k.key = Strings::Split(k.key, NESTED_KEY_DELIMITER).front();
	}

	auto b = DataBucketsRepository::NewEntity();
	auto r = GetData(k, true);
	// if we have an entry, use it
	if (r.id > 0) {
		b = r;
	}

	// add scoping to bucket
	if (k.character_id > 0) {
		b.character_id = k.character_id;
	}
	else if (k.account_id > 0) {
		b.account_id = k.account_id;
	}
	else if (k.npc_id > 0) {
		b.npc_id = k.npc_id;
	}
	else if (k.bot_id > 0) {
		b.bot_id = k.bot_id;
	} else if (k.zone_id > 0) {
		b.zone_id = k.zone_id;
		b.instance_id = k.instance_id;
	}

	const uint64 bucket_id         = b.id;
	int64        expires_time_unix = 0;

	if (!k.expires.empty()) {
		expires_time_unix = static_cast<int64>(std::time(nullptr)) + Strings::ToInt(k.expires);
		if (isalpha(k.expires[0]) || isalpha(k.expires[k.expires.length() - 1])) {
			expires_time_unix = static_cast<int64>(std::time(nullptr)) + Strings::TimeToSeconds(k.expires);
		}
	}

	b.expires = expires_time_unix;
	b.value   = k.value;
	b.key_    = k.key;

	// Check for nested keys (keys with dots)
	if (k_.key.find(NESTED_KEY_DELIMITER) != std::string::npos) {
		// Retrieve existing JSON or create a new one
		std::string existing_value = r.id > 0 ? r.value : "{}";
		json json_value = json::object();

		try {
			json_value = json::parse(existing_value);
		} catch (json::parse_error &e) {
			LogError("Failed to parse JSON for key [{}]: {}", k_.key, e.what());
			json_value = json::object(); // Reset to an empty object on error
		}

		// Recursively merge new key-value pair into the JSON object
		auto nested_keys = Strings::Split(k_.key, NESTED_KEY_DELIMITER);
		json *current = &json_value;

		for (size_t i = 0; i < nested_keys.size(); ++i) {
			const std::string &key_part = nested_keys[i];
			if (i == nested_keys.size() - 1) {
				// Set the value at the final key
				(*current)[key_part] = k_.value;
			} else {
				// Traverse or create nested objects
				if (!current->contains(key_part)) {
					(*current)[key_part] = json::object();
				} else if (!(*current)[key_part].is_object()) {
					// If key exists but is not an object, reset to object to avoid conflicts
					(*current)[key_part] = json::object();
				}
				current = &(*current)[key_part];
			}
		}

		// Serialize JSON back to string
		b.value = json_value.dump();
		b.key_ = nested_keys.front(); // Use the top-level key
	}

	if (bucket_id) {
		// update the cache if it exists
		if (CanCache(k)) {
			for (auto &e: g_data_bucket_cache) {
				if (CheckBucketMatch(e, k)) {
					e = b;
					break;
				}
			}
		}

		DataBucketsRepository::UpdateOne(database, b);
	}
	else {
		b = DataBucketsRepository::InsertOne(database, b);

		// add to cache if it doesn't exist
		if (CanCache(k) && !ExistsInCache(b)) {
			DeleteFromMissesCache(b);
			g_data_bucket_cache.emplace_back(b);
		}
	}
}

std::string DataBucket::GetData(const std::string &bucket_key)
{
	return GetData(DataBucketKey{.key = bucket_key}).value;
}

DataBucketsRepository::DataBuckets DataBucket::ExtractNestedValue(
	const DataBucketsRepository::DataBuckets &bucket,
	const std::string &full_key)
{
	auto nested_keys = Strings::Split(full_key, NESTED_KEY_DELIMITER);
	json json_value;

	try {
		json_value = json::parse(bucket.value); // Parse the JSON
	} catch (json::parse_error &ex) {
		LogError("Failed to parse JSON for key [{}]: {}", bucket.key_, ex.what());
		return DataBucketsRepository::NewEntity(); // Return empty entity on parse error
	}

	// Start from the top-level key (e.g., "progression")
	json *current = &json_value;

	// Traverse the JSON structure
	for (const auto &key_part: nested_keys) {
		LogDataBuckets("Looking for key part [{}] in JSON", key_part);

		if (!current->contains(key_part)) {
			LogDataBuckets("Key part [{}] not found in JSON for [{}]", key_part, full_key);
			return DataBucketsRepository::NewEntity();
		}

		current = &(*current)[key_part];
	}

	// Create a new entity with the extracted value
	DataBucketsRepository::DataBuckets result = bucket; // Copy the original bucket
	result.value = current->is_string() ? current->get<std::string>() : current->dump();
	return result;
}

// GetData fetches bucket data from the database or cache if it exists
// if the bucket doesn't exist, it will be added to the cache as a miss
// if ignore_misses_cache is true, the bucket will not be added to the cache as a miss
// the only place we should be ignoring the misses cache is on the initial read during SetData
DataBucketsRepository::DataBuckets DataBucket::GetData(const DataBucketKey &k_, bool ignore_misses_cache)
{
	DataBucketKey k = k_; // Copy the key so we can modify it

	bool is_nested_key = k.key.find(NESTED_KEY_DELIMITER) != std::string::npos;

	// Extract the top-level key for nested keys
	if (is_nested_key) {
		k.key = Strings::Split(k.key, NESTED_KEY_DELIMITER).front();
	}

	LogDataBuckets(
		"Getting bucket key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}] zone_id [{}] instance_id [{}]",
		k.key,
		k.bot_id,
		k.account_id,
		k.character_id,
		k.npc_id,
		k.zone_id,
		k.instance_id
	);

	bool can_cache = CanCache(k);

	// Attempt to retrieve the value from the cache
	if (can_cache) {
		for (const auto &e : g_data_bucket_cache) {
			if (CheckBucketMatch(e, k)) {
				if (e.expires > 0 && e.expires < std::time(nullptr)) {
					LogDataBuckets("Attempted to read expired key [{}] removing from cache", e.key_);
					DeleteData(k);
					return DataBucketsRepository::NewEntity();
				}

				LogDataBuckets("Returning key [{}] value [{}] from cache", e.key_, e.value);

				if (is_nested_key && !k_.key.empty()) {
					return ExtractNestedValue(e, k_.key);
				}

				return e;
			}
		}
	}

	// Fetch the value from the database
	auto r = DataBucketsRepository::GetWhere(
		database,
		fmt::format(
			" {} `key` = '{}' LIMIT 1",
			DataBucket::GetScopedDbFilters(k),
			k.key
		)
	);

	if (r.empty()) {
		// Handle cache misses
		if (!ignore_misses_cache && can_cache) {
			size_t size_before = g_data_bucket_cache.size();

			g_data_bucket_cache.emplace_back(
				DataBucketsRepository::DataBuckets{
					.id = 0,
					.key_ = k.key,
					.value = "",
					.expires = 0,
					.account_id = k.account_id,
					.character_id = k.character_id,
					.npc_id = k.npc_id,
					.bot_id = k.bot_id,
					.zone_id = k.zone_id,
					.instance_id = k.instance_id
				}
			);

			LogDataBuckets(
				"Key [{}] not found in database, adding to cache as a miss account_id [{}] character_id [{}] npc_id [{}] bot_id [{}] zone_id [{}] instance_id [{}] cache size before [{}] after [{}]",
				k.key,
				k.account_id,
				k.character_id,
				k.npc_id,
				k.bot_id,
				k.zone_id,
				k.instance_id,
				size_before,
				g_data_bucket_cache.size()
			);
		}

		return DataBucketsRepository::NewEntity();
	}

	auto bucket = r.front();

	// If the entry has expired, delete it
	if (bucket.expires > 0 && bucket.expires < static_cast<long long>(std::time(nullptr))) {
		DeleteData(k);
		return DataBucketsRepository::NewEntity();
	}

	// Add the value to the cache if it doesn't exist
	if (can_cache) {
		bool has_cache = false;
		for (const auto &e : g_data_bucket_cache) {
			if (e.id == bucket.id) {
				has_cache = true;
				break;
			}
		}

		if (!has_cache) {
			g_data_bucket_cache.emplace_back(bucket);
		}
	}

	// Handle nested key extraction
	if (is_nested_key && !k_.key.empty()) {
		return ExtractNestedValue(bucket, k_.key);
	}

	return bucket;
}

std::string DataBucket::GetDataExpires(const std::string &bucket_key)
{
	return GetDataExpires(DataBucketKey{.key = bucket_key});
}

std::string DataBucket::GetDataRemaining(const std::string &bucket_key)
{
	return GetDataRemaining(DataBucketKey{.key = bucket_key});
}

bool DataBucket::DeleteData(const std::string &bucket_key)
{
	return DeleteData(DataBucketKey{.key = bucket_key});
}

// GetDataBuckets bulk loads all data buckets for a mob
bool DataBucket::GetDataBuckets(Mob *mob)
{
	const uint32 id = mob->GetMobTypeIdentifier();

	if (!id) {
		return false;
	}

	if (mob->IsBot()) {
		BulkLoadEntitiesToCache(DataBucketLoadType::Bot, {id});
	}
	else if (mob->IsClient()) {
		BulkLoadEntitiesToCache(DataBucketLoadType::Account, {id});
		BulkLoadEntitiesToCache(DataBucketLoadType::Client, {id});
	}

	return true;
}

bool DataBucket::DeleteData(const DataBucketKey &k)
{
	if (CanCache(k)) {
		size_t size_before = g_data_bucket_cache.size();

		// delete from cache where contents match
		g_data_bucket_cache.erase(
			std::remove_if(
				g_data_bucket_cache.begin(),
				g_data_bucket_cache.end(),
				[&](DataBucketsRepository::DataBuckets &e) {
					return CheckBucketMatch(e, k);
				}
			),
			g_data_bucket_cache.end()
		);

		LogDataBuckets(
			"Deleting bucket key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}] bot_id [{}] zone_id [{}] instance_id [{}] cache size before [{}] after [{}]",
			k.key,
			k.bot_id,
			k.account_id,
			k.character_id,
			k.npc_id,
			k.bot_id,
			k.zone_id,
			k.instance_id,
			size_before,
			g_data_bucket_cache.size()
		);
	}

	return DataBucketsRepository::DeleteWhere(
		database,
		fmt::format(
			"{} `key` = '{}'",
			DataBucket::GetScopedDbFilters(k),
			k.key
		)
	);
}

std::string DataBucket::GetDataExpires(const DataBucketKey &k)
{
	LogDataBuckets(
		"Getting bucket expiration key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
		k.account_id,
		k.character_id,
		k.npc_id
	);

	auto r = GetData(k);
	if (r.id == 0) {
		return {};
	}

	return std::to_string(r.expires);
}

std::string DataBucket::GetDataRemaining(const DataBucketKey &k)
{
	LogDataBuckets(
		"Getting bucket remaining key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}] bot_id [{}] zone_id [{}] instance_id [{}]",
		k.key,
		k.bot_id,
		k.account_id,
		k.character_id,
		k.npc_id,
		k.bot_id,
		k.zone_id,
		k.instance_id
	);

	auto r = GetData(k);
	if (r.id == 0) {
		return "0";
	}

	return fmt::format("{}", r.expires - (long long) std::time(nullptr));
}

std::string DataBucket::GetScopedDbFilters(const DataBucketKey &k)
{
	std::vector<std::string> q = {};
	if (k.character_id > 0) {
		q.emplace_back(fmt::format("character_id = {}", k.character_id));
	}
	else {
		q.emplace_back("character_id = 0");
	}

	if (k.account_id > 0) {
		q.emplace_back(fmt::format("account_id = {}", k.account_id));
	}
	else {
		q.emplace_back("account_id = 0");
	}

	if (k.npc_id > 0) {
		q.emplace_back(fmt::format("npc_id = {}", k.npc_id));
	}
	else {
		q.emplace_back("npc_id = 0");
	}

	if (k.bot_id > 0) {
		q.emplace_back(fmt::format("bot_id = {}", k.bot_id));
	}
	else {
		q.emplace_back("bot_id = 0");
	}

	if (k.zone_id > 0) {
		q.emplace_back(fmt::format("zone_id = {} AND instance_id = {}", k.zone_id, k.instance_id));
	}
	else {
		q.emplace_back("zone_id = 0 AND instance_id = 0");
	}

	return fmt::format(
		"{} {}",
		Strings::Join(q, " AND "),
		!q.empty() ? "AND" : ""
	);
}

bool DataBucket::CheckBucketMatch(const DataBucketsRepository::DataBuckets &dbe, const DataBucketKey &k)
{
	return (
		dbe.key_ == k.key &&
		dbe.bot_id == k.bot_id &&
		dbe.account_id == k.account_id &&
		dbe.character_id == k.character_id &&
		dbe.npc_id == k.npc_id &&
		dbe.zone_id == k.zone_id &&
		dbe.instance_id == k.instance_id
	);
}

void DataBucket::LoadZoneCache(uint16 zone_id, uint16 instance_id)
{
	const auto &l = DataBucketsRepository::GetWhere(
		database,
		fmt::format(
			"zone_id = {} AND instance_id = {} AND (`expires` > {} OR `expires` = 0)",
			zone_id,
			instance_id,
			(long long) std::time(nullptr)
		)
	);

	if (l.empty()) {
		return;
	}

	LogDataBucketsDetail("cache size before [{}] l size [{}]", g_data_bucket_cache.size(), l.size());

	uint32 added_count = 0;

	for (const auto &e: l) {
		if (!ExistsInCache(e)) {
			added_count++;
		}
	}

	for (const auto &e: l) {
		if (!ExistsInCache(e)) {
			LogDataBucketsDetail("bucket id [{}] bucket key [{}] bucket value [{}]", e.id, e.key_, e.value);

			g_data_bucket_cache.emplace_back(e);
		}
	}

	LogDataBucketsDetail("cache size after [{}]", g_data_bucket_cache.size());

	LogDataBuckets(
		"Loaded [{}] zone keys new cache size is [{}]",
		l.size(),
		g_data_bucket_cache.size()
	);
}

void DataBucket::BulkLoadEntitiesToCache(DataBucketLoadType::Type t, std::vector<uint32> ids)
{
	if (ids.empty()) {
		return;
	}

	if (ids.size() == 1) {
		bool has_cache = false;

		for (const auto &e: g_data_bucket_cache) {
			if (t == DataBucketLoadType::Bot) {
				has_cache = e.bot_id == ids[0];
			}
			else if (t == DataBucketLoadType::Account) {
				has_cache = e.account_id == ids[0];
			}
			else if (t == DataBucketLoadType::Client) {
				has_cache = e.character_id == ids[0];
			}
		}

		if (has_cache) {
			LogDataBucketsDetail("LoadType [{}] ID [{}] has cache", DataBucketLoadType::Name[t], ids[0]);
			return;
		}
	}

	std::string column;

	switch (t) {
		case DataBucketLoadType::Bot:
			column = "bot_id";
			break;
		case DataBucketLoadType::Client:
			column = "character_id";
			break;
		case DataBucketLoadType::Account:
			column = "account_id";
			break;
		default:
			LogError("Incorrect LoadType [{}]", static_cast<int>(t));
			break;
	}

	const auto &l = DataBucketsRepository::GetWhere(
		database,
		fmt::format(
			"{} IN ({}) AND (`expires` > {} OR `expires` = 0)",
			column,
			Strings::Join(ids, ", "),
			(long long) std::time(nullptr)
		)
	);

	if (l.empty()) {
		return;
	}

	LogDataBucketsDetail("cache size before [{}] l size [{}]", g_data_bucket_cache.size(), l.size());

	uint32 added_count = 0;

	for (const auto &e: l) {
		if (!ExistsInCache(e)) {
			added_count++;
		}
	}

	for (const auto &e: l) {
		if (!ExistsInCache(e)) {
			LogDataBucketsDetail("bucket id [{}] bucket key [{}] bucket value [{}]", e.id, e.key_, e.value);

			g_data_bucket_cache.emplace_back(e);
		}
	}

	LogDataBucketsDetail("cache size after [{}]", g_data_bucket_cache.size());

	LogDataBuckets(
		"Bulk Loaded ids [{}] column [{}] new cache size is [{}]",
		ids.size(),
		column,
		g_data_bucket_cache.size()
	);
}

void DataBucket::DeleteCachedBuckets(DataBucketLoadType::Type type, uint32 id, uint32 secondary_id)
{
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &e) {
				return (
					(type == DataBucketLoadType::Bot && e.bot_id == id) ||
					(type == DataBucketLoadType::Account && e.account_id == id) ||
					(type == DataBucketLoadType::Client && e.character_id == id) ||
					(type == DataBucketLoadType::Zone && e.zone_id == id && e.instance_id == secondary_id)
				);
			}
		),
		g_data_bucket_cache.end()
	);

	LogDataBuckets(
		"LoadType [{}] id [{}] cache size before [{}] after [{}]",
		DataBucketLoadType::Name[type],
		id,
		size_before,
		g_data_bucket_cache.size()
	);
}

bool DataBucket::ExistsInCache(const DataBucketsRepository::DataBuckets &entry)
{
	for (const auto &e: g_data_bucket_cache) {
		if (e.id == entry.id) {
			return true;
		}
	}

	return false;
}

void DataBucket::DeleteFromMissesCache(DataBucketsRepository::DataBuckets e)
{
	// delete from cache where there might have been a written bucket miss to the cache
	// this is to prevent the cache from growing too large
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &ce) {
				return ce.id == 0 && ce.key_ == e.key_ &&
					   ce.account_id == e.account_id &&
					   ce.character_id == e.character_id &&
					   ce.npc_id == e.npc_id &&
					   ce.bot_id == e.bot_id &&
					   ce.zone_id == e.zone_id &&
					   ce.instance_id == e.instance_id;
			}
		),
		g_data_bucket_cache.end()
	);
	LogDataBucketsDetail(
		"Deleted bucket misses from cache where key [{}] size before [{}] after [{}]",
		e.key_,
		size_before,
		g_data_bucket_cache.size()
	);
}

void DataBucket::ClearCache()
{
	g_data_bucket_cache.clear();
	LogInfo("Cleared data buckets cache");
}

void DataBucket::DeleteFromCache(uint64 id, DataBucketLoadType::Type type)
{
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &e) {
				switch (type) {
					case DataBucketLoadType::Bot:
						return e.bot_id == id;
					case DataBucketLoadType::Client:
						return e.character_id == id;
					case DataBucketLoadType::Account:
						return e.account_id == id;
					default:
						return false;
				}
			}
		),
		g_data_bucket_cache.end()
	);

	LogDataBuckets(
		"Deleted [{}] id [{}] from cache size before [{}] after [{}]",
		DataBucketLoadType::Name[type],
		id,
		size_before,
		g_data_bucket_cache.size()
	);
}

void DataBucket::DeleteZoneFromCache(uint16 zone_id, uint16 instance_id, DataBucketLoadType::Type type)
{
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &e) {
				switch (type) {
					case DataBucketLoadType::Zone:
						return e.zone_id == zone_id && e.instance_id == instance_id;
					default:
						return false;
				}
			}
		),
		g_data_bucket_cache.end()
	);

	LogDataBuckets(
		"Deleted zone [{}] instance [{}] from cache size before [{}] after [{}]",
		zone_id,
		instance_id,
		size_before,
		g_data_bucket_cache.size()
	);
}

// CanCache returns whether a bucket can be cached or not
// characters are only in one zone at a time so we can cache locally to the zone
// bots (not implemented) are only in one zone at a time so we can cache locally to the zone
// npcs (ids) can be in multiple zones so we can't cache locally to the zone
bool DataBucket::CanCache(const DataBucketKey &key)
{
	if (key.character_id > 0 || key.account_id > 0 || key.bot_id > 0 || key.zone_id > 0) {
		return true;
	}

	return false;
}
