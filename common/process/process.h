#ifndef EQEMU_PROCESS_H
#define EQEMU_PROCESS_H

#include "../strings.h"

#include <cstdio>

#if _WIN32
#define popen _popen
#define pclose _pclose
#endif

class Process {
public:
	static std::string execute(const std::string &cmd);
};


#endif //EQEMU_PROCESS_H
