CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(web_interface_sources
	web_interface.cpp
	worldserver.cpp
)

SET(web_interface_headers
	worldserver.h
)

ADD_EXECUTABLE(web_interface ${web_interface_sources} ${web_interface_headers})

INSTALL(TARGETS web_interface RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ADD_DEFINITIONS(-DWEB_INTERFACE)

TARGET_LINK_LIBRARIES(web_interface common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(web_interface PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(web_interface "Ws2_32.lib")
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
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
