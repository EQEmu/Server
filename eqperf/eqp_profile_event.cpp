#include "eqp_profile_event.h"
#include "eqp_profile_timer.h"
#include "eqp_profiler.h"

EQP::CPU::ST::Event::Event(const char *function_name) {
	function_name_ = function_name;
	name_ = nullptr;
	start_ = GetCurrentTimer();
	
	identifier_ = EQP::CPU::ST::GetProfiler().EventStarted(function_name_, name_);
}

EQP::CPU::ST::Event::Event(const char *function_name, const char *name) {
	function_name_ = function_name;
	name_ = name;
	start_ = GetCurrentTimer();
	
	identifier_ = EQP::CPU::ST::GetProfiler().EventStarted(function_name_, name_);
}

EQP::CPU::ST::Event::~Event() {
	uint64_t end = GetCurrentTimer();
	
	EQP::CPU::ST::GetProfiler().EventFinished(end - start_, identifier_);
}

EQP::CPU::MT::Event::Event(const char *function_name) {
	function_name_ = function_name;
	name_ = nullptr;
	start_ = GetCurrentTimer();

	identifier_ = EQP::CPU::MT::GetProfiler().EventStarted(function_name_, name_);
}

EQP::CPU::MT::Event::Event(const char *function_name, const char *name) {
	function_name_ = function_name;
	name_ = name;
	start_ = GetCurrentTimer();

	identifier_ = EQP::CPU::MT::GetProfiler().EventStarted(function_name_, name_);
}

EQP::CPU::MT::Event::~Event() {
	uint64_t end = GetCurrentTimer();

	EQP::CPU::MT::GetProfiler().EventFinished(end - start_, identifier_);
}
