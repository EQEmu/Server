#ifndef EQEMU_LOGINSERVER_WEBSERVER_H
#define EQEMU_LOGINSERVER_WEBSERVER_H

#include "../common/http/httplib.h"
#include "../common/json/json.h"
#include "../common/types.h"

namespace LoginserverWebserver {

	class TokenManager {

	public:
		TokenManager() = default;

		struct token_data {
			std::string token;
			bool can_read;
			bool can_write;
			std::string user_agent;
			std::string remote_address;
		};

		std::map<std::string, token_data> loaded_api_tokens{};

		void LoadApiTokens();
		static bool TokenExists(const std::string &token);
		token_data GetToken(const std::string &token);
		static token_data CheckApiAuthorizationHeaders(const httplib::Request &request);
		static bool AuthCanRead(const httplib::Request &request, httplib::Response &res);
		static bool AuthCanWrite(const httplib::Request &request, httplib::Response &res);
	};

	void RegisterRoutes(httplib::Server &api);
	void SendResponse(const Json::Value &payload, httplib::Response &res);
	static Json::Value ParseRequestBody(const httplib::Request &request);
};

#endif //EQEMU_LOGINSERVER_WEBSERVER_H
