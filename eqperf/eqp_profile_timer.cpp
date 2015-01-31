#include "eqp_profile_timer.h"

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <time.h>
#endif

uint64_t EQP::GetCurrentTimer()
{
#ifdef _MSC_VER
	LARGE_INTEGER qpt_i;
	QueryPerformanceCounter(&qpt_i);
	return qpt_i.QuadPart;
#else
	timespec tp;
	if(clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
		uint64_t res = tp.tv_sec * 1000000000;
		res += tp.tv_nsec;
		return res;
	}
	return 0;
#endif
}
