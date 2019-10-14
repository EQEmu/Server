CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(eqlogin_sources
	account_management.cpp
	client.cpp
	client_manager.cpp
	database.cpp
	encryption.cpp
	loginserver_command_handler.cpp
	loginserver_webserver.cpp
	main.cpp
	server_manager.cpp
	world_server.cpp
)

SET(eqlogin_headers
	account_management.h
	client.h
	client_manager.h
	database.h
	encryption.h
	loginserver_command_handler.h
	loginserver_webserver.h
	login_server.h
	login_structures.h
	options.h
	server_manager.h
	world_server.h
)

ADD_EXECUTABLE(loginserver ${eqlogin_sources} ${eqlogin_headers})

INSTALL(TARGETS loginserver RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

TARGET_LINK_LIBRARIES(loginserver ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
