#ifndef EQEMU_CALLBACK_MANAGER_H
#define EQEMU_CALLBACK_MANAGER_H

#include <string>
#include <functional>

typedef std::function<void(void*)> eqemu_callback;

void RegisterEQCallback(std::string name, eqemu_callback func);
eqemu_callback GetEQCallback(std::string name);

#endif
