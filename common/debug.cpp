#include "debug.h"

#include <iostream>
#include <time.h>
#include <string.h>
#ifdef _WINDOWS
	#include <process.h>

	#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#else
	#include <sys/types.h>
	#include <unistd.h>
	#include <stdarg.h>
#endif
#include "../common/MiscFunctions.h"
#include "../common/platform.h"

#ifndef va_copy
	#define va_copy(d,s) ((d) = (s))
#endif

static volatile bool logFileValid = false;
static EQEMuLog realLogFile;
EQEMuLog *LogFile = &realLogFile;

static const char* FileNames[EQEMuLog::MaxLogID] = { "logs/eqemu", "logs/eqemu", "logs/eqemu_error", "logs/eqemu_debug", "logs/eqemu_quest", "logs/eqemu_commands", "logs/crash" };
static const char* LogNames[EQEMuLog::MaxLogID] = { "Status", "Normal", "Error", "Debug", "Quest", "Command", "Crash" };

EQEMuLog::EQEMuLog() {
//	MOpen = new Mutex;
//	MLog = new Mutex*[MaxLogID];
//	fp = new FILE*[MaxLogID];
//	pLogStatus = new uint8[MaxLogID];
	for (int i=0; i<MaxLogID; i++) {
		fp[i] = 0;
//		MLog[i] = new Mutex;
#if EQDEBUG >= 2
		pLogStatus[i] = 1 | 2;
#else
		pLogStatus[i] = 0;
#endif
		logCallbackFmt[i] = nullptr;
		logCallbackBuf[i] = nullptr;
		logCallbackPva[i] = nullptr;
	}
// TODO: Make this read from an ini or something, everyone has different opinions on what it should be
#if EQDEBUG < 2
	pLogStatus[Status] = 2;
	pLogStatus[Error] = 2;
	pLogStatus[Quest] = 2;
	pLogStatus[Commands] = 1;
#endif
	logFileValid = true;
}

EQEMuLog::~EQEMuLog() {
	logFileValid = false;
	for (int i=0; i<MaxLogID; i++) {
		LockMutex lock(&MLog[i]);	//to prevent termination race
		if (fp[i])
			fclose(fp[i]);
	}
//	safe_delete_array(fp);
//	safe_delete_array(MLog);
//	safe_delete_array(pLogStatus);
//	safe_delete(MOpen);
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
	int j = 0; char* ascii = new char[cols+1]; memset(ascii, 0, cols+1);
	uint32 i;
	for(i=skip; i<size; i++) {
		if ((i-skip)%cols==0) {
			if (i != skip)
				writeNTS(id, dofile, " | %s\n", ascii);
			writeNTS(id, dofile, "%4i: ", i-skip);
			memset(ascii, 0, cols+1);
			j = 0;
		}
		else if ((i-skip)%(cols/2) == 0) {
			writeNTS(id, dofile, "- ");
		}
		writeNTS(id, dofile, "%02X ", (unsigned char)data[i]);

		if (data[i] >= 32 && data[i] < 127)
			ascii[j++] = data[i];
		else
			ascii[j++] = '.';
	}
	uint32 k = ((i-skip)-1)%cols;
	if (k < 8)
		writeNTS(id, dofile, "  ");
	for (uint32 h = k+1; h < cols; h++) {
		writeNTS(id, dofile, "   ");
	}
	writeNTS(id, dofile, " | %s\n", ascii);
	if (dofile)
		fflush(fp[id]);
	safe_delete_array(ascii);
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

