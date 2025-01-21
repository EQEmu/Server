#ifndef EQEMU_DATABUCKET_H
#define EQEMU_DATABUCKET_H

#include <string>
#include "../common/types.h"
#include "../common/repositories/data_buckets_repository.h"
#include "mob.h"
#include "../common/json/json_archive_single_line.h"
#include "../common/servertalk.h"

struct DataBucketKey {
	std::string key;
	std::string value;
	std::string expires;
	int64_t     account_id = 0;
	int64_t     character_id = 0;
	int64_t     npc_id = 0;
	int64_t     bot_id = 0;
};

namespace DataBucketLoadType {
	enum Type : uint8 {
		Bot,
		Account,
		Client,
		MaxType
	};

	static const std::string Name[Type::MaxType] = {
		"Bot",
		"Account",
		"Client",
	};
}

class DataBucket {
public:
	// non-scoped bucket methods (for global buckets)
	static void SetData(const std::string &bucket_key, const std::string &bucket_value, std::string expires_time = "");
	static bool DeleteData(const std::string &bucket_key);
	static std::string GetData(const std::string &bucket_key);
	static std::string GetDataExpires(const std::string &bucket_key);
	static std::string GetDataRemaining(const std::string &bucket_key);

	static bool GetDataBuckets(Mob *mob);

	// scoped bucket methods
	static void SetData(const DataBucketKey &k);
	static bool DeleteData(const DataBucketKey &k);
	static DataBucketsRepository::DataBuckets GetData(const DataBucketKey &k, bool ignore_misses_cache = false);
	static std::string GetDataExpires(const DataBucketKey &k);
	static std::string GetDataRemaining(const DataBucketKey &k);
	static std::string GetScopedDbFilters(const DataBucketKey &k);

	// bucket repository versus key matching
	static bool CheckBucketMatch(const DataBucketsRepository::DataBuckets &dbe, const DataBucketKey &k);
	static bool ExistsInCache(const DataBucketsRepository::DataBuckets &entry);

	static void BulkLoadEntitiesToCache(DataBucketLoadType::Type t, std::vector<uint32> ids);
	static void DeleteCachedBuckets(DataBucketLoadType::Type type, uint32 id);

	static void DeleteFromMissesCache(DataBucketsRepository::DataBuckets e);
	static void ClearCache();
	static void DeleteFromCache(uint64 id, DataBucketLoadType::Type type);
	static bool CanCache(const DataBucketKey &key);
};

#endif //EQEMU_DATABUCKET_H
