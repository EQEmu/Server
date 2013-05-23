CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(eqlaunch_sources
	eqlaunch.cpp
	worldserver.cpp
	ZoneLaunch.cpp
)

SET(eqlaunch_headers
	worldserver.h
	ZoneLaunch.h
)

ADD_EXECUTABLE(eqlaunch ${eqlaunch_sources} ${eqlaunch_headers})

TARGET_LINK_LIBRARIES(eqlaunch Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(eqlaunch PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(eqlaunch "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(eqlaunch "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	IF(NOT FREEBSD)
		TARGET_LINK_LIBRARIES(eqlaunch "dl")
	ENDIF(NOT FREEBSD)
	TARGET_LINK_LIBRARIES(eqlaunch "z")
	TARGET_LINK_LIBRARIES(eqlaunch "m")
	TARGET_LINK_LIBRARIES(eqlaunch "rt")
	TARGET_LINK_LIBRARIES(eqlaunch "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ../Bin)
