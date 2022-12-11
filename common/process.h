#ifndef EQEMU_PROCESS_H
#define EQEMU_PROCESS_H

#include "global_define.h"
#include "eqemu_logsys.h"
#include "crash.h"
#include "strings.h"

#include <cstdio>

#if WINDOWS
#define popen _popen
#endif

class Process {
public:
	static std::string execute(const std::string &cmd);
};


#endif //EQEMU_PROCESS_H
