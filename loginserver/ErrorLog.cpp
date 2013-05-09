/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

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
#include <string.h>
#include "ErrorLog.h"

const char *eqLogTypes[_log_largest_type] =
{
	"Debug",
	"Error",
	"Database",
	"Network",
	"Network Trace",
	"Network Error",
	"World",
	"World Error",
	"Client",
	"Client Error"
};

ErrorLog::ErrorLog(const char* file_name)
{
	log_mutex = new Mutex();
	error_log = fopen(file_name, "w");
}

ErrorLog::~ErrorLog()
{
	log_mutex->lock();
	if(error_log)
	{
		fclose(error_log);
	}
	log_mutex->unlock();
	delete log_mutex;
}

void ErrorLog::Log(eqLogType type, const char *message, ...)
{
	if(type >= _log_largest_type)
	{
		return;
	}

	va_list argptr;
	char *buffer = new char[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	time_t m_clock;
	struct tm *m_time;
	time(&m_clock);
	m_time = localtime(&m_clock);

	log_mutex->lock();
	printf("[%s] [%02d.%02d.%02d - %02d:%02d:%02d] %s\n",
		eqLogTypes[type],
		m_time->tm_mon+1,
		m_time->tm_mday,
		m_time->tm_year%100,
		m_time->tm_hour,
		m_time->tm_min,
		m_time->tm_sec,
		buffer);

	if(error_log)
	{
		fprintf(error_log, "[%s] [%02d.%02d.%02d - %02d:%02d:%02d] %s\n",
			eqLogTypes[type],
			m_time->tm_mon+1,
			m_time->tm_mday,
			m_time->tm_year%100,
			m_time->tm_hour,
			m_time->tm_min,
			m_time->tm_sec,
			buffer);
		fflush(error_log);
	}

	log_mutex->unlock();
	delete[] buffer;
}

void ErrorLog::LogPacket(eqLogType type, const char *data, size_t size)
{
	if(type >= _log_largest_type)
	{
		return;
	}

	log_mutex->lock();
	time_t m_clock;
	struct tm *m_time;
	time(&m_clock);
	m_time = localtime(&m_clock);

	log_mutex->lock();
	printf("[%s] [%02d.%02d.%02d - %02d:%02d:%02d] dumping packet of size %u:\n",
		eqLogTypes[type],
		m_time->tm_mon+1,
		m_time->tm_mday,
		m_time->tm_year%100,
		m_time->tm_hour,
		m_time->tm_min,
		m_time->tm_sec,
		(unsigned int)size);

	if(error_log)
	{
		fprintf(error_log, "[%s] [%02d.%02d.%02d - %02d:%02d:%02d] dumping packet of size %u\n",
			eqLogTypes[type],
			m_time->tm_mon+1,
			m_time->tm_mday,
			m_time->tm_year%100,
			m_time->tm_hour,
			m_time->tm_min,
			m_time->tm_sec,
			(unsigned int)size);
	}

	char ascii[17]; //16 columns + 1 null term
	memset(ascii, 0, 17);

	size_t j = 0;
	size_t i = 0;
	for(; i < size; ++i)
	{
		if(i % 16 == 0)
		{
			if(i != 0)
			{
				printf(" | %s\n", ascii);
				if(error_log)
				{
					fprintf(error_log, " | %s\n", ascii);
				}
			}
			printf("%.4u: ", (unsigned int)i);
			memset(ascii, 0, 17);
			j = 0;
		}
		else if(i % 8 == 0)
		{
			printf("- ");
			if(error_log)
			{
				fprintf(error_log, "- ");
			}
		}

		printf("%02X ", (unsigned int)data[i]);
		if(error_log)
		{
			fprintf(error_log, "%02X ", (unsigned int)data[i]);
		}

		if(data[i] >= 32 && data[i] < 127)
		{
			ascii[j++] = data[i];
		}
		else
		{
			ascii[j++] = '.';
		}
	}

	size_t k = (i - 1) % 16;
	if(k < 8)
	{
		printf("  ");
		if(error_log)
		{
			fprintf(error_log, "  ");
		}
	}

	for(size_t h = k + 1; h < 16; ++h)
	{
		printf("   ");
		if(error_log)
		{
			fprintf(error_log, "   ");
		}
	}

	printf(" | %s\n", ascii);
	if(error_log)
	{
		fprintf(error_log, " | %s\n", ascii);
		fflush(error_log);
	}

	log_mutex->unlock();
}

