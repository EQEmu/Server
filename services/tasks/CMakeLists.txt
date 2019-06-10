CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(service_sources
	tasks_service.cpp
)

SET(service_headers
	tasks_service.h
)

ADD_EXECUTABLE(tasks_service ${service_sources} ${service_headers})

INSTALL(TARGETS tasks_service RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

TARGET_LINK_LIBRARIES(tasks_service ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
