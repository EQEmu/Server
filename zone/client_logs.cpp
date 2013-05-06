/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2004  EQEMu Development Team (http://eqemulator.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include "../common/features.h"

#ifdef CLIENT_LOGS
#include "client_logs.h"
#include "client.h"
#include <stdarg.h>

ClientLogs client_logs;

char ClientLogs::_buffer[MAX_CLIENT_LOG_MESSAGE_LENGTH+1];

void ClientLogs::subscribe(EQEMuLog::LogIDs id, Client *c) {
	if(id >= EQEMuLog::MaxLogID)
		return;
	if(c == nullptr)
		return;
	
	//make sure they arnt allready subscribed.
	
	vector<Client *>::iterator cur,end;
	cur = entries[id].begin();
	end = entries[id].end();
	for(; cur != end; cur++) {
		if(*cur == c) {
	printf("%s was allready subscribed to %d\n", c->GetName(), id);
			return;
		}
	}
	
	printf("%s has been subscribed to %d\n", c->GetName(), id);
	entries[id].push_back(c);
}

void ClientLogs::unsubscribe(EQEMuLog::LogIDs id, Client *c) {
	if(id >= EQEMuLog::MaxLogID)
		return;
	if(c == nullptr)
		return;
	
	vector<Client *>::iterator cur,end;
	cur = entries[id].begin();
	end = entries[id].end();
	for(; cur != end; cur++) {
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
	for(r = EQEMuLog::Status; r < EQEMuLog::MaxLogID; r++) {
		subscribe((EQEMuLog::LogIDs)r, c);
	}
}

void ClientLogs::unsubscribeAll(Client *c) {
	if(c == nullptr)
		return;
	int r;
	for(r = EQEMuLog::Status; r < EQEMuLog::MaxLogID; r++) {
		unsubscribe((EQEMuLog::LogIDs)r, c);
	}
}

void ClientLogs::clear() {
	int r;
	for(r = EQEMuLog::Status; r < EQEMuLog::MaxLogID; r++) {
		entries[r].clear();
	}
}

void ClientLogs::msg(EQEMuLog::LogIDs id, const char *buf) {
	if(id >= EQEMuLog::MaxLogID)
		return;
	vector<Client *>::iterator cur,end;
	cur = entries[id].begin();
	end = entries[id].end();
	for(; cur != end; cur++) {
		if(!(*cur)->InZone())
			continue;
		(*cur)->Message(CLIENT_LOG_CHANNEL, buf);
	}
}

void ClientLogs::EQEmuIO_buf(EQEMuLog::LogIDs id, const char *buf, uint8 size, uint32 count) {
	if(size != 1)
		return;	//cannot print multibyte data
	if(buf[0] == '\n' || buf[0] == '\r')
		return;	//skip new lines...
	if(count > MAX_CLIENT_LOG_MESSAGE_LENGTH)
		count = MAX_CLIENT_LOG_MESSAGE_LENGTH;
	memcpy(_buffer, buf, count);
	_buffer[count] = '\0';
	client_logs.msg(id, _buffer);
}

void ClientLogs::EQEmuIO_fmt(EQEMuLog::LogIDs id, const char *fmt, va_list ap) {
	if(fmt[0] == '\n' || fmt[0] == '\r')
		return;	//skip new lines...
	vsnprintf(_buffer, MAX_CLIENT_LOG_MESSAGE_LENGTH, fmt, ap);
	_buffer[MAX_CLIENT_LOG_MESSAGE_LENGTH] = '\0';
	client_logs.msg(id, _buffer);
}

void ClientLogs::EQEmuIO_pva(EQEMuLog::LogIDs id, const char *prefix, const char *fmt, va_list ap) {
	if(fmt[0] == '\n' || fmt[0] == '\r')
		return;	//skip new lines...
	char *buf = _buffer;
	int plen = snprintf(buf, MAX_CLIENT_LOG_MESSAGE_LENGTH, prefix);
	buf += plen;
	vsnprintf(buf, MAX_CLIENT_LOG_MESSAGE_LENGTH-plen, fmt, ap);
	_buffer[MAX_CLIENT_LOG_MESSAGE_LENGTH] = '\0';
	client_logs.msg(id, _buffer);
}

#endif //CLIENT_LOGS



