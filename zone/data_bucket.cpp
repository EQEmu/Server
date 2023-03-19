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
void DataBucket::SetData(std::string bucket_key, std::string bucket_value, std::string expires_time) {
	uint64 bucket_id = DataBucket::DoesBucketExist(bucket_key);

	std::string query;
	long long expires_time_unix = 0;

	if (!expires_time.empty()) {
		if (isalpha(expires_time[0]) || isalpha(expires_time[expires_time.length() - 1])) {
			expires_time_unix = (long long) std::time(nullptr) + Strings::TimeToSeconds(expires_time);
		} else {
			expires_time_unix = (long long) std::time(nullptr) + atoi(expires_time.c_str());
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
std::string DataBucket::GetData(std::string bucket_key) {
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
std::string DataBucket::GetDataExpires(std::string bucket_key) {
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

std::string DataBucket::GetDataRemaining(std::string bucket_key) {
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
uint64 DataBucket::DoesBucketExist(std::string bucket_key) {
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

	return std::stoull(row[0]);
}

/**
 * Deletes data bucket by key
 * @param bucket_key
 * @return
 */
bool DataBucket::DeleteData(std::string bucket_key) {
	std::string query = StringFormat(
			"DELETE FROM `data_buckets` WHERE `key` = '%s'",
			Strings::Escape(bucket_key).c_str()
	);

	auto results = database.QueryDatabase(query);

	return results.Success();
}
