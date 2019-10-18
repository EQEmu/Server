CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(eqlaunch_sources
	eqlaunch.cpp
	worldserver.cpp
	zone_launch.cpp
)

SET(eqlaunch_headers
	worldserver.h
	zone_launch.h
)

ADD_EXECUTABLE(eqlaunch ${eqlaunch_sources} ${eqlaunch_headers})

INSTALL(TARGETS eqlaunch RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

TARGET_LINK_LIBRARIES(eqlaunch ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
