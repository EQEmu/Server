#ifndef EQEMU_PROCESS_H
#define EQEMU_PROCESS_H

class Process {
public:
	static std::string execute(const std::string &cmd, bool return_result = true);
};


#endif //EQEMU_PROCESS_H
