//
// Created by Akkadius on 7/7/18.
//

#ifndef EQEMU_DATABUCKET_H
#define EQEMU_DATABUCKET_H

#include <string>
#include "../common/types.h"
#include "../common/repositories/data_buckets_repository.h"
#include "mob.h"

class DataBucket {
public:
	static void SetData(std::string bucket_key, std::string bucket_value, std::string expires_time = "");
	static bool DeleteData(std::string bucket_key);
	static std::string GetData(std::string bucket_key);
	static std::string GetDataExpires(std::string bucket_key);
	static std::string GetDataRemaining(std::string bucket_key);
	static bool GetDataBuckets(Mob* mob);
	static std::string CheckBucketKey(const Mob* mob, std::string_view full_name);

private:
	static uint64 DoesBucketExist(std::string bucket_key);
};

#endif //EQEMU_DATABUCKET_H
