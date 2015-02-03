#pragma once

#ifdef EQPERF_ENABLED

#include <string>
#include "eqp_profile_event.h"
#include "eqp_profiler_node.h"

#define eqp_comb_fin(x, y) x##y
#define eqp_comb(x, y) eqp_comb_fin(x, y)
#ifndef EQP_MULTITHREAD
#define _eqp EQP::CPU::ST::Event eqp_comb(eq_perf_event_, __LINE__) (__PRETTY_FUNCTION__);
#define _eqpn(x) EQP::CPU::ST::Event eqp_comb(eq_perf_event_, __LINE__) (__PRETTY_FUNCTION__, x);
#define _eqp_clear() EQP::CPU::ST::GetProfiler().Clear()
#define _eqp_dump(strm, count) EQP::CPU::ST::GetProfiler().Dump(strm, count)
#else
#define _eqp EQP::CPU::MT::Event eqp_comb(eq_perf_event_, __LINE__) (__PRETTY_FUNCTION__);
#define _eqpn(x) EQP::CPU::MT::Event eqp_comb(eq_perf_event_, __LINE__) (__PRETTY_FUNCTION__, x);
#define _eqp_clear() EQP::CPU::MT::GetProfiler().Clear()
#define _eqp_dump(strm, count) EQP::CPU::MT::GetProfiler().Dump(strm, count)
#endif

namespace EQP
{
	namespace CPU
	{
		namespace ST
		{
			class EQP_EXPORT Profiler
			{
			typedef EQP::CPU::ProfilerNode Node;
			public:
				Profiler();
				~Profiler();

				std::string EventStarted(const char *func, const char *name);
				void EventFinished(uint64_t time, std::string ident);
				void Dump(std::ostream &stream, int num = 0);
				void Clear();
			private:
				Node *root_;
				Node *current_;
				std::string identifier_;
			};

			EQP_EXPORT Profiler &GetProfiler();
		}

		namespace MT
		{
			class EQP_EXPORT Profiler
			{
			typedef EQP::CPU::ProfilerNode Node;
			class ThreadInfo {
			public:
				ThreadInfo();
				~ThreadInfo();
				Node *root_;
				Node *current_;
			};
			public:
				Profiler();
				~Profiler();

				std::string EventStarted(const char *func, const char *name);
				void EventFinished(uint64_t time, std::string ident);
				void Dump(std::ostream &stream, int num = 0);
				void Clear();
			private:
				struct impl;
				impl *imp_;
			};

			EQP_EXPORT Profiler &GetProfiler();
		}
	} // CPU
} // EQP

#else

#define _eqp 
#define _eqpn(x) 
#define _eqp_clear() 
#define _eqp_dump(strm, count) 
#endif

