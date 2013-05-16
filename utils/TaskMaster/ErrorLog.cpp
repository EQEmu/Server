#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include "ErrorLog.h"


EQEmuErrorLog::EQEmuErrorLog()
{
	mErrorLog = NULL;
	mErrorLog = fopen(LOG_FILE_NAME, "w");
	if(!mErrorLog) {
		Log(eqEmuLogConsole,"Opening of %s for writing failed, debug output will be tied to console only.", LOG_FILE_NAME);
	}
	else
		Log(eqEmuLogBoth, "EQEmuErrorLog Init Successful.");

	mErrorLogSQL = NULL;
	mErrorLogSQL = fopen(SQL_LOG_FILE_NAME, "a");
	if(!mErrorLog) {
		Log(eqEmuLogConsole, "SQL Log Init FAILED, %s could not be opened for writing", SQL_LOG_FILE_NAME);
	}
	else
	{
		Log(eqEmuLogConsole, "SQL Log Init Complete");
	}

}
EQEmuErrorLog::~EQEmuErrorLog()
{
	Log(eqEmuLogBoth, "EQEmuErrorLog Shutdown.");
	if(mErrorLog) {
		fclose(mErrorLog);
		mErrorLog = NULL;
	}
}

void EQEmuErrorLog::Log(unsigned int mOutputType, const char *msg, ...)
{
	va_list argptr;
	char *buffer = new char[MAX_LOG_LEN];

	va_start(argptr, msg);
	_vsnprintf(buffer, MAX_LOG_LEN, msg, argptr);
	va_end(argptr);

    time_t mClock;
    struct tm *mTime;
    time(&mClock);
    mTime = localtime(&mClock);

	if(mOutputType & eqEmuLogConsole){
		printf("[Debug] [%02d.%02d.%02d - %02d:%02d:%02d] %s\n", mTime->tm_mon+1, mTime->tm_mday, mTime->tm_year%100, mTime->tm_hour, mTime->tm_min, mTime->tm_sec, buffer);
	}

	if(mOutputType & eqEmuLogFile){
		if(mErrorLog){
			fprintf(mErrorLog, "[%02d.%02d.%02d - %02d:%02d:%02d] %s\n", mTime->tm_mon+1, mTime->tm_mday, mTime->tm_year%100, mTime->tm_hour, mTime->tm_min, mTime->tm_sec, buffer);
		}
	}

	if(mOutputType & eqEmuLogSQL)
	{
		if(mErrorLogSQL){
			fprintf(mErrorLogSQL, "# [%02d.%02d.%02d - %02d:%02d:%02d]\n", mTime->tm_mon+1, mTime->tm_mday, mTime->tm_year%100, mTime->tm_hour, mTime->tm_min, mTime->tm_sec);
			fprintf(mErrorLogSQL, "%s;\n\n", buffer);
		}
	}

	if(buffer)
		delete[] buffer;
}