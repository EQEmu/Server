CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(eqlogin_sources
	client.cpp
	client_manager.cpp
	config.cpp
	database_mysql.cpp
	database_postgresql.cpp
	encryption.cpp
	main.cpp
	server_manager.cpp
	world_server.cpp
)

SET(eqlogin_headers
	client.h
	client_manager.h
	config.h
	database.h
	database_mysql.h
	database_postgresql.h
	encryption.h
	login_server.h
	login_structures.h
	options.h
	server_manager.h
	world_server.h
)

FIND_PACKAGE(OpenSSL REQUIRED)

INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})

ADD_EXECUTABLE(loginserver ${eqlogin_sources} ${eqlogin_headers})

INSTALL(TARGETS loginserver RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

TARGET_LINK_LIBRARIES(loginserver ${SERVER_LIBS} ${OPENSSL_LIBRARIES})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
