/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/logsys.h"
#include "../common/base_packet.h"
#include "mob.h"
#include <stdarg.h>
#include <stdio.h>

void log_message_mob(LogType type, Mob *who, const char *fmt, ...) {
	if(!who->IsLoggingEnabled())
		return;	//could prolly put this in the macro, but it feels even dirtier than prototyping this in common

	char prefix_buffer[256];
	snprintf(prefix_buffer, 255, "[%s] %s: ", log_type_info[type].name, who->GetName());
	prefix_buffer[255] = '\0';

	va_list args;
	va_start(args, fmt);
	LogFile->writePVA(EQEMuLog::Debug, prefix_buffer, fmt, args);
	va_end(args);
}

void log_message_mobVA(LogType type, Mob *who, const char *fmt, va_list args) {
	if(!who->IsLoggingEnabled())
		return;	//could prolly put this in the macro, but it feels even dirtier than prototyping this in common

	char prefix_buffer[256];
	snprintf(prefix_buffer, 255, "[%s] %s: ", log_type_info[type].name, who->GetName());
	prefix_buffer[255] = '\0';

	LogFile->writePVA(EQEMuLog::Debug, prefix_buffer, fmt, args);
}

void log_hex_mob(LogType type, Mob *who, const char *data, uint32 length, uint8 padding) {
	if(!who->IsLoggingEnabled())
		return;	//could prolly put this in the macro, but it feels even dirtier than prototyping this in common

	log_hex(type,data,length,padding);
}

void log_packet_mob(LogType type, Mob *who, const BasePacket *p) {
	if(!who->IsLoggingEnabled())
		return;	//could prolly put this in the macro, but it feels even dirtier than prototyping this in common

	char buffer[80];
	p->build_header_dump(buffer);
	log_message(type,"[%s] %s: %s", log_type_info[type].name, who->GetName(), buffer);
	log_hex(type,(const char *)p->pBuffer,p->size);
}

