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
#include "../common/web_interface_utils.h"
#include "worldserver.h"

struct per_session_data_eqemu {
	bool auth;
	std::string uuid;
	std::list<std::string> *send_queue;
};

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
			case ServerOP_WIWorldResponse: {
				/* Generic Response routine: web_interface server recieves packet from World - 
					Relays data back to client
				*/
				_log(WEB_INTERFACE__ERROR, "WI Recieved packet from world 0x%04x, size %d", pack->opcode, pack->size);
				WI_Client_Request_Struct* WICR = (WI_Client_Request_Struct*)pack->pBuffer;
				std::string Data;
				Data.assign(WICR->JSON_Data, pack->size - 64);
				/* Check if Session is Valid before sending data back*/
				if (sessions[WICR->Client_UUID]){
					sessions[WICR->Client_UUID]->send_queue->push_back(Data.c_str()); 
				}
				break;
			}
		}
	}

	safe_delete(pack);
	return;
}
