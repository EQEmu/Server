#include <map>
#include "callback_manager.h"

std::map<std::string, eqemu_callback> callback_functions;

void RegisterEQCallback(std::string name, eqemu_callback func) {
	callback_functions[name] = func;
}

eqemu_callback GetEQCallback(std::string name) {
	auto iter = callback_functions.find(name);
	if(iter == callback_functions.end()) {
		return nullptr;
	}
	
	return iter->second;
}
