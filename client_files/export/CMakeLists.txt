CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(export_sources
	main.cpp
)

SET(export_headers
)

ADD_EXECUTABLE(export_client_files ${export_sources} ${export_headers})

INSTALL(TARGETS export_client_files RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(export_client_files common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(export_client_files PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(export_client_files "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(export_client_files "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(export_client_files "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(export_client_files "z")
	TARGET_LINK_LIBRARIES(export_client_files "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(export_client_files "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(export_client_files "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
