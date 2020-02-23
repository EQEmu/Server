CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(ucs_sources
	chatchannel.cpp
	clientlist.cpp
	database.cpp
	ucs.cpp
	ucsconfig.cpp
	worldserver.cpp
)

SET(ucs_headers
	chatchannel.h
	clientlist.h
	database.h
	ucsconfig.h
	worldserver.h
)

ADD_EXECUTABLE(ucs ${ucs_sources} ${ucs_headers})

INSTALL(TARGETS ucs RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ADD_DEFINITIONS(-DUCS)

TARGET_LINK_LIBRARIES(ucs ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
