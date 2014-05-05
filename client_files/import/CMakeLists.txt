CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(import_sources
	main.cpp
)

SET(import_headers
)

ADD_EXECUTABLE(import_client_files ${import_sources} ${import_headers})

INSTALL(TARGETS import_client_files RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(import_client_files Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(import_client_files PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(import_client_files "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(import_client_files "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(import_client_files "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(import_client_files "z")
	TARGET_LINK_LIBRARIES(import_client_files "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(import_client_files "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(import_client_files "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
