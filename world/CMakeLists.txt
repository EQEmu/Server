CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(world_sources
	adventure.cpp
	adventure_manager.cpp
	client.cpp
	cliententry.cpp
	clientlist.cpp
	console.cpp
	dynamic_zone.cpp
	dynamic_zone_manager.cpp
	eql_config.cpp
	eqemu_api_world_data_service.cpp
	expedition_database.cpp
	expedition_message.cpp
	launcher_link.cpp
	launcher_list.cpp
	lfplist.cpp
	login_server.cpp
	login_server_list.cpp
	main.cpp
	queryserv.cpp
	shared_task_manager.cpp
	shared_task_world_messaging.cpp
	ucs.cpp
	web_interface.cpp
	web_interface_eqw.cpp
	wguild_mgr.cpp
	world_event_scheduler.cpp
	world_config.cpp
	world_console_connection.cpp
	world_server_command_handler.cpp
	worlddb.cpp
	world_store.cpp
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
	dynamic_zone.h
	dynamic_zone_manager.h
	eql_config.h
	eqemu_api_world_data_service.h
	expedition_database.h
	expedition_message.h
	launcher_link.h
	launcher_list.h
	lfplist.h
	login_server.h
	login_server_list.h
	queryserv.h
	shared_task_manager.h
	shared_task_world_messaging.h
	sof_char_create_data.h
	ucs.h
	web_interface.h
	web_interface_eqw.h
	wguild_mgr.h
	world_config.h
	world_console_connection.h
	world_tcp_connection.h
	world_server_command_handler.h
	worlddb.h
	world_event_scheduler.h
	world_store.h
	zonelist.h
	zoneserver.h
)

ADD_EXECUTABLE(world ${world_sources} ${world_headers})

INSTALL(TARGETS world RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ADD_DEFINITIONS(-DWORLD)

TARGET_LINK_LIBRARIES(world ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
