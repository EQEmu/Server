/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/features.h"
#include "../common/eqemu_logsys.h"

#ifdef CLIENT_LOGS
#include "client_logs.h"
#include "client.h"
#include "entity.h"
#include <stdarg.h>

ClientLogs client_logs;

char ClientLogs::_buffer[MAX_CLIENT_LOG_MESSAGE_LENGTH+1];

void ClientLogs::subscribe(EQEmuLog::LogIDs id, Client *c) {
	if(id >= EQEmuLog::MaxLogID)
		return;
	if(c == nullptr)
		return;

	std::vector<Client *>::iterator cur,end;
	cur = entries[id].begin();
	end = entries[id].end();
	for(; cur != end; ++cur) {
		if(*cur == c) {
			printf("%s was already subscribed to %d\n", c->GetName(), id);
			return;
		}
	}

	printf("%s has been subscribed to %d\n", c->GetName(), id);
	entries[id].push_back(c);
}

void ClientLogs::unsubscribe(EQEmuLog::LogIDs id, Client *c) {
	if(id >= EQEmuLog::MaxLogID)
		return;
	if(c == nullptr)
		return;

	std::vector<Client *>::iterator cur,end;
	cur = entries[id].begin();
	end = entries[id].end();
	for(; cur != end; ++cur) {
		if(*cur == c) {
			entries[id].erase(cur);
			return;
		}
	}
}

void ClientLogs::subscribeAll(Client *c) {
	if(c == nullptr)
		return;
	int r;
	for(r = EQEmuLog::Status; r < EQEmuLog::MaxLogID; r++) {
		subscribe((EQEmuLog::LogIDs)r, c);
	}
}

void ClientLogs::unsubscribeAll(Client *c) {
	if(c == nullptr)
		return;
	int r;
	for(r = EQEmuLog::Status; r < EQEmuLog::MaxLogID; r++) {
		unsubscribe((EQEmuLog::LogIDs)r, c);
	}
}

void ClientLogs::clear() {
	int r;
	for(r = EQEmuLog::Status; r < EQEmuLog::MaxLogID; r++) {
		entries[r].clear();
	}
}

void ClientLogs::msg(EQEmuLog::LogIDs id, const char *buf) {
	if(id >= EQEmuLog::MaxLogID)
		return;
	std::vector<Client *>::iterator cur,end;
	cur = entries[id].begin();
	end = entries[id].end();
	for(; cur != end; ++cur) {
		if(!(*cur)->InZone())
			continue;

		(*cur)->Message(CLIENT_LOG_CHANNEL, buf);
	}
}

static uint32 gmsay_log_message_colors[EQEmuLogSys::MaxLogID] = {
	15, // "Status", - Yellow
	15,	// "Normal", - Yellow
	3,	// "Error", - Red
	14,	// "Debug", - Light Green
	4,	// "Quest", 
	5,	// "Command", 
	3	// "Crash" 
};

void ClientLogs::ClientMessage(uint16 log_type, std::string& message){
	entity_list.MessageStatus(0, 80, gmsay_log_message_colors[log_type], "%s", message.c_str());
}

#endif //CLIENT_LOGS



