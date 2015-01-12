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
	for (int i = 0; i < MaxLogID; i++) {
		fp[i] = 0;
		logCallbackFmt[i] = nullptr;
		logCallbackBuf[i] = nullptr;
		logCallbackPva[i] = nullptr;
	}
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

bool EQEmuLog::open(LogIDs id)
{
	if (!logFileValid) {
		return false;
	}
	if (id >= MaxLogID) {
		return false;
	}
	LockMutex lock(&MOpen);
	if (pLogStatus[id] & 4) {
		return false;
	}
	if (fp[id]) {
		//cerr<<"Warning: LogFile already open"<<endl;
		return true;
	}
	char exename[200] = "";
	const EQEmuExePlatform &platform = GetExecutablePlatform();
	if (platform == ExePlatformWorld) {
		snprintf(exename, sizeof(exename), "_world");
	} else if (platform == ExePlatformZone) {
		snprintf(exename, sizeof(exename), "_zone");
	} else if (platform == ExePlatformLaunch) {
		snprintf(exename, sizeof(exename), "_launch");
	} else if (platform == ExePlatformUCS) {
		snprintf(exename, sizeof(exename), "_ucs");
	} else if (platform == ExePlatformQueryServ) {
		snprintf(exename, sizeof(exename), "_queryserv");
	} else if (platform == ExePlatformSharedMemory) {
		snprintf(exename, sizeof(exename), "_shared_memory");
	} else if (platform == ExePlatformClientImport) {
		snprintf(exename, sizeof(exename), "_import");
	} else if (platform == ExePlatformClientExport) {
		snprintf(exename, sizeof(exename), "_export");
	}
	char filename[200];
	#ifndef NO_PIDLOG
	snprintf(filename, sizeof(filename), "%s%s_%04i.log", FileNames[id], exename, getpid());
	#else
	snprintf(filename, sizeof(filename), "%s%s.log", FileNames[id], exename);
	#endif
	fp[id] = fopen(filename, "a");
	if (!fp[id]) {
		std::cerr << "Failed to open log file: " << filename << std::endl;
		pLogStatus[id] |= 4; // set file state to error
		return false;
	}
	fputs("---------------------------------------------\n", fp[id]);
	write(id, "Starting Log: %s", filename);
	return true;
}

bool EQEmuLog::write(LogIDs id, const char *fmt, ...)
{
	if (!logFileValid) {
		return false;
	}
	if (id >= MaxLogID) {
		return false;
	}
		
	bool dofile = false;
	if (pLogStatus[id] & 1) {
		dofile = open(id);
	}
	if (!(dofile || pLogStatus[id] & 2)) {
		return false;
	}
	LockMutex lock(&MLog[id]);
	if (!logFileValid) {
		return false;    //check again for threading race reasons (to avoid two mutexes)
	}

	va_list argptr, tmpargptr;
	va_start(argptr, fmt);

	logger.Log(id, vStringFormat(fmt, argptr).c_str());

	if (logCallbackFmt[id]) {
		msgCallbackFmt p = logCallbackFmt[id];
		va_copy(tmpargptr, argptr);
		p(id, fmt, tmpargptr );
	}
	return true;
}

//write with Prefix and a VA_list
bool EQEmuLog::writePVA(LogIDs id, const char *prefix, const char *fmt, va_list argptr)
{
	if (!logFileValid) {
		return false;
	}
	if (id >= MaxLogID) {
		return false;
	}
	bool dofile = false;
	if (pLogStatus[id] & 1) {
		dofile = open(id);
	}
	if (!(dofile || pLogStatus[id] & 2)) {
		return false;
	}
	LockMutex lock(&MLog[id]);
	if (!logFileValid) {
		return false;    //check again for threading race reasons (to avoid two mutexes)
	}
	time_t aclock;
	struct tm *newtime;
	time( &aclock ); /* Get time in seconds */
	newtime = localtime( &aclock ); /* Convert time to struct */
	va_list tmpargptr;
	if (dofile) {
		#ifndef NO_PIDLOG
		fprintf(fp[id], "[%02d.%02d. - %02d:%02d:%02d] %s", newtime->tm_mon + 1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec, prefix);
		#else
		fprintf(fp[id], "%04i [%02d.%02d. - %02d:%02d:%02d] %s", getpid(), newtime->tm_mon + 1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec, prefix);
		#endif
		va_copy(tmpargptr, argptr);
		vfprintf( fp[id], fmt, tmpargptr );
	}
	if (logCallbackPva[id]) {
		msgCallbackPva p = logCallbackPva[id];
		va_copy(tmpargptr, argptr);
		p(id, prefix, fmt, tmpargptr );
	}
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "[%s] %s", LogNames[id], prefix);
			vfprintf( stderr, fmt, argptr );
		} 
		/* Console Output */
		else {
			

#ifdef _WINDOWS
			HANDLE  console_handle;
			console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

			CONSOLE_FONT_INFOEX info = { 0 };
			info.cbSize = sizeof(info);
			info.dwFontSize.Y = 12; // leave X as zero
			info.FontWeight = FW_NORMAL;
			wcscpy(info.FaceName, L"Lucida Console");
			SetCurrentConsoleFontEx(console_handle, NULL, &info);

			if (id == EQEmuLog::LogIDs::Status){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::Yellow); }
			if (id == EQEmuLog::LogIDs::Error){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightRed); }
			if (id == EQEmuLog::LogIDs::Normal){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightGreen); }
			if (id == EQEmuLog::LogIDs::Debug){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::Yellow); }
			if (id == EQEmuLog::LogIDs::Quest){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightCyan); }
			if (id == EQEmuLog::LogIDs::Commands){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightMagenta); }
			if (id == EQEmuLog::LogIDs::Crash){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightRed); }
