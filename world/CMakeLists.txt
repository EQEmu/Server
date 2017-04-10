CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(world_sources
	adventure.cpp
	adventure_manager.cpp
	client.cpp
	cliententry.cpp
	clientlist.cpp
	console.cpp
	eql_config.cpp
	launcher_link.cpp
	launcher_list.cpp
	lfplist.cpp
	login_server.cpp
	login_server_list.cpp
	net.cpp
	queryserv.cpp
	ucs.cpp
	web_interface.cpp
	web_interface_eqw.cpp
	wguild_mgr.cpp
	world_config.cpp
	world_console_connection.cpp
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
	console.h
	eql_config.h
	launcher_link.h
	launcher_list.h
	lfplist.h
	login_server.h
	login_server_list.h
	net.h
	queryserv.h
	sof_char_create_data.h
	ucs.h
	web_interface.h
	web_interface_eqw.h
	wguild_mgr.h
	world_config.h
	world_console_connection.h
	world_tcp_connection.h
	worlddb.h
	zonelist.h
	zoneserver.h
)

ADD_EXECUTABLE(world ${world_sources} ${world_headers})

INSTALL(TARGETS world RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ADD_DEFINITIONS(-DWORLD)

TARGET_LINK_LIBRARIES(world ${SERVER_LIBS})

IF(EQEMU_BUILD_PERL)
	TARGET_LINK_LIBRARIES(world ${PERL_LIBRARY})
ENDIF(EQEMU_BUILD_PERL)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
