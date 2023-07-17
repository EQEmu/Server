//
// Created by Akkadius on 7/7/18.
//

#ifndef EQEMU_DATABUCKET_H
#define EQEMU_DATABUCKET_H

#include <string>
#include "../common/types.h"
#include "../common/repositories/data_buckets_repository.h"
#include "mob.h"

struct DataBucketEntry {
	DataBucketsRepository::DataBuckets e;
	uint64_t                           updated_time;
};

struct DataBucketKey {
	std::string key;
	std::string value;
	std::string expires;
	int64_t     character_id;
	int64_t     npc_id;
	int64_t     bot_id;
};

namespace DataBucketLoadType {
	enum Type : uint8 {
		Bot,
		Client,
		NPC,
		MaxType
	};

	static const std::string Name[Type::MaxType] = {
		"Bot",
		"Client",
		"NPC",
	};
}

class DataBucket {
public:
	// non-scoped bucket methods (for global buckets)
	static void SetData(const std::string& bucket_key, const std::string& bucket_value, std::string expires_time = "");
	static bool DeleteData(const std::string& bucket_key);
	static std::string GetData(const std::string& bucket_key);
	static std::string GetDataExpires(const std::string& bucket_key);
	static std::string GetDataRemaining(const std::string& bucket_key);

	static bool GetDataBuckets(Mob* mob);

	static uint64_t GetCurrentTimeUNIX();

	// scoped bucket methods
	static void SetData(const DataBucketKey& k);
	static bool DeleteData(const DataBucketKey& k);
	static DataBucketsRepository::DataBuckets GetData(const DataBucketKey& k);
	static std::string GetDataExpires(const DataBucketKey& k);
	static std::string GetDataRemaining(const DataBucketKey& k);
	static std::string GetScopedDbFilters(const DataBucketKey& k);

	// bucket repository versus key matching
	static bool CheckBucketMatch(const DataBucketsRepository::DataBuckets& dbe, const DataBucketKey& k);
	static bool ExistsInCache(const DataBucketsRepository::DataBuckets& e);

	static void BulkLoadEntities(DataBucketLoadType::Type t, std::vector<uint32> ids);
	static void DeleteCachedBuckets(DataBucketLoadType::Type t, uint32 id);
};

#endif //EQEMU_DATABUCKET_H
