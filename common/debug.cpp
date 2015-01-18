#include <iostream>
#include <string>

#ifdef _WINDOWS
	#include <process.h>

	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp

	#include <conio.h>
	#include <iostream>
	#include <dos.h>

namespace ConsoleColor {
	enum Colors {
		Black = 0,
		Blue = 1,
		Green = 2,
		Cyan = 3,
		Red = 4,
		Magenta = 5,
		Brown = 6,
		LightGray = 7,
		DarkGray = 8,
		LightBlue = 9,
		LightGreen = 10,
		LightCyan = 11,
		LightRed = 12,
		LightMagenta = 13,
		Yellow = 14,
		White = 15,
	};
}

#else

	#include <sys/types.h>
	#include <unistd.h>

#endif

#include "eqemu_logsys.h"
#include "debug.h"
#include "misc_functions.h"
#include "platform.h"
#include "eqemu_logsys.h"
#include "string_util.h"

#ifndef va_copy
	#define va_copy(d,s) ((d) = (s))
#endif

static volatile bool logFileValid = false;
static EQEmuLog realLogFile;
EQEmuLog *LogFile = &realLogFile;

static const char* FileNames[EQEmuLog::MaxLogID] = { "logs/eqemu", "logs/eqemu", "logs/eqemu_error", "logs/eqemu_debug", "logs/eqemu_quest", "logs/eqemu_commands", "logs/crash" };
static const char* LogNames[EQEmuLog::MaxLogID] = { "Status", "Normal", "Error", "Debug", "Quest", "Command", "Crash" };

EQEmuLog::EQEmuLog()
{
	pLogStatus[EQEmuLog::LogIDs::Status] = LOG_LEVEL_STATUS;
	pLogStatus[EQEmuLog::LogIDs::Normal] = LOG_LEVEL_NORMAL;
	pLogStatus[EQEmuLog::LogIDs::Error] = LOG_LEVEL_ERROR;
	pLogStatus[EQEmuLog::LogIDs::Debug] = LOG_LEVEL_DEBUG;
	pLogStatus[EQEmuLog::LogIDs::Quest] = LOG_LEVEL_QUEST;
	pLogStatus[EQEmuLog::LogIDs::Commands] = LOG_LEVEL_COMMANDS;
	pLogStatus[EQEmuLog::LogIDs::Crash] = LOG_LEVEL_CRASH;
	logFileValid = true;
}

EQEmuLog::~EQEmuLog()
{
	logFileValid = false;
	for (int i = 0; i < MaxLogID; i++) {
		LockMutex lock(&MLog[i]);	//to prevent termination race
		if (fp[i]) {
			fclose(fp[i]);
		}
	}
}