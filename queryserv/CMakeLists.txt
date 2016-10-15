CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(qserv_sources
	database.cpp
	lfguild.cpp
	queryserv.cpp
	queryservconfig.cpp
	worldserver.cpp
)

SET(qserv_headers
	database.h
	lfguild.h
	queryservconfig.h
	worldserver.h
)

ADD_EXECUTABLE(queryserv ${qserv_sources} ${qserv_headers})

INSTALL(TARGETS queryserv RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ADD_DEFINITIONS(-DQSERV)

TARGET_LINK_LIBRARIES(queryserv common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY} libuv fmt)

IF(WIN32)
	TARGET_LINK_LIBRARIES(queryserv "ws2_32" "psapi" "iphlpapi" "userenv")
ENDIF(WIN32)

IF(UNIX)
	TARGET_LINK_LIBRARIES(queryserv "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(queryserv "z")
	TARGET_LINK_LIBRARIES(queryserv "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(queryserv "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(queryserv "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
