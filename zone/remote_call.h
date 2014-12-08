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
#ifndef ZONE_REMOTE_CALL_H
#define ZONE_REMOTE_CALL_H

#include <map>
#include <string>
#include <vector>

typedef void(*RemoteCallHandler)(const std::string&, const std::string&, const std::string&, const std::vector<std::string>&);

void RemoteCallResponse(const std::string &connection_id, const std::string &request_id, const std::map<std::string, std::string> &res, const std::string &error);
void RemoteCall(const std::string &connection_id, const std::string &method, const std::vector<std::string> &params);

void register_remote_call_handlers();

void handle_rc_subscribe(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_unsubscribe(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_get_initial_entity_positions(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_move_entity(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_zone_action(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_get_entity_attributes(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);
void handle_rc_set_entity_attribute(const std::string &method, const std::string &connection_id, const std::string &request_id, const std::vector<std::string> &params);

#endif

