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
#ifndef WORLD_REMOTE_CALL_H
#define WORLD_REMOTE_CALL_H

#include <map>
#include <string>
#include <vector>

typedef void(*RemoteCallHandler)(const std::string&, const std::string&, const std::string&, const std::vector<std::string>&);

void RemoteCallResponse(const std::string &connection_id, const std::string &request_id, const std::map<std::string, std::string> &res, const std::string &error);

void register_remote_call_handlers();
void handle_rc_list_zones(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_get_zone_info(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_relay(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_get_file_contents(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_save_file_contents(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);

#endif

