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
		// loop cache and update cache value and timestamp
		if (CanCache(k)) {
			for (auto &ce: g_data_bucket_cache) {
				if (CheckBucketMatch(ce, k)) {
					ce = b;
					break;
				}
			}
		}

		DataBucketsRepository::UpdateOne(database, b);
	}
	else {
		b.key_ = k.key;
		b = DataBucketsRepository::InsertOne(database, b);
		if (CanCache(k) && !ExistsInCache(b)) {
			g_data_bucket_cache.emplace_back(b);
			DeleteFromMissesCache(b);
		}
	}
}

std::string DataBucket::GetData(const std::string &bucket_key)
{
	DataBucketKey k = {};
	k.key = bucket_key;
	return GetData(k).value;
}

// GetData fetches bucket data from the database or cache if it exists
// if the bucket doesn't exist, it will be added to the cache as a miss
// if ignore_misses_cache is true, the bucket will not be added to the cache as a miss
// the only place we should be ignoring the misses cache is on the initial read during SetData
DataBucketsRepository::DataBuckets DataBucket::GetData(const DataBucketKey &k, bool ignore_misses_cache)
{
	LogDataBuckets(
		"Getting bucket key [{}] bot_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
		k.character_id,
		k.npc_id
	);

	for (const auto &ce: g_data_bucket_cache) {
		if (CheckBucketMatch(ce, k)) {
			if (ce.expires > 0 && ce.expires < std::time(nullptr)) {
				LogDataBuckets("Attempted to read expired key [{}] removing from cache", ce.key_);
				DeleteData(k);
				return DataBucketsRepository::NewEntity();
			}

			// this is a bucket miss, return empty entity
			// we still cache bucket misses, so we don't have to hit the database
			if (ce.id == 0) {
				return DataBucketsRepository::NewEntity();
			}

			LogDataBuckets("Returning key [{}] value [{}] from cache", ce.key_, ce.value);
			return ce;
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
		if (!ignore_misses_cache) {
			size_t size_before = g_data_bucket_cache.size();

			// cache bucket misses, so we don't have to hit the database
			// when scripts try to read a bucket that doesn't exist
			g_data_bucket_cache.emplace_back(
				DataBucketsRepository::DataBuckets{
					.id = 0,
					.key_ = k.key,
					.value = "",
					.expires = 0,
					.character_id = k.character_id,
					.npc_id = k.npc_id,
					.bot_id = k.bot_id
				}
			);

			LogDataBuckets(
				"Key [{}] not found in database, adding to cache as a miss character_id [{}] npc_id [{}] bot_id [{}] cache size before [{}] after [{}]",
				k.key,
				k.character_id,
				k.npc_id,
				k.bot_id,
				size_before,
				g_data_bucket_cache.size()
			);
		}

		return {};
	}

	// if the entry has expired, delete it
	if (r[0].expires > 0 && r[0].expires < (long long) std::time(nullptr)) {
		DeleteData(k);
		return {};
	}

	bool has_cache = false;

	for (auto &ce: g_data_bucket_cache) {
		if (ce.id == r[0].id) {
			has_cache = true;
			break;
		}
	}

	if (!has_cache) {
		g_data_bucket_cache.emplace_back(r[0]);
	}

	return r[0];
}

std::string DataBucket::GetDataExpires(const std::string &bucket_key)
{
	DataBucketKey k = {};
	k.key = bucket_key;

	return GetDataExpires(k);
}

std::string DataBucket::GetDataRemaining(const std::string &bucket_key)
{
	DataBucketKey k = {};
	k.key = bucket_key;
	return GetDataRemaining(k);
}

bool DataBucket::DeleteData(const std::string &bucket_key)
{
	DataBucketKey k = {};
	k.key = bucket_key;
	return DeleteData(k);
}

// GetDataBuckets bulk loads all data buckets for a mob
bool DataBucket::GetDataBuckets(Mob *mob)
{
	DataBucketLoadType::Type t{};
	const uint32             id = mob->GetMobTypeIdentifier();

	if (!id) {
		return false;
	}

	if (mob->IsBot()) {
		t = DataBucketLoadType::Bot;
	}
	else if (mob->IsClient()) {
		t = DataBucketLoadType::Client;
	}
	else if (mob->IsNPC()) {
		t = DataBucketLoadType::NPC;
	}

	BulkLoadEntities(t, {id});

	return true;
}

bool DataBucket::DeleteData(const DataBucketKey &k)
{
	size_t size_before = g_data_bucket_cache.size();

	// delete from cache where contents match
	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &ce) {
				return CheckBucketMatch(ce, k);
			}
		),
		g_data_bucket_cache.end()
	);

	LogDataBuckets(
		"Deleting bucket key [{}] bot_id [{}] character_id [{}] npc_id [{}] cache size before [{}] after [{}]",
		k.key,
		k.bot_id,
		k.character_id,
		k.npc_id,
		size_before,
		g_data_bucket_cache.size()
	);

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
		"Getting bucket expiration key [{}] bot_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
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
		"Getting bucket remaining key [{}] bot_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
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
		dbe.character_id == k.character_id &&
		dbe.npc_id == k.npc_id
	);
}

void DataBucket::BulkLoadEntities(DataBucketLoadType::Type t, std::vector<uint32> ids)
{
	if (ids.empty()) {
		return;
	}

	if (ids.size() == 1) {
		bool            has_cache = false;
		for (const auto &ce: g_data_bucket_cache) {
			if (t == DataBucketLoadType::Bot) {
				has_cache = ce.bot_id == ids[0];
			}
			else if (t == DataBucketLoadType::Client) {
				has_cache = ce.character_id == ids[0];
			}
			else if (t == DataBucketLoadType::NPC) {
				has_cache = ce.npc_id == ids[0];
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
		case DataBucketLoadType::NPC:
			column = "npc_id";
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

void DataBucket::DeleteCachedBuckets(DataBucketLoadType::Type t, uint32 id)
{
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &ce) {
				return (
					(t == DataBucketLoadType::Bot && ce.bot_id == id) ||
					(t == DataBucketLoadType::Client && ce.character_id == id) ||
					(t == DataBucketLoadType::NPC && ce.npc_id == id)
				);
			}
		),
		g_data_bucket_cache.end()
	);

	LogDataBuckets(
		"LoadType [{}] id [{}] cache size before [{}] after [{}]",
		DataBucketLoadType::Name[t],
		id,
		size_before,
		g_data_bucket_cache.size()
	);
}

int64_t DataBucket::GetCurrentTimeUNIX()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}

bool DataBucket::ExistsInCache(const DataBucketsRepository::DataBuckets &e)
{
	for (const auto &ce: g_data_bucket_cache) {
		if (ce.id == e.id) {
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
			[&](DataBucketsRepository::DataBuckets &ce) {
				switch (type) {
					case DataBucketLoadType::Bot:
						return ce.bot_id == id;
					case DataBucketLoadType::Client:
						return ce.character_id == id;
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
	if (key.character_id > 0) {
		return true;
	}

	return false;
}
