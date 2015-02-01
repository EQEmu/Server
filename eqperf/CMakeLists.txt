CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(eqperf_sources
	eqp_profile_event.cpp
	eqp_profile_timer.cpp
	eqp_profiler.cpp
	eqp_profiler_node.cpp
)

SET(eqperf_headers
	eqp_profile_event.h
	eqp_profile_function.h
	eqp_profile_timer.h
	eqp_profiler.h
	eqp_profiler_node.h
)

ADD_LIBRARY(eqperf ${eqperf_sources} ${eqperf_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
