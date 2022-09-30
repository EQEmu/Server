#ifndef EQEMU_WORLD_API_H
#define EQEMU_WORLD_API_H

#include "../../common/http/httplib.h"

class WorldApi {
public:
	static void BootWebserver(int port = 0, const std::string& sidecar_key = "");
	static void LogHandler(const httplib::Request &req, const httplib::Response &res);
};


#endif //EQEMU_WORLD_API_H
