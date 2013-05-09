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

#ifndef CLIENT_LOGS_H
#define CLIENT_LOGS_H
#include "../common/debug.h"
#include "../common/features.h"

#ifdef CLIENT_LOGS
#include "../common/eq_packet_structs.h"

#define CLIENT_LOG_CHANNEL MT_Chat10Echo

//trim messages to this length before sending to any clients
#define MAX_CLIENT_LOG_MESSAGE_LENGTH 512

#include <vector>
using namespace std;

class Client;

class ClientLogs {
public:
	static void EQEmuIO_buf(EQEMuLog::LogIDs id, const char *buf, uint8 size, uint32 count);
	static void EQEmuIO_fmt(EQEMuLog::LogIDs id, const char *fmt, va_list ap);
	static void EQEmuIO_pva(EQEMuLog::LogIDs id, const char *prefix, const char *fmt, va_list ap);

	void subscribe(EQEMuLog::LogIDs id, Client *c);
	void unsubscribe(EQEMuLog::LogIDs id, Client *c);
	void subscribeAll(Client *c);
	void unsubscribeAll(Client *c);
	void clear();	//unsubscribes everybody

	void msg(EQEMuLog::LogIDs id, const char *buf);

protected:

	vector<Client *> entries[EQEMuLog::MaxLogID];

	static char _buffer[MAX_CLIENT_LOG_MESSAGE_LENGTH+1];
};

extern ClientLogs client_logs;

#endif //CLIENT_LOGS
#endif

