#include "data_bucket.h"
#include <utility>
#include "../common/string_util.h"
#include "zonedb.h"
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
			expires_time_unix = (long long) std::time(nullptr) + DataBucket::ParseStringTimeToInt(expires_time);
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
				EscapeString(bucket_value).c_str(),
				EscapeString(update_expired_time).c_str(),
				bucket_id
		);
	}
	else {
		query = StringFormat(
				"INSERT INTO `data_buckets` (`key`, `value`, `expires`) VALUES ('%s', '%s', '%lld')",
				EscapeString(bucket_key).c_str(),
				EscapeString(bucket_value).c_str(),
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
			"DELETE FROM `data_buckets` WHERE `key` = '%s'",
			EscapeString(bucket_key).c_str()
	);

	auto results = database.QueryDatabase(query);

	return results.Success();
}

/**
 * Converts string to integer for use when setting expiration times
 * @param time_string
 * @return
 */
uint32 DataBucket::ParseStringTimeToInt(std::string time_string)
{
	uint32 duration = 0;

	std::transform(time_string.begin(), time_string.end(), time_string.begin(), ::tolower);

	if (time_string.length() < 1)
		return 0;

	std::string time_unit = time_string;
	time_unit.erase(remove_if(time_unit.begin(), time_unit.end(), [](char c) { return !isdigit(c); }), time_unit.end());

	uint32 unit = static_cast<uint32>(atoi(time_unit.c_str()));

	if (time_string.find('s') != std::string::npos)
		duration = unit;
	if (time_string.find('m') != std::string::npos)
		duration = unit * 60;
	if (time_string.find('h') != std::string::npos)
		duration = unit * 3600;
	if (time_string.find('d') != std::string::npos)
		duration = unit * 86400;
	if (time_string.find('y') != std::string::npos)
		duration = unit * 31556926;

	return duration;
}