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
#ifndef EQEMU_ERROR_LOG_H
#define EQEMU_ERROR_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string>

#include "../common/Mutex.h"

using namespace std;

/**
 * Dictates the log type specified in ErrorLog for Log(...)
 */
enum eqLogType
{
	log_debug,
	log_error,
	log_database,
	log_network,
	log_network_trace,
	log_network_error,
	log_world,
	log_world_error,
	log_client,
	log_client_error,
	_log_largest_type
};

/**
 * Basic error logging class.
 * Thread safe logging class that records time and date to both a file and to console(if exists).
 */
class ErrorLog
{
public:
	/**
	* Constructor: opens the log file for writing and creates our mutex for writing to the log.
	*/
	ErrorLog(const char* file_name);

	/**
	* Closes the file and destroys the mutex.
	*/
	~ErrorLog();

	/**
	* Writes to the log system a variable message.
	*/
	void Log(eqLogType type, const char *message, ...);

	/**
	* Writes to the log system a packet.
	*/
	void LogPacket(eqLogType type, const char *data, size_t size);

protected:
	Mutex *log_mutex;
	FILE* error_log;
};

#endif

