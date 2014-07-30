CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(lb_sources
    src/class.cpp
	src/class_info.cpp
	src/class_registry.cpp
	src/class_rep.cpp
	src/create_class.cpp
	src/error.cpp
	src/exception_handler.cpp
	src/function.cpp
	src/inheritance.cpp
	src/link_compatibility.cpp
	src/object_rep.cpp
	src/open.cpp
	src/pcall.cpp
	src/scope.cpp
	src/stack_content_by_name.cpp
	src/weak_ref.cpp
	src/wrapper_base.cpp
)

SET(lb_headers
    
)

ADD_LIBRARY(luabind ${lb_sources} ${lb_headers})


IF(UNIX)
    ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
