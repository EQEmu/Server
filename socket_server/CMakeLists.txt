CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(socket_server_sources
	database.cpp
	socket_server.cpp
	socket_server_config.cpp
	worldserver.cpp
)

SET(socket_server_headers
	database.h
	socket_server_config.h
	worldserver.h
)

ADD_EXECUTABLE(socket_server ${socket_server_sources} ${socket_server_headers})

INSTALL(TARGETS socket_server RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ADD_DEFINITIONS(-DQSERV)

TARGET_LINK_LIBRARIES(socket_server Common ${Boost_LIBRARIES} debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(socket_server PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(socket_server "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(socket_server "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(socket_server "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(socket_server "z")
	TARGET_LINK_LIBRARIES(socket_server "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(socket_server "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(socket_server "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
