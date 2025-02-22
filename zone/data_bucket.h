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
	uint64_t    account_id   = 0;
	uint64_t    character_id = 0;
	uint32_t    npc_id       = 0;
	uint32_t    bot_id       = 0;
	uint16_t    zone_id      = 0;
	uint16_t    instance_id  = 0;
};

namespace DataBucketLoadType {
	enum Type : uint8 {
		Bot,
		Account,
		Client,
		Zone,
		MaxType
	};

	static const std::string Name[Type::MaxType] = {
		"Bot",
		"Account",
		"Client",
		"Zone"
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
	static void SetData(const DataBucketKey &k_);
	static bool DeleteData(const DataBucketKey &k);
	static DataBucketsRepository::DataBuckets GetData(const DataBucketKey &k_, bool ignore_misses_cache = false);
	static std::string GetDataExpires(const DataBucketKey &k);
	static std::string GetDataRemaining(const DataBucketKey &k);
	static std::string GetScopedDbFilters(const DataBucketKey &k);

	// bucket repository versus key matching
	static bool CheckBucketMatch(const DataBucketsRepository::DataBuckets &dbe, const DataBucketKey &k);
	static bool ExistsInCache(const DataBucketsRepository::DataBuckets &entry);

	static void LoadZoneCache(uint16 zone_id, uint16 instance_id);
	static void BulkLoadEntitiesToCache(DataBucketLoadType::Type t, std::vector<uint32> ids);
	static void DeleteCachedBuckets(DataBucketLoadType::Type type, uint32 id, uint32 secondary_id = 0);

	static void DeleteFromMissesCache(DataBucketsRepository::DataBuckets e);
	static void ClearCache();
	static void DeleteFromCache(uint64 id, DataBucketLoadType::Type type);
	static void DeleteZoneFromCache(uint16 zone_id, uint16 instance_id, DataBucketLoadType::Type type);
	static bool CanCache(const DataBucketKey &key);
	static DataBucketsRepository::DataBuckets
	ExtractNestedValue(const DataBucketsRepository::DataBuckets &bucket, const std::string &full_key);
};

#endif //EQEMU_DATABUCKET_H
