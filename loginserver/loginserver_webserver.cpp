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

#include "loginserver_webserver.h"
#include "server_manager.h"
#include "login_server.h"
#include "../common/json/json.h"
#include "../common/string_util.h"
#include "account_management.h"

extern LoginServer server;

namespace LoginserverWebserver {

	constexpr static int HTTP_RESPONSE_OK           = 200;
	constexpr static int HTTP_RESPONSE_BAD_REQUEST  = 400;
	constexpr static int HTTP_RESPONSE_UNAUTHORIZED = 401;

	/**
	 * @param api
	 */
	void RegisterRoutes(httplib::Server &api)
	{
		server.token_manager = new LoginserverWebserver::TokenManager;
		server.token_manager->LoadApiTokens();

		api.Get(
			"/v1/servers/list", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanRead(request, res)) {
					return;
				}

				Json::Value response;
				auto        iter = server.server_manager->getWorldServers().begin();
				while (iter != server.server_manager->getWorldServers().end()) {
					Json::Value row;
					row["server_long_name"]  = (*iter)->GetServerLongName();
					row["server_short_name"] = (*iter)->GetServerLongName();
					row["server_list_id"]    = (*iter)->GetServerListID();
					row["server_status"]     = (*iter)->GetStatus();
					row["zones_booted"]      = (*iter)->GetZonesBooted();
					row["local_ip"]          = (*iter)->GetLocalIP();
					row["remote_ip"]         = (*iter)->GetRemoteIP();
					row["players_online"]    = (*iter)->GetPlayersOnline();
					response.append(row);
					++iter;
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/create", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanWrite(request, res)) {
					return;
				}

				Json::Value request_body = LoginserverWebserver::ParseRequestBody(request);
				std::string username     = request_body.get("username", "").asString();
				std::string password     = request_body.get("password", "").asString();
				std::string email        = request_body.get("email", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					response["error"] = "Username or password not set";
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				int32 account_created_id = AccountManagement::CreateLoginServerAccount(username, password, email);
				if (account_created_id > 0) {
					response["message"]            = "Account created successfully!";
					response["data"]["account_id"] = account_created_id;
				}
				else if (account_created_id == -1) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Account already exists!";
				}
				else {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Account failed to create!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/create/external", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanWrite(request, res)) {
					return;
				}

				Json::Value request_body     = LoginserverWebserver::ParseRequestBody(request);
				std::string username         = request_body.get("username", "").asString();
				std::string password         = request_body.get("password", "").asString();
				std::string email            = request_body.get("email", "").asString();
				uint32      login_account_id = request_body.get("login_account_id", "").asInt();

				Json::Value response;
				if (username.empty() || password.empty()) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				std::string source_loginserver = "eqemu";
				int32       account_created_id = AccountManagement::CreateLoginServerAccount(
					username,
					password,
					email,
					source_loginserver,
					login_account_id
				);

				if (account_created_id > 0) {
					response["message"]            = "Account created successfully!";
					response["data"]["account_id"] = account_created_id;
				}
				else if (account_created_id == -1) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Account already exists!";
				}
				else {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Account failed to create!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/credentials/validate/local", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanRead(request, res)) {
					return;
				}

