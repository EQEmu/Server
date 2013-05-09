#ifndef EQWX_ERRORLOG__H
#define EQWX_ERRORLOG__H

#include <stdio.h>
#include <stdarg.h>

//Log options
enum{
	eqEmuLogConsole = 1,
	eqEmuLogFile = 2,
	eqEmuLogSQL = 4,
	eqEmuLogBoth = (eqEmuLogConsole | eqEmuLogFile),
};

//log file output
#define LOG_FILE_NAME "debug.txt"
#define SQL_LOG_FILE_NAME "sql_log.sql"

//max single log length
//same as a client:message() max len
#define MAX_LOG_LEN 4096

//much simplified logging function, similar to the one used on the server (Tho much simpler)
class EQEmuErrorLog {
public:

	EQEmuErrorLog();
	~EQEmuErrorLog();
	void Log(unsigned int mOutputType, const char *msg, ...);
private:
	FILE* mErrorLog;
	FILE* mErrorLogSQL;
};

#endif