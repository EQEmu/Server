CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(qserv_sources
    database.cpp
    lfguild.cpp
    queryserv.cpp
    queryservconfig.cpp
    worldserver.cpp
)

SET(qserv_headers
    database.h
    lfguild.h
    queryservconfig.h
    worldserver.h
)

ADD_EXECUTABLE(queryserv ${qserv_sources} ${qserv_headers})

INSTALL(TARGETS queryserv RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ADD_DEFINITIONS(-DQSERV)

TARGET_LINK_LIBRARIES(queryserv ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
