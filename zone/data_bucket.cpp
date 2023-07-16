#include "data_bucket.h"
#include "zonedb.h"
#include "mob.h"
#include <ctime>
#include <cctype>
#include "../common/repositories/data_buckets_repository.h"

std::vector<DataBucketEntry> data_bucket_cache;

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
		for (auto &ce: g_data_bucket_cache) {
			if (CheckBucketMatch(ce.e, k)) {
				ce.e            = b;
				ce.updated_time = GetCurrentTimeUNIX();
			}
		}

		DataBucketsRepository::UpdateOne(database, b);
	}
	else {
		b.key_ = k.key;
		b = DataBucketsRepository::InsertOne(database, b);
		if (!ExistsInCache(b)) {
			// add data bucket and timestamp to cache
			g_data_bucket_cache.emplace_back(
				DataBucketEntry{
					.e = b,
					.updated_time = DataBucket::GetCurrentTimeUNIX()
				}
			);

			// delete from cache where there might have been a written bucket miss to the cache
			// this is to prevent the cache from growing too large
			LogDataBucketsDetail(
				"Deleting bucket misses from cache where key [{}] size before [{}]",
				b.key_,
				g_data_bucket_cache.size()
			);
			g_data_bucket_cache.erase(
				std::remove_if(
					g_data_bucket_cache.begin(),
					g_data_bucket_cache.end(),
					[&](DataBucketEntry &ce) {
						return ce.e.id == 0 && ce.e.key_ == b.key_ &&
							   ce.e.character_id == b.character_id &&
							   ce.e.npc_id == b.npc_id &&
							   ce.e.bot_id == b.bot_id;
					}
				),
				g_data_bucket_cache.end()
			);
			LogDataBucketsDetail(
				"Deleted bucket misses from cache where key [{}] size after [{}]",
				b.key_,
				g_data_bucket_cache.size()
			);
		}
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
	for (const auto& ce : data_bucket_cache) {
		if (CheckBucketMatch(ce.e, k)) {
			return ce.e.value;
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
		return {};
	}

	// if the entry has expired, delete it
	if (r[0].expires > 0 && r[0].expires < (long long) std::time(nullptr)) {
		DeleteData(k);
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
	DataBucketKey r = {};
	r.key = bucket_key;
	return DeleteData(r);
}

bool DataBucket::GetDataBuckets(Mob *mob)
{
	DataBucketKey k = mob->GetScopedBucketKeys();
	auto          l = BaseDataBucketsRepository::GetWhere(
		database,
		fmt::format(
			"{} (`expires` > {} OR `expires` = 0)",
			DataBucket::GetScopedDbFilters(k),
			(long long) std::time(nullptr)
		)
	);

	if (l.empty()) {
		return false;
	}

	mob->m_data_bucket_cache.clear();

	DataBucketCache d;

	for (const auto &e: l) {
		d.bucket_id      = e.id;
		d.bucket_key     = e.key_;
		d.bucket_value   = e.value;
		d.bucket_expires = e.expires;

		mob->m_data_bucket_cache.emplace_back(d);
	}

	return true;
}

std::string DataBucket::CheckBucketKey(const Mob *mob, const DataBucketKey &k)
{
	std::string     bucket_value;
	for (const auto &d: mob->m_data_bucket_cache) {
		if (d.bucket_key == k.key) {
			bucket_value = d.bucket_value;
			break;
		}
	}
	return bucket_value;
}

bool DataBucket::DeleteData(const DataBucketKey &k)
{
	for (auto ce = data_bucket_cache.begin(); ce != data_bucket_cache.end(); ce++) {
		if (CheckBucketMatch(ce->e, k)) {
			data_bucket_cache.erase(ce);
		}
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
		"Getting bucket expiration key [{}] bot_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
		k.character_id,
		k.npc_id
	);

	for (const auto &ce: g_data_bucket_cache) {
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
	LogDataBuckets(
		"Getting bucket remaining key [{}] bot_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
		k.character_id,
		k.npc_id
	);

	for (const auto &ce: g_data_bucket_cache) {
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
