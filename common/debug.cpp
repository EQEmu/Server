#include <iostream>
#include <string>
#include <cstdarg>
#include <time.h>

#ifdef _WINDOWS
	#include <process.h>

	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp

#else
	
	#include <sys/types.h>
	#include <unistd.h>

#endif

#include "debug.h"
#include "string_util.h"
#include "misc_functions.h"
#include "platform.h"

#ifndef va_copy
	#define va_copy(d,s) ((d) = (s))
#endif

static volatile bool logFileValid = false;
static EQEMuLog realLogFile;
EQEMuLog *LogFile = &realLogFile;

static const char* FileNames[EQEMuLog::MaxLogID] = { "logs/eqemu", "logs/eqemu", "logs/eqemu_error", "logs/eqemu_debug", "logs/eqemu_quest", "logs/eqemu_commands", "logs/crash" };
static const char* LogNames[EQEMuLog::MaxLogID] = { "Status", "Normal", "Error", "Debug", "Quest", "Command", "Crash" };

EQEMuLog::EQEMuLog() {
	for (int i=0; i<MaxLogID; i++) {
		fp[i] = 0;
		logCallbackFmt[i] = nullptr;
		logCallbackBuf[i] = nullptr;
		logCallbackPva[i] = nullptr;
	}
	
	pLogStatus[Status] = LOG_LEVEL_STATUS;
	pLogStatus[Normal] = LOG_LEVEL_NORMAL;
	pLogStatus[Error] = LOG_LEVEL_ERROR;
	pLogStatus[Debug] = LOG_LEVEL_DEBUG;
	pLogStatus[Quest] = LOG_LEVEL_QUEST;
	pLogStatus[Commands] = LOG_LEVEL_COMMANDS;
	pLogStatus[Crash] = LOG_LEVEL_CRASH;
	logFileValid = true;
}

EQEMuLog::~EQEMuLog() {
	logFileValid = false;
	for (int i=0; i<MaxLogID; i++) {
		LockMutex lock(&MLog[i]);	//to prevent termination race
		if (fp[i])
			fclose(fp[i]);
	}
}

bool EQEMuLog::open(LogIDs id) {
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
	if(platform == ExePlatformWorld) {
		snprintf(exename, sizeof(exename), "_world");
	} else if(platform == ExePlatformZone) {
		snprintf(exename, sizeof(exename), "_zone");
	} else if(platform == ExePlatformLaunch) {
		snprintf(exename, sizeof(exename), "_launch");
	} else if(platform == ExePlatformUCS) {
		snprintf(exename, sizeof(exename), "_ucs");
	} else if(platform == ExePlatformQueryServ) {
		snprintf(exename, sizeof(exename), "_queryserv");
	} else if(platform == ExePlatformSharedMemory) {
		snprintf(exename, sizeof(exename), "_shared_memory");
	} else if(platform == ExePlatformClientImport) {
		snprintf(exename, sizeof(exename), "_import");
	} else if(platform == ExePlatformClientExport) {
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
	fputs("---------------------------------------------\n",fp[id]);
	write(id, "Starting Log: %s", filename);
	return true;
}

bool EQEMuLog::write(LogIDs id, const char *fmt, ...) {
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
	if (!(dofile || pLogStatus[id] & 2))
		return false;
	LockMutex lock(&MLog[id]);
	if (!logFileValid)
		return false;	//check again for threading race reasons (to avoid two mutexes)

	time_t aclock;
	struct tm *newtime;

	time( &aclock ); /* Get time in seconds */
	newtime = localtime( &aclock ); /* Convert time to struct */

	if (dofile)
#ifndef NO_PIDLOG
		fprintf(fp[id], "[%02d.%02d. - %02d:%02d:%02d] ", newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
#else
		fprintf(fp[id], "%04i [%02d.%02d. - %02d:%02d:%02d] ", getpid(), newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
#endif

	va_list argptr, tmpargptr;
	va_start(argptr, fmt);
	if (dofile) {
		va_copy(tmpargptr, argptr);
		vfprintf( fp[id], fmt, tmpargptr );
	}
	if(logCallbackFmt[id]) {
		msgCallbackFmt p = logCallbackFmt[id];
		va_copy(tmpargptr, argptr);
		p(id, fmt, tmpargptr );
	}
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "[%s] ", LogNames[id]);
			vfprintf( stderr, fmt, argptr );
		}
		else {
			fprintf(stdout, "[%s] ", LogNames[id]);
			vfprintf( stdout, fmt, argptr );
		}
	}
	va_end(argptr);
	if (dofile)
		fprintf(fp[id], "\n");
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "\n");
			fflush(stderr);
		} else {
			fprintf(stdout, "\n");
			fflush(stdout);
		}
	}
	if(dofile)
		fflush(fp[id]);
	return true;
}

