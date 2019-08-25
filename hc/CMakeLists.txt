CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(hc_sources
	eq.cpp
	main.cpp
	login.cpp
	world.cpp
)

SET(hc_headers
	eq.h
	login.h
	world.h
)

ADD_EXECUTABLE(hc ${hc_sources} ${hc_headers})

INSTALL(TARGETS hc RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

TARGET_LINK_LIBRARIES(hc ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
