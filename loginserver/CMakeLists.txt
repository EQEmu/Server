CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(eqlogin_sources
	client.cpp
	client_manager.cpp
	config.cpp
	database_mysql.cpp
	database_postgresql.cpp
	main.cpp
	server_manager.cpp
	world_server.cpp
)

IF(MSVC OR MINGW)
	ADD_DEFINITIONS(-DNOMINMAX)
	SET(eqlogin_sources ${eqlogin_sources} encryption.cpp)
ENDIF(MSVC OR MINGW)

SET(eqlogin_headers
	client.h
	client_manager.h
	config.h
	database.h
	database_mysql.h
	database_postgresql.h
	encryption.h
	eq_crypto_api.h
	login_server.h
	login_structures.h
	options.h
	server_manager.h
	world_server.h
)

IF(UNIX)
	SET(EQEMU_UNIX_ENC_LIBRARY_LOC "${CMAKE_SOURCE_DIR}/dependencies" CACHE PATH "Location of EQEmuAuthCrypto and cryptopp")
	LINK_DIRECTORIES(${EQEMU_UNIX_ENC_LIBRARY_LOC})
ENDIF(UNIX)

ADD_EXECUTABLE(loginserver ${eqlogin_sources} ${eqlogin_headers})

INSTALL(TARGETS loginserver RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(loginserver common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(loginserver PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(loginserver "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(loginserver "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(loginserver "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(loginserver "z")
	TARGET_LINK_LIBRARIES(loginserver "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(loginserver "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(loginserver "pthread")
	TARGET_LINK_LIBRARIES(loginserver "EQEmuAuthCrypto")
	TARGET_LINK_LIBRARIES(loginserver "cryptopp")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
