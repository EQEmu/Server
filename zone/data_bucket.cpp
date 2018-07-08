#include "data_bucket.h"
#include <utility>
#include "../common/string_util.h"
#include "zonedb.h"
#include <ctime>

/**
 * Persists data via bucket_name as key
 * @param bucket_key
 * @param bucket_value
 */
void DataBucket::SetData(std::string bucket_key, std::string bucket_value, uint32 expires_at_unix) {
	uint64 bucket_id = DataBucket::DoesBucketExist(bucket_key);

	std::string query;

	if (bucket_id > 0) {
		std::string update_expired_time;
		if (expires_at_unix > 0) {
			update_expired_time = StringFormat(", `expires` = %u ", expires_at_unix);
		}

		query = StringFormat(
				"UPDATE `data_buckets` SET `value` = '%s' %s WHERE `id` = %i",
				EscapeString(bucket_value).c_str(),
				EscapeString(update_expired_time).c_str(),
				bucket_id
		);
	}
	else {
		query = StringFormat(
				"INSERT INTO `data_buckets` (`key`, `value`, `expires`) VALUES ('%s', '%s', '%u')",
				EscapeString(bucket_key).c_str(),
				EscapeString(bucket_value).c_str(),
				expires_at_unix
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
 * Checks for bucket existence by bucket_name key
 * @param bucket_key
 * @return
 */
uint64 DataBucket::DoesBucketExist(std::string bucket_key) {
	std::string query = StringFormat(
			"SELECT `id` from `data_buckets` WHERE `key` = '%s' AND (`expires` > %lld OR `expires` = 0) LIMIT 1",
			EscapeString(bucket_key).c_str(),
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
			"DELETE FROM `data_buckets` WHERE `key` = '%s' AND (`expires` > %lld OR `expires` = 0)",
			EscapeString(bucket_key).c_str()
	);

	auto results = database.QueryDatabase(query);

	return results.Success();
}