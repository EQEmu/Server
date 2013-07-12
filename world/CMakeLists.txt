CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(world_sources
	Adventure.cpp
	AdventureManager.cpp
	client.cpp
	cliententry.cpp
	clientlist.cpp
	CMakeLists.txt
	console.cpp
	EQLConfig.cpp
	EQW.cpp
	EQWHTTPHandler.cpp
	EQWParser.cpp
	HTTPRequest.cpp
	LauncherLink.cpp
	LauncherList.cpp
	lfplist.cpp
	LoginServer.cpp
	LoginServerList.cpp
	net.cpp
	perl_EQLConfig.cpp
	perl_EQW.cpp
	perl_HTTPRequest.cpp
	queryserv.cpp
	ucs.cpp
	wguild_mgr.cpp
	world_logsys.cpp
	WorldConfig.cpp
	worlddb.cpp
	zonelist.cpp
	zoneserver.cpp
)

SET(world_headers
	Adventure.h
	AdventureManager.h
	AdventureTemplate.h
	client.h
	cliententry.h
	clientlist.h
	CMakeLists.txt
	console.h
	EQLConfig.h
	EQW.h
	EQWHTTPHandler.h
	EQWParser.h
	HTTPRequest.h
	LauncherLink.h
	LauncherList.h
	lfplist.h
	LoginServer.h
	LoginServerList.h
	net.h
	queryserv.h
	SoFCharCreateData.h
	ucs.h
	wguild_mgr.h
	WorldConfig.h
	worlddb.h
	WorldTCPConnection.h
	zonelist.h
	zoneserver.h
)

ADD_EXECUTABLE(world ${world_sources} ${world_headers})

INSTALL(TARGETS world RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ADD_DEFINITIONS(-DWORLD)

TARGET_LINK_LIBRARIES(world Common ${PERL_LIBRARY} debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(world PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(world "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(world "WS2_32")
ENDIF(MINGW)

IF(UNIX)

	IF(NOT FREEBSD)
		TARGET_LINK_LIBRARIES(world "dl")
	ENDIF(NOT FREEBSD)
	TARGET_LINK_LIBRARIES(world "z")
	TARGET_LINK_LIBRARIES(world "m")
	TARGET_LINK_LIBRARIES(world "rt")
	TARGET_LINK_LIBRARIES(world "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
