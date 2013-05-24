
#include "../common/debug.h"
#include "../common/logsys.h"
#include "../common/StringUtil.h"

#include "zoneserver.h"
#include "client.h"

#include <stdarg.h>
#include <stdio.h>


void log_message_clientVA(LogType type, Client *who, const char *fmt, va_list args) {

	std::string prefix_buffer;
	StringFormat(prefix_buffer,"[%s] %s: ", log_type_info[type].name, who->GetAccountName());

	LogFile->writePVA(EQEMuLog::Debug, prefix_buffer.c_str(), fmt, args);
}

void log_message_client(LogType type, Client *who, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_message_clientVA(type, who, fmt, args);
	va_end(args);
}

void log_message_zoneVA(LogType type, ZoneServer *who, const char *fmt, va_list args) {

	std::string prefix_buffer, zone_tag;
	const char *zone_name=who->GetZoneName();
	
	if (zone_name == nullptr)
        StringFormat(zone_tag,"[%d]", who->GetID());
	else
		StringFormat(zone_tag,"[%d] [%s]",who->GetID(),zone_name);

	StringFormat(prefix_buffer, "[%s] %s ", log_type_info[type].name, zone_tag.c_str());

	LogFile->writePVA(EQEMuLog::Debug, prefix_buffer.c_str(), fmt, args);
}

void log_message_zone(LogType type, ZoneServer *who, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_message_zoneVA(type, who, fmt, args);
	va_end(args);
}