//write with Prefix and a VA_list
bool EQEMuLog::writePVA(LogIDs id, const char *prefix, const char *fmt, va_list argptr) {
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
	if (!logFileValid)
		return false;	//check again for threading race reasons (to avoid two mutexes)

	time_t aclock;
	struct tm *newtime;

	time( &aclock ); /* Get time in seconds */
	newtime = localtime( &aclock ); /* Convert time to struct */

	va_list tmpargptr;

	if (dofile) {
#ifndef NO_PIDLOG
		fprintf(fp[id], "[%02d.%02d. - %02d:%02d:%02d] %s", newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec, prefix);
#else
		fprintf(fp[id], "%04i [%02d.%02d. - %02d:%02d:%02d] %s", getpid(), newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec, prefix);
#endif
		va_copy(tmpargptr, argptr);
		vfprintf( fp[id], fmt, tmpargptr );
	}
	if(logCallbackPva[id]) {
		msgCallbackPva p = logCallbackPva[id];
		va_copy(tmpargptr, argptr);
		p(id, prefix, fmt, tmpargptr );
	}
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "[%s] %s", LogNames[id], prefix);
			vfprintf( stderr, fmt, argptr );
		}
		else {
			fprintf(stdout, "[%s] %s", LogNames[id], prefix);
			vfprintf( stdout, fmt, argptr );
		}
	}
	va_end(argptr);
	if (dofile)
		fprintf(fp[id], "\n");
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8)
			fprintf(stderr, "\n");
		else
			fprintf(stdout, "\n");
	}
	if(dofile)
		fflush(fp[id]);
	return true;
}

bool EQEMuLog::writebuf(LogIDs id, const char *buf, uint8 size, uint32 count) {
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
	if (!(dofile || pLogStatus[id] & 2))
		return false;
	LockMutex lock(&MLog[id]);
	if (!logFileValid)
		return false;	//check again for threading race reasons (to avoid two mutexes)

	time_t aclock;
	struct tm *newtime;

	time( &aclock ); /* Get time in seconds */
	newtime = localtime( &aclock ); /* Convert time to struct */

	if (dofile)
#ifndef NO_PIDLOG
		fprintf(fp[id], "[%02d.%02d. - %02d:%02d:%02d] ", newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
#else
		fprintf(fp[id], "%04i [%02d.%02d. - %02d:%02d:%02d] ", getpid(), newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
#endif

	if (dofile) {
		fwrite(buf, size, count, fp[id]);
		fprintf(fp[id], "\n");
	}
	if(logCallbackBuf[id]) {
		msgCallbackBuf p = logCallbackBuf[id];
		p(id, buf, size, count);
	}
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "[%s] ", LogNames[id]);
			fwrite(buf, size, count, stderr);
			fprintf(stderr, "\n");
		} else {
			fprintf(stdout, "[%s] ", LogNames[id]);
			fwrite(buf, size, count, stdout);
			fprintf(stdout, "\n");
		}
	}
	if(dofile)
		fflush(fp[id]);
	return true;
}

