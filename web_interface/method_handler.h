/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2014 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef WI_METHOD_HANDLER_H
#define WI_METHOD_HANDLER_H

#define CheckParams(sz, msg) if (!document.HasMember("params")) { \
	WriteWebCallResponseString(session, document, "Missing parameters, expected: " + std::string(msg), true); \
	if (!document.HasMember("params") && sz > 0) { \
		WriteWebCallResponseString(session, document, "Missing parameters, expected: " + std::string(msg), true); \
		return; \
	} \
	auto &params = document["params"]; \
	if(!params.IsArray()) { \
		WriteWebCallResponseString(session, document, "Missing parameters, expected: " + std::string(msg), true); \
		return; \
	} \
	if(params.Size() != sz) { \
		WriteWebCallResponseString(session, document, "Missing parameters, expected: " + std::string(msg), true); \
		return; \
	} \
} \

#define VerifyID() std::string id; \
	if (document.HasMember("id")) { \
		id = document["id"].GetString(); \
	} \


void register_methods();
void handle_method_token_auth(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method);
void handle_method_no_args(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method);
void handle_method_get_zone_info(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method);
void handle_method_subscribe(per_session_data_eqemu *session, rapidjson::Document &document, std::string &method);

#endif

