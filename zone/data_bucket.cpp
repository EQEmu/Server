#include "data_bucket.h"
#include "zonedb.h"
#include "mob.h"
#include <ctime>
#include <cctype>

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
	bool found = false;
	auto b = DataBucketsRepository::NewEntity();

	for (uint32 index = 0; index < data_bucket_cache.size(); index++) {
		auto& ce = data_bucket_cache[index];
		if (CheckBucketMatch(ce.e, k)) {
			b     = ce.e;
			found = true;

			ce.e.value      = k.value;
			ce.updated_time = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
			).count();
		}
	}

	if (!found) {

		auto r = GetData(k);
		// if we have an entry, use it
		if (r.id == 0) {
			b = r;
		}

		if (k.character_id > 0) {
			b.character_id = k.character_id;
		} else if (k.npc_id > 0) {
			b.npc_id = k.npc_id;
		} else if (k.bot_id > 0) {
			b.bot_id = k.bot_id;
		}
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
		DataBucketsRepository::UpdateOne(database, b);
	} else {
		DataBucketEntry dbe;

		dbe.e            = b;
		dbe.updated_time = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();

		data_bucket_cache.emplace_back(dbe);

		b.key_ = k.key;
		DataBucketsRepository::InsertOne(database, b);
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
	DataBucketKey r = {};
	r.key = bucket_key;
	return DeleteData(r);
}

// GetDataBuckets bulk loads all data buckets for a mob
bool DataBucket::GetDataBuckets(Mob *mob)
{
	DataBucketKey k = mob->GetScopedBucketKeys();
	auto          l = DataBucketsRepository::GetWhere(
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
	for (const auto& ce : data_bucket_cache) {
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
	for (const auto& ce : data_bucket_cache) {
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
