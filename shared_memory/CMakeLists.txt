CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(shared_memory_sources
	base_data.cpp
	items.cpp
	loot.cpp
	main.cpp
	npc_faction.cpp
	spells.cpp
	skill_caps.cpp
)

SET(shared_memory_headers
	base_data.h
	items.h
	loot.h
	npc_faction.h
	spells.h
	skill_caps.h
)

ADD_EXECUTABLE(shared_memory ${shared_memory_sources} ${shared_memory_headers})

INSTALL(TARGETS shared_memory RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(shared_memory Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(shared_memory PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(shared_memory "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(shared_memory "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(shared_memory "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(shared_memory "z")
	TARGET_LINK_LIBRARIES(shared_memory "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(shared_memory "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(shared_memory "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