				Json::Value request_body = LoginserverWebserver::ParseRequestBody(request);
				std::string username     = request_body.get("username", "").asString();
				std::string password     = request_body.get("password", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				uint32 login_account_id = AccountManagement::CheckLoginserverUserCredentials(
					username,
					password
				);

				if (login_account_id > 0) {
					response["message"]            = "Credentials valid!";
					response["data"]["account_id"] = login_account_id;
				}
				else {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Credentials invalid!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/credentials/update/local", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanWrite(request, res)) {
					return;
				}

				Json::Value request_body = LoginserverWebserver::ParseRequestBody(request);
				std::string username     = request_body.get("username", "").asString();
				std::string password     = request_body.get("password", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				Database::DbLoginServerAccount
					login_server_account = server.db->GetLoginServerAccountByAccountName(
					username
				);

				if (!login_server_account.loaded) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Failed to find associated loginserver account!";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				bool credentials_valid = AccountManagement::UpdateLoginserverUserCredentials(
					username,
					password
				);

				if (credentials_valid) {
					response["message"] = "Loginserver account credentials updated!";
				}
				else {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Failed to update loginserver account credentials!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);


		api.Post(
			"/v1/account/credentials/update/external", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanWrite(request, res)) {
					return;
				}

				Json::Value request_body = LoginserverWebserver::ParseRequestBody(request);
				std::string username     = request_body.get("username", "").asString();
				std::string password     = request_body.get("password", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				std::string source_loginserver = "eqemu";

				Database::DbLoginServerAccount
					login_server_account = server.db->GetLoginServerAccountByAccountName(
					username,
					source_loginserver
				);

				if (!login_server_account.loaded) {
					response["error"] = "Failed to find associated loginserver account!";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				bool credentials_valid = AccountManagement::UpdateLoginserverUserCredentials(
					username,
					password,
					source_loginserver
				);

				if (credentials_valid) {
					response["message"] = "Loginserver account credentials updated!";
				}
				else {
					response["error"] = "Failed to update loginserver account credentials!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/credentials/validate/external", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanRead(request, res)) {
					return;
				}

				Json::Value request_body = LoginserverWebserver::ParseRequestBody(request);
				std::string username     = request_body.get("username", "").asString();
				std::string password     = request_body.get("password", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				uint32 account_id = AccountManagement::CheckExternalLoginserverUserCredentials(
					username,
					password
				);

				if (account_id > 0) {
					response["message"]            = "Credentials valid!";
					response["data"]["account_id"] = account_id;
				}
				else {
					response["error"] = "Credentials invalid!";
					res.status = HTTP_RESPONSE_BAD_REQUEST;
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);
	}

	/**
	 * @param payload
	 * @param res
	 */
	void SendResponse(const Json::Value &payload, httplib::Response &res)
	{
		if (res.get_header_value("response_set") == "true") {
			res.set_header("response_set", "");
			return;
		}

		std::stringstream response_payload;

		if (payload.empty()) {
			Json::Value response;
			response["message"] = "There were no results found";
			response_payload << response;
			res.set_content(response_payload.str(), "application/json");
			return;
		}

		response_payload << payload;
		res.set_content(response_payload.str(), "application/json");
	}

	/**
	 * @param payload
	 * @param res
	 */
	Json::Value ParseRequestBody(const httplib::Request &request)
	{
		Json::Value request_body;

		try {
			std::stringstream ss;
			ss.str(request.body);
			ss >> request_body;
		}
		catch (std::exception &) {
			request_body["error"] = "Payload invalid";

			return request_body;
		}

		return request_body;
	}

	/**
	 * @param request
	 * @param res
	 */
	bool LoginserverWebserver::TokenManager::AuthCanRead(const httplib::Request &request, httplib::Response &res)
	{
		LoginserverWebserver::TokenManager::token_data
			user_token = LoginserverWebserver::TokenManager::CheckApiAuthorizationHeaders(request);

		if (!user_token.can_read) {
			Json::Value       response;
			std::stringstream response_payload;
			response["message"] = "Authorization token is either invalid or cannot read!";
			response_payload << response;

			res.status = HTTP_RESPONSE_UNAUTHORIZED;
			res.set_content(response_payload.str(), "application/json");
			res.set_header("response_set", "true");

			LogWarning(
				"AuthCanRead access failure remote_address [{0}] user_agent [{1}]",
				user_token.remote_address,
				user_token.user_agent
			);

			return false;
		}

		return true;
	}

	/**
	 * @param request
	 * @param res
	 */
	bool LoginserverWebserver::TokenManager::AuthCanWrite(const httplib::Request &request, httplib::Response &res)
	{
		LoginserverWebserver::TokenManager::token_data
			user_token = LoginserverWebserver::TokenManager::CheckApiAuthorizationHeaders(request);

		if (!user_token.can_write) {
			Json::Value       response;
			std::stringstream response_payload;
			response["message"] = "Authorization token is either invalid or cannot write!";
			response_payload << response;

			res.status = HTTP_RESPONSE_UNAUTHORIZED;
			res.set_content(response_payload.str(), "application/json");
			res.set_header("response_set", "true");

			LogWarning(
				"AuthCanWrite access failure remote_address [{0}] user_agent [{1}]",
				user_token.remote_address,
				user_token.user_agent
			);

			return false;
		}

		return true;
	}

	/**
	 * @param request
	 * @return
	 */
	LoginserverWebserver::TokenManager::token_data
	LoginserverWebserver::TokenManager::CheckApiAuthorizationHeaders(
		const httplib::Request &request
	)
	{
		std::string authorization_key;

		LoginserverWebserver::TokenManager::token_data user_token{};

		for (const auto &header : request.headers) {
			auto header_key   = header.first;
			auto header_value = header.second;
			if (header_key == "Authorization") {
				authorization_key = header.second;
				find_replace(authorization_key, "Bearer ", "");
				if (LoginserverWebserver::TokenManager::TokenExists(authorization_key)) {
					user_token = server.token_manager->GetToken(authorization_key);
				}
			}

			if (header_key == "REMOTE_ADDR") {
				user_token.remote_address = header.second;
			}

			if (header_key == "User-Agent") {
				user_token.user_agent = header.second;
			}
		}

		LogDebug(
			"Authentication Request | remote_address [{0}] user_agent [{1}] authorization_key [{2}] request_path [{3}]",
			user_token.remote_address,
			user_token.user_agent,
			authorization_key,
			request.path
		);

		return user_token;
	}

	/**
	 * Loads API Tokens
	 */
	void TokenManager::LoadApiTokens()
	{
		auto      results     = server.db->GetLoginserverApiTokens();
		int       token_count = 0;
		for (auto row         = results.begin(); row != results.end(); ++row) {
			LoginserverWebserver::TokenManager::token_data token_data;
			token_data.token     = row[0];
			token_data.can_write = std::stoi(row[1]) > 0;
			token_data.can_read  = std::stoi(row[2]) > 0;

			LogDebug(
				"Inserting api token to internal list [{0}] write {1} read {2}",
				token_data.token,
				token_data.can_read,
				token_data.can_write
			);

			server.token_manager->loaded_api_tokens.insert(
				std::make_pair(
					token_data.token,
					token_data
				)
			);

			token_count++;
		}

		LogInfo("Loaded [{}] API token(s)", token_count);
	}

	/**
	 * @param token
	 * @return
	 */
	bool TokenManager::TokenExists(const std::string &token)
	{
		auto it = server.token_manager->loaded_api_tokens.find(token);

		return !(it == server.token_manager->loaded_api_tokens.end());
	}

	/**
	 * @param token
	 * @return
	 */
	LoginserverWebserver::TokenManager::token_data TokenManager::GetToken(
		const std::string &token
	)
	{
		auto iter = server.token_manager->loaded_api_tokens.find(token);
		if (iter != server.token_manager->loaded_api_tokens.end()) {
			return iter->second;
		}

		return {};
	}
}
