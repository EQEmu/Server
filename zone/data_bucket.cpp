#include "data_bucket.h"
#include "entity.h"
#include "zonedb.h"
#include "mob.h"
#include "worldserver.h"
#include <ctime>
#include <cctype>

extern WorldServer worldserver;

std::vector<DataBucketsRepository::DataBuckets> g_data_bucket_cache = {};

void DataBucket::SetData(const std::string &bucket_key, const std::string &bucket_value, std::string expires_time)
{
	auto k = DataBucketKey{
		.key = bucket_key,
		.value = bucket_value,
		.expires = expires_time,
		.account_id = 0,
		.character_id = 0,
		.npc_id = 0,
		.bot_id = 0
	};

	DataBucket::SetData(k);
}

void DataBucket::SetData(const DataBucketKey &k)
{
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
		b.key_ = k.key;
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

// GetData fetches bucket data from the database or cache if it exists
// if the bucket doesn't exist, it will be added to the cache as a miss
// if ignore_misses_cache is true, the bucket will not be added to the cache as a miss
// the only place we should be ignoring the misses cache is on the initial read during SetData
DataBucketsRepository::DataBuckets DataBucket::GetData(const DataBucketKey &k, bool ignore_misses_cache)
{
	LogDataBuckets(
		"Getting bucket key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
		k.account_id,
		k.character_id,
		k.npc_id
	);

	bool can_cache = CanCache(k);

	// check the cache first if we can cache
	if (can_cache) {
		for (const auto &e: g_data_bucket_cache) {
			if (CheckBucketMatch(e, k)) {
				if (e.expires > 0 && e.expires < std::time(nullptr)) {
					LogDataBuckets("Attempted to read expired key [{}] removing from cache", e.key_);
					DeleteData(k);
					return DataBucketsRepository::NewEntity();
				}

				// this is a bucket miss, return empty entity
				// we still cache bucket misses, so we don't have to hit the database
				if (e.id == 0) {
					return DataBucketsRepository::NewEntity();
				}

				LogDataBuckets("Returning key [{}] value [{}] from cache", e.key_, e.value);
				return e;
			}
		}
	}

	auto r = DataBucketsRepository::GetWhere(
		database,
		fmt::format(
			"{} `key` = '{}' LIMIT 1",
			DataBucket::GetScopedDbFilters(k),
			k.key
		)
	);

	if (r.empty()) {

		// if we're ignoring the misses cache, don't add to the cache
		// the only place this is ignored is during the initial read of SetData
		bool add_to_misses_cache = !ignore_misses_cache && can_cache;
		if (add_to_misses_cache) {
			size_t size_before = g_data_bucket_cache.size();

			// cache bucket misses, so we don't have to hit the database
			// when scripts try to read a bucket that doesn't exist
			g_data_bucket_cache.emplace_back(
				DataBucketsRepository::DataBuckets{
					.id = 0,
					.key_ = k.key,
					.value = "",
					.expires = 0,
					.account_id = k.account_id,
					.character_id = k.character_id,
					.npc_id = k.npc_id,
					.bot_id = k.bot_id
				}
			);

			LogDataBuckets(
				"Key [{}] not found in database, adding to cache as a miss account_id [{}] character_id [{}] npc_id [{}] bot_id [{}] cache size before [{}] after [{}]",
				k.key,
				k.account_id,
				k.character_id,
				k.npc_id,
				k.bot_id,
				size_before,
				g_data_bucket_cache.size()
			);
		}

		return {};
	}

	auto bucket = r.front();

	// if the entry has expired, delete it
	if (bucket.expires > 0 && bucket.expires < (long long) std::time(nullptr)) {
		DeleteData(k);
		return {};
	}

	// add to cache if it doesn't exist
	if (can_cache) {
		bool has_cache = false;

		for (auto &e: g_data_bucket_cache) {
			if (e.id == bucket.id) {
				has_cache = true;
				break;
			}
		}

		if (!has_cache) {
			g_data_bucket_cache.emplace_back(bucket);
		}
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
			"Deleting bucket key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}] cache size before [{}] after [{}]",
			k.key,
			k.bot_id,
			k.account_id,
			k.character_id,
			k.npc_id,
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
		"Getting bucket remaining key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
		k.account_id,
		k.character_id,
		k.npc_id
	);

	auto r = GetData(k);
	if (r.id == 0) {
		return "0";
	}

	return fmt::format("{}", r.expires - (long long) std::time(nullptr));
}

std::string DataBucket::GetScopedDbFilters(const DataBucketKey &k)
{
	std::vector<std::string> query = {};
	if (k.character_id > 0) {
		query.emplace_back(fmt::format("character_id = {}", k.character_id));
	}
	else {
		query.emplace_back("character_id = 0");
	}

	if (k.account_id > 0) {
		query.emplace_back(fmt::format("account_id = {}", k.account_id));
	}
	else {
		query.emplace_back("account_id = 0");
	}

	if (k.npc_id > 0) {
		query.emplace_back(fmt::format("npc_id = {}", k.npc_id));
	}
	else {
		query.emplace_back("npc_id = 0");
	}

	if (k.bot_id > 0) {
		query.emplace_back(fmt::format("bot_id = {}", k.bot_id));
	}
	else {
		query.emplace_back("bot_id = 0");
	}

	return fmt::format(
		"{} {}",
		Strings::Join(query, " AND "),
		!query.empty() ? "AND" : ""
	);
}

bool DataBucket::CheckBucketMatch(const DataBucketsRepository::DataBuckets &dbe, const DataBucketKey &k)
{
	return (
		dbe.key_ == k.key &&
		dbe.bot_id == k.bot_id &&
		dbe.account_id == k.account_id &&
		dbe.character_id == k.character_id &&
		dbe.npc_id == k.npc_id
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

void DataBucket::DeleteCachedBuckets(DataBucketLoadType::Type type, uint32 id)
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
					(type == DataBucketLoadType::Client && e.character_id == id)
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
					   ce.bot_id == e.bot_id;
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

// CanCache returns whether a bucket can be cached or not
// characters are only in one zone at a time so we can cache locally to the zone
// bots (not implemented) are only in one zone at a time so we can cache locally to the zone
// npcs (ids) can be in multiple zones so we can't cache locally to the zone
bool DataBucket::CanCache(const DataBucketKey &key)
{
	if (key.character_id > 0 || key.account_id > 0 || key.bot_id > 0) {
		return true;
	}

	return false;
}
