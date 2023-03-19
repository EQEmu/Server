#include "data_bucket.h"
#include <utility>
#include "../common/strings.h"
#include "zonedb.h"
#include "../common/zone_store.h"
#include <ctime>
#include <cctype>
#include <algorithm>

/**
 * Persists data via bucket_name as key
 * @param bucket_key
 * @param bucket_value
 * @param expires_time
 */
void DataBucket::SetData(const std::string& bucket_key, const std::string& bucket_value, std::string expires_time) {
	uint64 bucket_id = DataBucket::DoesBucketExist(bucket_key);

	std::string query;
	long long expires_time_unix = 0;

	if (!expires_time.empty()) {
		if (isalpha(expires_time[0]) || isalpha(expires_time[expires_time.length() - 1])) {
			expires_time_unix = (long long) std::time(nullptr) + Strings::TimeToSeconds(expires_time);
		} else {
			expires_time_unix = (long long) std::time(nullptr) + Strings::ToInt(expires_time.c_str());
		}
	}

	if (bucket_id > 0) {
		std::string update_expired_time;
		if (expires_time_unix > 0) {
			update_expired_time = StringFormat(", `expires` = %lld ", expires_time_unix);
		}

		query = StringFormat(
				"UPDATE `data_buckets` SET `value` = '%s' %s WHERE `id` = %i",
				Strings::Escape(bucket_value).c_str(),
				Strings::Escape(update_expired_time).c_str(),
				bucket_id
		);
	}
	else {
		query = StringFormat(
				"INSERT INTO `data_buckets` (`key`, `value`, `expires`) VALUES ('%s', '%s', '%lld')",
				Strings::Escape(bucket_key).c_str(),
				Strings::Escape(bucket_value).c_str(),
				expires_time_unix
		);
	}

	database.QueryDatabase(query);
}

/**
 * Retrieves data via bucket_name as key
 * @param bucket_key
 * @return
 */
std::string DataBucket::GetData(const std::string& bucket_key) {
	std::string query = StringFormat(
			"SELECT `value` from `data_buckets` WHERE `key` = '%s' AND (`expires` > %lld OR `expires` = 0)  LIMIT 1",
			bucket_key.c_str(),
			(long long) std::time(nullptr)
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return std::string();
	}

	if (results.RowCount() != 1)
		return std::string();

	auto row = results.begin();

	return std::string(row[0]);
}

/**
 * Retrieves data expires time via bucket_name as key
 * @param bucket_key
 * @return
 */
std::string DataBucket::GetDataExpires(const std::string& bucket_key) {
	std::string query = StringFormat(
			"SELECT `expires` from `data_buckets` WHERE `key` = '%s' AND (`expires` > %lld OR `expires` = 0)  LIMIT 1",
			bucket_key.c_str(),
			(long long) std::time(nullptr)
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return std::string();
	}

	if (results.RowCount() != 1)
		return std::string();

	auto row = results.begin();

	return std::string(row[0]);
}

std::string DataBucket::GetDataRemaining(const std::string& bucket_key) {
	if (DataBucket::GetDataExpires(bucket_key).empty()) {
		return "0";
	}
	std::string query = fmt::format(
		"SELECT (`expires` - UNIX_TIMESTAMP()) AS `remaining` from `data_buckets` WHERE `key` = '{}' AND (`expires` > {} OR `expires` = 0) LIMIT 1",
		bucket_key.c_str(),
		(long long) std::time(nullptr)
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return std::string();
	}

	if (results.RowCount() != 1)
		return std::string();

	auto row = results.begin();

	return std::string(row[0]);
}

/**
 * Checks for bucket existence by bucket_name key
 * @param bucket_key
 * @return
 */
uint64 DataBucket::DoesBucketExist(const std::string& bucket_key) {
	std::string query = StringFormat(
			"SELECT `id` from `data_buckets` WHERE `key` = '%s' AND (`expires` > %lld OR `expires` = 0) LIMIT 1",
			Strings::Escape(bucket_key).c_str(),
			(long long) std::time(nullptr)
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

	auto row = results.begin();
	if (results.RowCount() != 1)
		return 0;

	return Strings::ToUnsignedBigInt(row[0]);
}

/**
 * Deletes data bucket by key
 * @param bucket_key
 * @return
 */
bool DataBucket::DeleteData(const std::string& bucket_key) {
	std::string query = StringFormat(
			"DELETE FROM `data_buckets` WHERE `key` = '%s'",
			Strings::Escape(bucket_key).c_str()
	);

	auto results = database.QueryDatabase(query);

	return results.Success();
}

bool DataBucket::GetDataBuckets(Mob* mob)
{
	auto l = BaseDataBucketsRepository::GetWhere(
		database,
		fmt::format(
			"`key` LIKE '{}-%'",
			Strings::Escape(mob->GetBucketKey())
		)
	);

	if (l.empty()) {
		return false;
	}

	mob->m_data_bucket_cache.clear();

	DataBucketCache d;

	for (const auto& e : l) {
		d.bucket_id = e.id;
		d.bucket_key = e.key;
		d.bucket_value = e.value;
		d.bucket_expires = e.expires;

		mob->m_data_bucket_cache.emplace_back(d);
	}

	return true;
}

std::string DataBucket::CheckBucketKey(const Mob* mob, std::string_view full_name)
{
	std::string bucket_value;
	for (const auto &d : mob->m_data_bucket_cache) {
		if (d.bucket_key == full_name) {
			bucket_value = d.bucket_value;
			break;
		}
	}
	return bucket_value;
}

