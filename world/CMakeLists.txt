CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(world_sources
	adventure.cpp
	adventure_manager.cpp
	client.cpp
	cliententry.cpp
	clientlist.cpp
	CMakeLists.txt
	console.cpp
	eql_config.cpp
	eqw.cpp
	eqw_http_handler.cpp
	eqw_parser.cpp
	http_request.cpp
	launcher_link.cpp
	launcher_list.cpp
	lfplist.cpp
	login_server.cpp
	login_server_list.cpp
	net.cpp
	perl_eql_config.cpp
	perl_eqw.cpp
	perl_http_request.cpp
	queryserv.cpp
	ucs.cpp
	wguild_mgr.cpp
	world_config.cpp
	worlddb.cpp
	zonelist.cpp
	zoneserver.cpp
)

SET(world_headers
	adventure.h
	adventure_manager.h
	adventure_template.h
	client.h
	cliententry.h
	clientlist.h
	CMakeLists.txt
	console.h
	eql_config.h
	eqw.h
	eqw_http_handler.h
	eqw_parser.h
	http_request.h
	launcher_link.h
	launcher_list.h
	lfplist.h
	login_server.h
	login_server_list.h
	net.h
	queryserv.h
	sof_char_create_data.h
	ucs.h
	wguild_mgr.h
	world_config.h
	worlddb.h
	world_tcp_connection.h
	zonelist.h
	zoneserver.h
)

ADD_EXECUTABLE(world ${world_sources} ${world_headers})

INSTALL(TARGETS world RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ADD_DEFINITIONS(-DWORLD)

TARGET_LINK_LIBRARIES(world common ${PERL_LIBRARY} debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(world PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(world "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(world "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(world "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(world "z")
	TARGET_LINK_LIBRARIES(world "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(world "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(world "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