#endif

			fprintf(stdout, "[%s] %s", LogNames[id], prefix);
			vfprintf(stdout, fmt, argptr);

#ifdef _WINDOWS
			/* Always set back to white*/
			SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::White);
#endif
		}
	}
	va_end(argptr);
	if (dofile) {
		fprintf(fp[id], "\n");
	}
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "\n");
		} else {
			fprintf(stdout, "\n");
		}
	}
	if (dofile) {
		fflush(fp[id]);
	}
	return true;
}

bool EQEmuLog::writebuf(LogIDs id, const char *buf, uint8 size, uint32 count)
{
	if (!logFileValid) {
		return false;
	}
	if (id >= MaxLogID) {
		return false;
	}
	bool dofile = false;
	if (pLogStatus[id] & 1) {
		dofile = open(id);
	}
	if (!(dofile || pLogStatus[id] & 2)) {
		return false;
	}
	LockMutex lock(&MLog[id]);
	if (!logFileValid) {
		return false;    //check again for threading race reasons (to avoid two mutexes)
	}
	time_t aclock;
	struct tm *newtime;
	time( &aclock ); /* Get time in seconds */
	newtime = localtime( &aclock ); /* Convert time to struct */
	if (dofile)
	#ifndef NO_PIDLOG
		fprintf(fp[id], "[%02d.%02d. - %02d:%02d:%02d] ", newtime->tm_mon + 1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
	#else
		fprintf(fp[id], "%04i [%02d.%02d. - %02d:%02d:%02d] ", getpid(), newtime->tm_mon + 1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
	#endif
	if (dofile) {
		fwrite(buf, size, count, fp[id]);
		fprintf(fp[id], "\n");
	}
	if (logCallbackBuf[id]) {
		msgCallbackBuf p = logCallbackBuf[id];
		p(id, buf, size, count);
	}
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "[%s] ", LogNames[id]);
			fwrite(buf, size, count, stderr);
			fprintf(stderr, "\n");
		} else {
#ifdef _WINDOWS
			HANDLE  console_handle;
			console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

			CONSOLE_FONT_INFOEX info = { 0 };
			info.cbSize = sizeof(info);
			info.dwFontSize.Y = 12; // leave X as zero
			info.FontWeight = FW_NORMAL;
			wcscpy(info.FaceName, L"Lucida Console");
			SetCurrentConsoleFontEx(console_handle, NULL, &info);

			if (id == EQEmuLog::LogIDs::Status){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::Yellow); }
			if (id == EQEmuLog::LogIDs::Error){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightRed); }
			if (id == EQEmuLog::LogIDs::Normal){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightGreen); }
			if (id == EQEmuLog::LogIDs::Debug){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightGreen); }
			if (id == EQEmuLog::LogIDs::Quest){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightCyan); }
			if (id == EQEmuLog::LogIDs::Commands){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightMagenta); }
			if (id == EQEmuLog::LogIDs::Crash){ SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::LightRed); }
#endif

			fprintf(stdout, "[%s] ", LogNames[id]);
			fwrite(buf, size, count, stdout);
			fprintf(stdout, "\n");

#ifdef _WINDOWS
			/* Always set back to white*/
			SetConsoleTextAttribute(console_handle, ConsoleColor::Colors::White);
