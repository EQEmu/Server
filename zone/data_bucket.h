//
// Created by Akkadius on 7/7/18.
//

#ifndef EQEMU_DATABUCKET_H
#define EQEMU_DATABUCKET_H


#include <string>
#include "../common/types.h"

class DataBucket {
public:
	static void SetData(std::string bucket_key, std::string bucket_value, std::string expires_time = "");
	static bool DeleteData(std::string bucket_key);
	static std::string GetData(std::string bucket_key);
	static std::string GetDataExpires(std::string bucket_key);
	static std::string GetDataRemaining(std::string bucket_key);
private:
	static uint64 DoesBucketExist(std::string bucket_key);
	static uint32 ParseStringTimeToInt(std::string time_string);
};


#endif //EQEMU_DATABUCKET_H
