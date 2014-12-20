CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(web_interface_sources
	method_handler.cpp
	remote_call.cpp
	web_interface.cpp
	worldserver.cpp
)

SET(web_interface_headers
	method_handler.h
	remote_call.h
	worldserver.h
)

ADD_EXECUTABLE(web_interface ${web_interface_sources} ${web_interface_headers})

INSTALL(TARGETS web_interface RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ADD_DEFINITIONS(-DWEB_INTERFACE)

IF(MSVC)
	TARGET_LINK_LIBRARIES(web_interface common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY} ${LIBWEBSOCKETS_LIBRARIES})
ELSE(MSVC)
	TARGET_LINK_LIBRARIES(web_interface common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY} ${LIBWEBSOCKETS_LIBRARIES_SHARED})
ENDIF(MSVC)

IF(MSVC)
	SET_TARGET_PROPERTIES(web_interface PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(web_interface "Ws2_32.lib")
	TARGET_LINK_LIBRARIES(web_interface rpcrt4)
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(web_interface "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(web_interface "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(web_interface "z")
	TARGET_LINK_LIBRARIES(web_interface "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(web_interface "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(web_interface "pthread")
	TARGET_LINK_LIBRARIES(web_interface "uuid")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
