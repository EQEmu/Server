
#include "../common/debug.h"
#include "../common/logsys.h"
#include "zoneserver.h"
#include "client.h"
#include <stdarg.h>
#include <stdio.h>


void log_message_clientVA(LogType type, Client *who, const char *fmt, va_list args) {
	char prefix_buffer[256];
	snprintf(prefix_buffer, 255, "[%s] %s: ", log_type_info[type].name, who->GetAccountName());
	prefix_buffer[255] = '\0';

	LogFile->writePVA(EQEMuLog::Debug, prefix_buffer, fmt, args);
}

void log_message_client(LogType type, Client *who, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_message_clientVA(type, who, fmt, args);
	va_end(args);
}

void log_message_zoneVA(LogType type, ZoneServer *who, const char *fmt, va_list args) {

	char prefix_buffer[256];
	char zone_tag[65];
	const char *zone_name=who->GetZoneName();
	if (*zone_name==0)
		snprintf(zone_tag,64,"[%d]", who->GetID());
	else
		snprintf(zone_tag,64,"[%d] [%s]",who->GetID(),zone_name);

	snprintf(prefix_buffer, 255, "[%s] %s ", log_type_info[type].name, zone_tag);
	prefix_buffer[255] = '\0';

	LogFile->writePVA(EQEMuLog::Debug, prefix_buffer, fmt, args);
}

void log_message_zone(LogType type, ZoneServer *who, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_message_zoneVA(type, who, fmt, args);
	va_end(args);
}