#endif


		}
	}
	if (dofile) {
		fflush(fp[id]);
	}
	return true;
}

bool EQEmuLog::writeNTS(LogIDs id, bool dofile, const char *fmt, ...)
{
	va_list argptr, tmpargptr;
	va_start(argptr, fmt);
	if (dofile) {
		va_copy(tmpargptr, argptr);
		vfprintf( fp[id], fmt, tmpargptr );
	}
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			vfprintf( stderr, fmt, argptr );
		} else {
			vfprintf( stdout, fmt, argptr );
		}
	}
	va_end(argptr);
	return true;
};

bool EQEmuLog::Dump(LogIDs id, uint8* data, uint32 size, uint32 cols, uint32 skip)
{
	if (!logFileValid) {
		logger.LogDebug(EQEmuLogSys::Detail, "Error: Dump() from null pointer");
		return false;
	}
	if (size == 0) {
		return true;
	}
	if (!LogFile) {
		return false;
	}
	if (id >= MaxLogID) {
		return false;
	}
	bool dofile = false;
	if (pLogStatus[id] & 1) {
		dofile = open(id);
	}
	if (!(dofile || pLogStatus[id] & 2)) {
		return false;
	}
	LockMutex lock(&MLog[id]);
	if (!logFileValid) {
		return false;    //check again for threading race reasons (to avoid two mutexes)
	}
	write(id, "Dumping Packet: %i", size);
	// Output as HEX
	int beginningOfLineOffset = 0;
	uint32 indexInData;
	std::string asciiOutput;
	for (indexInData = skip; indexInData < size; indexInData++) {
		if ((indexInData - skip) % cols == 0) {
			if (indexInData != skip) {
				writeNTS(id, dofile, " | %s\n", asciiOutput.c_str());
			}
			writeNTS(id, dofile, "%4i: ", indexInData - skip);
			asciiOutput.clear();
			beginningOfLineOffset = 0;
		} else if ((indexInData - skip) % (cols / 2) == 0) {
			writeNTS(id, dofile, "- ");
		}
		writeNTS(id, dofile, "%02X ", (unsigned char)data[indexInData]);
		if (data[indexInData] >= 32 && data[indexInData] < 127) {
			// According to http://msdn.microsoft.com/en-us/library/vstudio/ee404875(v=vs.100).aspx
			// Visual Studio 2010 doesn't have std::to_string(int) but it does have the long long
			// version.
			asciiOutput.append(std::to_string((long long)data[indexInData]));
		} else {
			asciiOutput.append(".");
		}
	}
	uint32 k = ((indexInData - skip) - 1) % cols;
	if (k < 8) {
		writeNTS(id, dofile, "  ");
	}
	for (uint32 h = k + 1; h < cols; h++) {
		writeNTS(id, dofile, "   ");
	}
	writeNTS(id, dofile, " | %s\n", asciiOutput.c_str());
	if (dofile) {
		fflush(fp[id]);
	}
	return true;
}

void EQEmuLog::SetCallback(LogIDs id, msgCallbackFmt proc)
{
	if (!logFileValid) {
		return;
	}
	if (id >= MaxLogID) {
		return;
	}
	logCallbackFmt[id] = proc;
}

void EQEmuLog::SetCallback(LogIDs id, msgCallbackBuf proc)
{
	if (!logFileValid) {
		return;
	}
	if (id >= MaxLogID) {
		return;
	}
	logCallbackBuf[id] = proc;
}

void EQEmuLog::SetCallback(LogIDs id, msgCallbackPva proc)
{
	if (!logFileValid) {
		return;
	}
	if (id >= MaxLogID) {
		return;
	}
	logCallbackPva[id] = proc;
}

void EQEmuLog::SetAllCallbacks(msgCallbackFmt proc)
{
	if (!logFileValid) {
		return;
	}
	int r;
	for (r = Status; r < MaxLogID; r++) {
		SetCallback((LogIDs)r, proc);
	}
}

void EQEmuLog::SetAllCallbacks(msgCallbackBuf proc)
{
	if (!logFileValid) {
		return;
	}
	int r;
	for (r = Status; r < MaxLogID; r++) {
		SetCallback((LogIDs)r, proc);
	}
}

void EQEmuLog::SetAllCallbacks(msgCallbackPva proc)
{
	if (!logFileValid) {
		return;
	}
	int r;
	for (r = Status; r < MaxLogID; r++) {
		SetCallback((LogIDs)r, proc);
	}
}

