/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

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
