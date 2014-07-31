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
#include "../common/debug.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <list>
#include <map>

#include "../common/servertalk.h"
#include "../common/packet_functions.h"
#include "../common/md5.h"
#include "../common/packet_dump.h"
#include "worldserver.h"
#include "web_interface.h"

extern std::map<std::string, per_session_data_eqemu*> sessions;

WorldServer::WorldServer(std::string shared_key)
: WorldConnection(EmuTCPConnection::packetModeWebInterface, shared_key.c_str()){
	pTryReconnect = true;
}

WorldServer::~WorldServer(){
}

void WorldServer::OnConnected(){
	_log(WEB_INTERFACE__INIT, "Connected to World.");
	WorldConnection::OnConnected();
}

void WorldServer::Process(){
	WorldConnection::Process();
	if (!Connected())
		return;

	ServerPacket *pack = nullptr;
	while((pack = tcpc.PopPacket())){
		_log(WEB_INTERFACE__TRACE, "Received Opcode: %4X", pack->opcode);
		switch(pack->opcode) {
			case 0: { break; }
			case ServerOP_KeepAlive: { break; }
			case ServerOP_WIRemoteCallResponse: {
				char *id = nullptr;
				char *session_id = nullptr;
				char *error = nullptr;

				id = new char[pack->ReadUInt32() + 1];
				pack->ReadString(id);

				session_id = new char[pack->ReadUInt32() + 1];
				pack->ReadString(session_id);

				error = new char[pack->ReadUInt32() + 1];
				pack->ReadString(error);

				uint32 param_count = pack->ReadUInt32();
				std::vector<std::string> params;
				for(uint32 i = 0; i < param_count; ++i) {
					char *p = new char[pack->ReadUInt32() + 1];
					pack->ReadString(p);
					params.push_back(p);
					safe_delete_array(p);
				}

				//send the response to client...
				rapidjson::StringBuffer s;
				rapidjson::Writer<rapidjson::StringBuffer> writer(s);
				writer.StartObject();
				writer.String("id");
				if(strlen(id) == 0) {
					writer.Null();
				} else {
					writer.String(id);
				}

				if(strlen(error) != 0) {
					writer.String("error");
					writer.Bool(true);

					writer.String("result");
					writer.String(error);
				} else {
					writer.String("error");
					writer.Null();
					writer.String("result");
					writer.StartArray();
					uint32 p_sz = (uint32)params.size();
					for(uint32 i = 0; i < p_sz; ++i) {
						writer.String(params[i].c_str());
					}
					writer.EndArray();
				}

				writer.EndObject();

				if(sessions.count(session_id) != 0) {
					per_session_data_eqemu *session = sessions[session_id];
					session->send_queue->push_back(s.GetString());
				}

				safe_delete_array(id);
				safe_delete_array(session_id);
				safe_delete_array(error);
				break;
			}
		}
	}

	safe_delete(pack);
	return;
}
