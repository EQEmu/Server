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

#include "logsys.h"
#include "debug.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "misc.h"
#include "eq_packet.h"


#define LOG_CATEGORY(category) #category ,
const char *log_category_names[NUMBER_OF_LOG_CATEGORIES] = {
	#include "logtypes.h"
};


//this array is private to this file, only a const version of it is exposed
#define LOG_TYPE(category, type, enabled) { enabled, LOG_ ##category, #category "__" #type },
static LogTypeStatus real_log_type_info[NUMBER_OF_LOG_TYPES+1] =
{
	#include "logtypes.h"
	{ false, NUMBER_OF_LOG_CATEGORIES, "BAD TYPE" }	/* dummy trailing record */
};
const LogTypeStatus *log_type_info = real_log_type_info;



void log_hex(LogType type, const void *data, unsigned long length, unsigned char padding) {
	if(!is_log_enabled(type))
		return;
	char buffer[80];
	uint32 offset;
	for(offset=0;offset<length;offset+=16) {
		build_hex_line((const char *)data,length,offset,buffer,padding);
		log_message(type, "%s", buffer);	//%s is to prevent % escapes in the ascii
	}
}

void log_packet(LogType type, const BasePacket *p) {
	if(!is_log_enabled(type))
		return;
	char buffer[80];
	p->build_header_dump(buffer);
	log_message(type,"%s", buffer);
	log_hex(type,(const char *)p->pBuffer,p->size);
}

void log_raw_packet(LogType type, uint16 seq, const BasePacket *p) {
	if(!is_log_enabled(type))
		return;
	char buffer[196];
	p->build_raw_header_dump(buffer, seq);
	log_message(type,buffer);
	log_hex(type,(const char *)p->pBuffer,p->size);
}


void log_enable(LogType t) {
	real_log_type_info[t].enabled = true;
}

void log_disable(LogType t) {
	real_log_type_info[t].enabled = false;
}

void log_toggle(LogType t) {
	real_log_type_info[t].enabled = !real_log_type_info[t].enabled;
}


bool load_log_settings(const char *filename) {
	//this is a terrible algorithm, but im lazy today
	FILE *f = fopen(filename, "r");
	if(f == nullptr)
		return(false);
	char linebuf[512], type_name[256], value[256];
	while(!feof(f)) {
		if(fgets(linebuf, 512, f) == nullptr)
			continue;
#ifdef _WINDOWS
		if (sscanf(linebuf, "%[^=]=%[^\n]\n", type_name, value) != 2)
			continue;
#else
		if (sscanf(linebuf, "%[^=]=%[^\r\n]\n", type_name, value) != 2)
			continue;
#endif

		if(type_name[0] == '\0' || type_name[0] == '#')
			continue;

		//first make sure we understand the value
		bool enabled;
		if(!strcasecmp(value, "on") || !strcasecmp(value, "true") || !strcasecmp(value, "yes") || !strcasecmp(value, "enabled") || !strcmp(value, "1"))
			enabled = true;
		else if(!strcasecmp(value, "off") || !strcasecmp(value, "false") || !strcasecmp(value, "no") || !strcasecmp(value, "disabled") || !strcmp(value, "0"))
			enabled = false;
		else {
			printf("Unable to parse value '%s' from %s. Skipping line.", value, filename);
			continue;
		}

		int r;
		//first see if it is a category name
		for(r = 0; r < NUMBER_OF_LOG_CATEGORIES; r++) {
			if(!strcasecmp(log_category_names[r], type_name))
				break;
		}
		if(r != NUMBER_OF_LOG_CATEGORIES) {
			//matched a category.
			int k;
			for(k = 0; k < NUMBER_OF_LOG_TYPES; k++) {
				if(log_type_info[k].category != r)
					continue;	//does not match this category.
				if(enabled)
					log_enable(LogType(k));
				else
					log_disable(LogType(k));
			}
			continue;
		}

		for(r = 0; r < NUMBER_OF_LOG_TYPES; r++) {
			if(!strcasecmp(log_type_info[r].name, type_name))
				break;
		}
		if(r == NUMBER_OF_LOG_TYPES) {
			printf("Unable to locate log type %s from file %s. Skipping line.", type_name, filename);
			continue;
		}

		//got it all figured out, do something now...
		if(enabled)
			log_enable(LogType(r));
		else
			log_disable(LogType(r));
	}
	fclose(f);
	return(true);
}

