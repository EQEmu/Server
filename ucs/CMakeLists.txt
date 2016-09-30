CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

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

TARGET_LINK_LIBRARIES(ucs common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY} libuv fmt)

IF(WIN32)
	TARGET_LINK_LIBRARIES(ucs "ws2_32" "psapi" "iphlpapi" "userenv")
ENDIF(WIN32)

IF(UNIX)
	TARGET_LINK_LIBRARIES(ucs "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(ucs "z")
	TARGET_LINK_LIBRARIES(ucs "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(ucs "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(ucs "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
