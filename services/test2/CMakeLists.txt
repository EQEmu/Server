CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(service_sources
	test2_service.cpp
)

SET(service_headers
	test2_service.h
)

ADD_EXECUTABLE(test2_service ${service_sources} ${service_headers})

INSTALL(TARGETS test2_service RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

TARGET_LINK_LIBRARIES(test2_service ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
