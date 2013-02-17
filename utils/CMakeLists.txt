CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(UNIX)
    ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

IF(UNIX AND EQEMU_BUILD_CLEANIPC)
    SET(cleanipc_sources
        cleanipc.cpp
    )

    SET(cleanipc_headers
    )
    
    ADD_EXECUTABLE(cleanipc ${cleanipc_sources} ${cleanipc_headers})
    TARGET_LINK_LIBRARIES(cleanipc "rt")
    SET(EXECUTABLE_OUTPUT_PATH ../Bin)
ENDIF(UNIX AND EQEMU_BUILD_CLEANIPC)

IF(EQEMU_BUILD_AZONE)
    ADD_SUBDIRECTORY(azone2)
ENDIF(EQEMU_BUILD_AZONE)