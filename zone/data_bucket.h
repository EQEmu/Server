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
	static void SetData(const std::string& bucket_key, const std::string& bucket_value, std::string expires_time = "");
	static bool DeleteData(const std::string& bucket_key);
	static std::string GetData(const std::string& bucket_key);
	static std::string GetDataExpires(const std::string& bucket_key);
	static std::string GetDataRemaining(const std::string& bucket_key);
	static bool GetDataBuckets(Mob* mob);
	static std::string CheckBucketKey(const Mob* mob, std::string_view full_name);

private:
	static uint64 DoesBucketExist(const std::string& bucket_key);
};

#endif //EQEMU_DATABUCKET_H
