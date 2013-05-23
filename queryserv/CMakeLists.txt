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

ADD_DEFINITIONS(-DQSERV)

TARGET_LINK_LIBRARIES(queryserv Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(queryserv PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(queryserv "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(queryserv "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	IF(NOT FREEBSD)
		TARGET_LINK_LIBRARIES(queryserv "dl")
	ENDIF(NOT FREEBSD)
	TARGET_LINK_LIBRARIES(queryserv "z")
	TARGET_LINK_LIBRARIES(queryserv "m")
	TARGET_LINK_LIBRARIES(queryserv "rt")
	TARGET_LINK_LIBRARIES(queryserv "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ../Bin)
