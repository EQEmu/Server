#include "data_bucket.h"
#include "entity.h"
#include "zonedb.h"
#include "mob.h"
#include <ctime>
#include <cctype>

std::vector<DataBucketEntry> g_data_bucket_cache = {};

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
	auto r = GetData(k);
	// if we have an entry, use it
	if (r.id > 0) {
		b = r;
	}

	// add scoping to bucket
	if (k.character_id > 0) {
		b.character_id = k.character_id;
	} else if (k.npc_id > 0) {
		b.npc_id = k.npc_id;
	} else if (k.bot_id > 0) {
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
		for (auto& ce : g_data_bucket_cache) {
			if (CheckBucketMatch(ce.e, k)) {
				ce.e = b;
				ce.updated_time = GetCurrentTimeUNIX();
			}
		}

		DataBucketsRepository::UpdateOne(database, b);
	} else {
		b.key_ = k.key;

		// add data bucket and timestamp to cache
		g_data_bucket_cache.emplace_back(
			DataBucketEntry{
				.e = DataBucketsRepository::InsertOne(database, b),
				.updated_time = DataBucket::GetCurrentTimeUNIX()
			}
		);
	}
}

std::string DataBucket::GetData(const std::string &bucket_key)
{
	DataBucketKey k = {};
	k.key = bucket_key;
	return GetData(k).value;
}

DataBucketsRepository::DataBuckets DataBucket::GetData(const DataBucketKey &k)
{
	for (const auto& ce : g_data_bucket_cache) {
		if (CheckBucketMatch(ce.e, k)) {
			if (ce.e.expires > 0 && ce.e.expires < std::time(nullptr)) {
				LogDataBuckets("Attempted to read expired key [{}] removing from cache", ce.e.key_);
				DeleteData(k);
				return DataBucketsRepository::NewEntity();
			}

			LogDataBuckets("Returning key [{}] value [{}] from cache", ce.e.key_, ce.e.value);
			return ce.e;
		}
	}

	auto r = DataBucketsRepository::GetWhere(
		database,
		fmt::format(
			"{} `key` = '{}' AND (`expires` > {} OR `expires` = 0) LIMIT 1",
			DataBucket::GetScopedDbFilters(k),
			k.key,
			(long long) std::time(nullptr)
		)
	);

	if (r.empty()) {
		return {};
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
	DataBucketLoadType::Type t;
	const uint32             id = mob->GetMobTypeIdentifier();

	if (!id) {
		return false;
	}

	if (mob->IsBot()) {
		t = DataBucketLoadType::Bot;
	} else if (mob->IsClient()) {
		t = DataBucketLoadType::Client;
	} else if (mob->IsNPC()) {
		t = DataBucketLoadType::NPC;
	}

	BulkLoadEntities(t, {id});

	return true;
}

bool DataBucket::DeleteData(const DataBucketKey &k)
{
	// delete from cache where contents match
	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketEntry& ce) {
				return CheckBucketMatch(ce.e, k);
			}
		),
		g_data_bucket_cache.end()
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
	for (const auto& ce : g_data_bucket_cache) {
		if (CheckBucketMatch(ce.e, k)) {
			return std::to_string(ce.e.expires);
		}
	}

	auto r = GetData(k);
	if (r.id == 0) {
		return {};
	}

	return std::to_string(r.expires);
}

std::string DataBucket::GetDataRemaining(const DataBucketKey &k)
{
	for (const auto& ce : g_data_bucket_cache) {
		if (CheckBucketMatch(ce.e, k)) {
			return std::to_string(ce.e.expires - static_cast<uint32>(std::time(nullptr)));
		}
	}

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
	else if (k.npc_id > 0) {
		query.emplace_back(fmt::format("npc_id = {}", k.npc_id));
	}
	else if (k.bot_id > 0) {
		query.emplace_back(fmt::format("bot_id = {}", k.bot_id));
	}

	return fmt::format(
		"{} {}",
		Strings::Join(query, " AND "),
		!query.empty() ? "AND" : ""
	);
}

bool DataBucket::CheckBucketMatch(const DataBucketsRepository::DataBuckets& dbe, const DataBucketKey& k)
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
		bool has_cache = false;
		for (const auto& ce : g_data_bucket_cache) {
			if (t == DataBucketLoadType::Bot) {
				has_cache = ce.e.bot_id == ids[0];
			} else if (t == DataBucketLoadType::Client) {
				has_cache = ce.e.character_id == ids[0];
			} else if (t == DataBucketLoadType::NPC) {
				has_cache = ce.e.npc_id == ids[0];
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
			LogError("Incorrect LoadType [{}]", t);
			break;
	}

	const auto& l = DataBucketsRepository::GetWhere(
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

	g_data_bucket_cache.reserve(g_data_bucket_cache.size() + l.size());

	for (const auto& e : l) {
		LogDataBucketsDetail("bucket id [{}] bucket key [{}] bucket value [{}]", e.id, e.key_, e.value);

		g_data_bucket_cache.emplace_back(
			DataBucketEntry{
				.e = e,
				.updated_time = GetCurrentTimeUNIX()
			}
		);
	}

	LogDataBucketsDetail("cache size after [{}]", g_data_bucket_cache.size());

	LogDataBuckets(
		"Bulk Loaded ids [{}] column [{}] new cache size is [{}]",
		ids.size(),
		column,
		g_data_bucket_cache.size()
	);
}

uint64_t DataBucket::GetCurrentTimeUNIX()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}