bool EQEMuLog::writeNTS(LogIDs id, bool dofile, const char *fmt, ...) {
	va_list argptr, tmpargptr;
	va_start(argptr, fmt);
	if (dofile) {
		va_copy(tmpargptr, argptr);
		vfprintf( fp[id], fmt, tmpargptr );
	}
	if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8)
			vfprintf( stderr, fmt, argptr );
		else
			vfprintf( stdout, fmt, argptr );
	}
	va_end(argptr);
	return true;
};

bool EQEMuLog::Dump(LogIDs id, uint8* data, uint32 size, uint32 cols, uint32 skip) {
	if (!logFileValid) {
#if EQDEBUG >= 10
	std::cerr << "Error: Dump() from null pointer" << std::endl;
#endif
		return false;
	}
	if (size == 0)
		return true;
	if (!LogFile)
		return false;
	if (id >= MaxLogID)
		return false;
	bool dofile = false;
	if (pLogStatus[id] & 1) {
		dofile = open(id);
	}
	if (!(dofile || pLogStatus[id] & 2))
		return false;
	LockMutex lock(&MLog[id]);
	if (!logFileValid)
		return false;	//check again for threading race reasons (to avoid two mutexes)

	write(id, "Dumping Packet: %i", size);
	// Output as HEX
	
	int beginningOfLineOffset = 0; 
	uint32 indexInData;
	std::string asciiOutput;

	for(indexInData=skip; indexInData<size; indexInData++) {
		if ((indexInData-skip)%cols==0) {
			if (indexInData != skip)
				writeNTS(id, dofile, " | %s\n", asciiOutput.c_str());
			writeNTS(id, dofile, "%4i: ", indexInData-skip);
			asciiOutput.clear();
			beginningOfLineOffset = 0;
		}
		else if ((indexInData-skip)%(cols/2) == 0) {
			writeNTS(id, dofile, "- ");
		}
		writeNTS(id, dofile, "%02X ", (unsigned char)data[indexInData]);

		if (data[indexInData] >= 32 && data[indexInData] < 127)
		{
			// According to http://msdn.microsoft.com/en-us/library/vstudio/ee404875(v=vs.100).aspx
			// Visual Studio 2010 doesn't have std::to_string(int) but it does have the long long 
			// version.
			asciiOutput.append(std::to_string((long long)data[indexInData]));
		}
		else
		{
			asciiOutput.append(".");
		}
	}
	uint32 k = ((indexInData-skip)-1)%cols;
	if (k < 8)
		writeNTS(id, dofile, "  ");
	for (uint32 h = k+1; h < cols; h++) {
		writeNTS(id, dofile, "   ");
	}
	writeNTS(id, dofile, " | %s\n", asciiOutput.c_str());
	if (dofile)
		fflush(fp[id]);
	return true;
}

void EQEMuLog::SetCallback(LogIDs id, msgCallbackFmt proc) {
	if (!logFileValid)
		return;
	if (id >= MaxLogID) {
		return;
	}
	logCallbackFmt[id] = proc;
}

void EQEMuLog::SetCallback(LogIDs id, msgCallbackBuf proc) {
	if (!logFileValid)
		return;
	if (id >= MaxLogID) {
		return;
	}
	logCallbackBuf[id] = proc;
}

void EQEMuLog::SetCallback(LogIDs id, msgCallbackPva proc) {
	if (!logFileValid)
		return;
	if (id >= MaxLogID) {
		return;
	}
	logCallbackPva[id] = proc;
}

void EQEMuLog::SetAllCallbacks(msgCallbackFmt proc) {
	if (!logFileValid)
		return;
	int r;
	for(r = Status; r < MaxLogID; r++) {
		SetCallback((LogIDs)r, proc);
	}
}

void EQEMuLog::SetAllCallbacks(msgCallbackBuf proc) {
	if (!logFileValid)
		return;
	int r;
	for(r = Status; r < MaxLogID; r++) {
		SetCallback((LogIDs)r, proc);
	}
}

void EQEMuLog::SetAllCallbacks(msgCallbackPva proc) {
	if (!logFileValid)
		return;
	int r;
	for(r = Status; r < MaxLogID; r++) {
		SetCallback((LogIDs)r, proc);
	}
}

