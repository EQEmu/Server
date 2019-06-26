#EQEmu CMake
#Variables used:
#EQEMU_DISABLE_CRT_SECURE_WARNINGS
#EQEMU_FAST_FLOATINGPOINT
#EQEMU_ENABLE_CRASH_LOGGING
#EQEMU_DISABLE_SAFESEH
#EQEMU_BUILD_MSVC_MP
#EQEMU_DEBUG_LEVEL
#EQEMU_LOG_LEVEL_STATUS
#EQEMU_LOG_LEVEL_NORMAL
#EQEMU_LOG_LEVEL_ERROR
#EQEMU_LOG_LEVEL_DEBUG
#EQEMU_LOG_LEVEL_QUEST
#EQEMU_LOG_LEVEL_COMMANDS
#EQEMU_LOG_LEVEL_CRASH
#EQEMU_DEPOP_INVALIDATES_CACHE
#EQEMU_ENABLE_BOTS
#EQEMU_DISABLE_LOGSYS
#EQEMU_COMMANDS_LOGGING
#EQEMU_BUILD_SERVER
#EQEMU_BUILD_LOGIN
#EQEMU_BUILD_TESTS
#EQEMU_BUILD_PERL
#EQEMU_BUILD_LUA
#EQEMU_SANITIZE_LUA_LIBS
#EQEMU_BUILD_CLIENT_FILES
#EQEMU_USE_MAP_MMFS
#EQEMU_MAP_DIR

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
IF(POLICY CMP0074)
	cmake_policy(SET CMP0074 NEW)
ENDIF()

#FindMySQL is located here so lets make it so CMake can find it
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

#Our project name is EQEmu
PROJECT(EQEmu)

#Default build type is set to RelWithDebInfo for generators that honor that like makefiles
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies" "${CMAKE_PREFIX_PATH}")

#Add our various windows definitions
IF(MSVC OR MINGW)
	ADD_DEFINITIONS(-D_WINDOWS)
	IF(CMAKE_CL_64)
		ADD_DEFINITIONS(-DWIN64)
	ELSE(CMAKE_CL_64)
		ADD_DEFINITIONS(-DWIN32)
	ENDIF(CMAKE_CL_64)
ENDIF(MSVC OR MINGW)

IF(MSVC)
	IF(CMAKE_CL_64)
		SET(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/zlib_x64")
		SET(MYSQL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/mysql_x64")
		SET(LUA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luaj_x64")
		SET(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/openssl_x64")
		SET(SODIUM_INCLUDE_HINTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsodium/include")
		IF(MSVC_VERSION GREATER 1800)
			SET(SODIUM_LIBRARY_HINTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsodium/x64/Release/v140/dynamic")
		ELSEIF(MSVC_VERSION EQUAL 1800)
			SET(SODIUM_LIBRARY_HINTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsodium/x64/Release/v120/dynamic")
		ELSE()
			SET(SODIUM_LIBRARY_HINTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsodium/x64/Release/v110/dynamic")
		ENDIF()
	ELSE(CMAKE_CL_64)
		SET(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/zlib_x86")
		SET(MYSQL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/mysql_x86")
		SET(LUA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luaj_x86")
		SET(SODIUM_INCLUDE_HINTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsodium/include")
		SET(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/openssl_x86")
		IF(MSVC_VERSION GREATER 1800)
			SET(SODIUM_LIBRARY_HINTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsodium/Win32/Release/v140/dynamic")
		ELSEIF(MSVC_VERSION EQUAL 1800)
			SET(SODIUM_LIBRARY_HINTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsodium/Win32/Release/v120/dynamic")
		ELSE()
			SET(SODIUM_LIBRARY_HINTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsodium/Win32/Release/v110/dynamic")
		ENDIF()
	ENDIF(CMAKE_CL_64)

	#disable CRT warnings on windows cause they're annoying as shit and we use C functions everywhere
	OPTION(EQEMU_DISABLE_CRT_SECURE_WARNINGS "Disable Secure CRT Warnings" ON)
	IF(EQEMU_DISABLE_CRT_SECURE_WARNINGS)
		ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ENDIF(EQEMU_DISABLE_CRT_SECURE_WARNINGS)

	#fast FP if you'd like it
	OPTION(EQEMU_FAST_FLOATINGPOINT "Use MSVC /fp:fast option" ON)
	IF(EQEMU_FAST_FLOATINGPOINT)
		ADD_DEFINITIONS(/fp:fast)
	ENDIF(EQEMU_FAST_FLOATINGPOINT)

	#crash logging currently only works on windows x86/x64
	OPTION(EQEMU_ENABLE_CRASH_LOGGING "Enable crash logging" ON)
	IF(EQEMU_ENABLE_CRASH_LOGGING)
		ADD_DEFINITIONS(-DCRASH_LOGGING)
	ENDIF(EQEMU_ENABLE_CRASH_LOGGING)

	OPTION(EQEMU_BUILD_MSVC_MP "Enable build with multiple processes." ON)
	IF(EQEMU_BUILD_MSVC_MP)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	ENDIF(EQEMU_BUILD_MSVC_MP)

	#We want to compile /MT not /MD so we change that
	FOREACH(flag_var CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO)
		IF(${flag_var} MATCHES "/MD")
			STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		ENDIF(${flag_var} MATCHES "/MD")
	ENDFOREACH(flag_var)

	ADD_DEFINITIONS(-DNOMINMAX)
ELSE(MSVC)
	#Normally set by perl but we don't use the perl flags anymore so we set it.
	ADD_DEFINITIONS(-DHAS_UNION_SEMUN)
ENDIF(MSVC)

#FreeBSD support
IF(UNIX)
	IF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
		ADD_DEFINITIONS(-DFREEBSD)
		ADD_DEFINITIONS(-D_GLIBCXX_USE_C99)
		SET(FREEBSD TRUE)
	ENDIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	IF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
		ADD_DEFINITIONS(-DDARWIN)
		SET(DARWIN TRUE)
	ENDIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
ENDIF(UNIX)

#debug level, 5 is default. Most people wont ever change this but it's there if you want to
SET(EQEMU_DEBUG_LEVEL 5 CACHE STRING "EQEmu debug level:
	0 - Quiet mode Errors to file Status and Normal ignored
	1 - Status and Normal to console, Errors to logfile
	2 - Status, Normal, and Error to console and logfile
	3 - Light debug release errors and status
	4 - Moderate debug release errors and status
	5 - Maximum debug release errors and status
	10 - More errors than you ever wanted to see"
)

SET(EQEMU_LOG_LEVEL_STATUS 2 CACHE STRING "EQEmu logging level for [Status]:
	0 - Disabled
	1 - Ouput to File Enabled
	2 - Output to stdout Enabled
	3 - Output to File and stdout Enabled
	8 - Output to stderr Enabled
	9 - Output to File and stderr Enabled
	11 - Output to File, stdout and stderr Enabled"
)

SET(EQEMU_LOG_LEVEL_NORMAL 3 CACHE STRING "EQEmu logging level for [Normal]:
	0 - Disabled
	1 - Ouput to File Enabled
	2 - Output to stdout Enabled
	3 - Output to File and stdout Enabled
	8 - Output to stderr Enabled
	9 - Output to File and stderr Enabled
	11 - Output to File, stdout and stderr Enabled"
)

SET(EQEMU_LOG_LEVEL_ERROR 2 CACHE STRING "EQEmu logging level for [Error]:
	0 - Disabled
	1 - Ouput to File Enabled
	2 - Output to stdout Enabled
	3 - Output to File and stdout Enabled
	8 - Output to stderr Enabled
	9 - Output to File and stderr Enabled
	11 - Output to File, stdout and stderr Enabled"
)

SET(EQEMU_LOG_LEVEL_DEBUG 3 CACHE STRING "EQEmu logging level for [Debug]:
	0 - Disabled
	1 - Ouput to File Enabled
	2 - Output to stdout Enabled
	3 - Output to File and stdout Enabled
	8 - Output to stderr Enabled
	9 - Output to File and stderr Enabled
	11 - Output to File, stdout and stderr Enabled"
)

SET(EQEMU_LOG_LEVEL_QUEST 2 CACHE STRING "EQEmu logging level for [Quest]:
	0 - Disabled
	1 - Ouput to File Enabled
	2 - Output to stdout Enabled
	3 - Output to File and stdout Enabled
	8 - Output to stderr Enabled
	9 - Output to File and stderr Enabled
	11 - Output to File, stdout and stderr Enabled"
)

SET(EQEMU_LOG_LEVEL_COMMANDS 1 CACHE STRING "EQEmu logging level for [Commands]:
	0 - Disabled
	1 - Ouput to File Enabled
	2 - Output to stdout Enabled
	3 - Output to File and stdout Enabled
	8 - Output to stderr Enabled
	9 - Output to File and stderr Enabled
	11 - Output to File, stdout and stderr Enabled"
)

SET(EQEMU_LOG_LEVEL_CRASH 3 CACHE STRING "EQEmu logging level for [Crash]:
	0 - Disabled
	1 - Ouput to File Enabled
	2 - Output to stdout Enabled
	3 - Output to File and stdout Enabled
	8 - Output to stderr Enabled
	9 - Output to File and stderr Enabled
	11 - Output to File, stdout and stderr Enabled"
)

MARK_AS_ADVANCED(EQEMU_LOG_LEVEL_STATUS EQEMU_LOG_LEVEL_NORMAL EQEMU_LOG_LEVEL_ERROR EQEMU_LOG_LEVEL_DEBUG EQEMU_LOG_LEVEL_QUEST EQEMU_LOG_LEVEL_COMMANDS EQEMU_LOG_LEVEL_CRASH)

#NPC Types Cache Behavior
OPTION(EQEMU_DEPOP_INVALIDATES_CACHE "#repop invalidates the npc_types cache (will cause a larger database hit on #repop but is more convienent)." ON)

#Bots are a compile time option so on/off
OPTION(EQEMU_ENABLE_BOTS "Enable Bots" OFF)

#Disable entire _mlog system (excludes trade/command logs)
OPTION(EQEMU_DISABLE_LOGSYS "Disable Logging INI System" ON)

#Enable GM Command log system
OPTION(EQEMU_COMMANDS_LOGGING "Enable GM Command logs" ON)

IF(EQEMU_COMMANDS_LOGGING)
	ADD_DEFINITIONS(-DCOMMANDS_LOGGING)
ENDIF(EQEMU_COMMANDS_LOGGING)

IF(EQEMU_DISABLE_LOGSYS)
	ADD_DEFINITIONS(-DDISABLE_LOGSYS)
ENDIF(EQEMU_DISABLE_LOGSYS)

IF(EQEMU_ENABLE_BOTS)
	ADD_DEFINITIONS(-DBOTS)
ENDIF(EQEMU_ENABLE_BOTS)

#What to build
OPTION(EQEMU_BUILD_SERVER "Build the game server." ON)
OPTION(EQEMU_BUILD_LOGIN "Build the login server." OFF)
OPTION(EQEMU_BUILD_HC "Build the headless client." OFF)
OPTION(EQEMU_BUILD_TESTS "Build utility tests." OFF)
OPTION(EQEMU_BUILD_PERL "Build Perl parser." ON)
OPTION(EQEMU_BUILD_LUA "Build Lua parser." ON)
OPTION(EQEMU_BUILD_CLIENT_FILES "Build Client Import/Export Data Programs." ON)

#C++11 stuff
IF(NOT MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-user-defined-literal")
	ENDIF()
ENDIF(NOT MSVC)

#Various definitions
IF(EQEMU_BUILD_PERL)
	ADD_DEFINITIONS(-DEMBPERL)
	ADD_DEFINITIONS(-DEMBPERL_PLUGIN)
ENDIF(EQEMU_BUILD_PERL)
IF(EQEMU_BUILD_LUA)
	ADD_DEFINITIONS(-DLUA_EQEMU)
ENDIF(EQEMU_BUILD_LUA)

#Disabled until reevaluation performed
#OPTION(EQEMU_USE_MAP_MMFS "Create and use Zone Map MMF files." OFF)
#IF(EQEMU_USE_MAP_MMFS)
#	ADD_DEFINITIONS(-DUSE_MAP_MMFS)
#ENDIF(EQEMU_USE_MAP_MMFS)

SET(EQEMU_MAP_DIR "./Maps" CACHE STRING "The dir that maps, water maps, and paths are located in.")

ADD_DEFINITIONS(-DEQDEBUG=${EQEMU_DEBUG_LEVEL})
ADD_DEFINITIONS(-DINVERSEXY)
ADD_DEFINITIONS(-DFIELD_ITEMS)
ADD_DEFINITIONS(-DMAP_DIR="${EQEMU_MAP_DIR}")
ADD_DEFINITIONS(-DLOG_LEVEL_STATUS=${EQEMU_LOG_LEVEL_STATUS})
ADD_DEFINITIONS(-DLOG_LEVEL_NORMAL=${EQEMU_LOG_LEVEL_NORMAL})
ADD_DEFINITIONS(-DLOG_LEVEL_ERROR=${EQEMU_LOG_LEVEL_ERROR})
ADD_DEFINITIONS(-DLOG_LEVEL_DEBUG=${EQEMU_LOG_LEVEL_DEBUG})
ADD_DEFINITIONS(-DLOG_LEVEL_QUEST=${EQEMU_LOG_LEVEL_QUEST})
ADD_DEFINITIONS(-DLOG_LEVEL_COMMANDS=${EQEMU_LOG_LEVEL_COMMANDS})
ADD_DEFINITIONS(-DLOG_LEVEL_CRASH=${EQEMU_LOG_LEVEL_CRASH})
ADD_DEFINITIONS(-DGLM_FORCE_RADIANS)
ADD_DEFINITIONS(-DGLM_FORCE_CTOR_INIT)
ADD_DEFINITIONS(-DGLM_ENABLE_EXPERIMENTAL)

#Find everything we need
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(MySQL REQUIRED)
IF(EQEMU_BUILD_PERL)
	FIND_PACKAGE(PerlLibs REQUIRED)
	INCLUDE_DIRECTORIES(SYSTEM "${PERL_INCLUDE_PATH}")
ENDIF(EQEMU_BUILD_PERL)

SET(SERVER_LIBS common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY} libuv fmt recast_navigation)

FIND_PACKAGE(Sodium REQUIRED)
IF(SODIUM_FOUND)
	OPTION(EQEMU_ENABLE_SECURITY "Use Encryption For TCP Connections" ON)
	IF(EQEMU_ENABLE_SECURITY)
		INCLUDE_DIRECTORIES(SYSTEM "${SODIUM_INCLUDE_DIRS}")
		ADD_DEFINITIONS(-DENABLE_SECURITY)
		SET(SERVER_LIBS ${SERVER_LIBS} ${SODIUM_LIBRARIES})
	ENDIF()
ENDIF()

IF(WIN32)
	SET(SERVER_LIBS ${SERVER_LIBS} "ws2_32" "psapi" "iphlpapi" "userenv")
ENDIF()

IF(UNIX)
	SET(SERVER_LIBS ${SERVER_LIBS} ${CMAKE_DL_LIBS} "z" "m" "pthread")
	IF(NOT DARWIN)
		SET(SERVER_LIBS ${SERVER_LIBS} "rt")
	ENDIF()
	SET(SERVER_LIBS ${SERVER_LIBS} "uuid")
ENDIF()

IF(EQEMU_BUILD_LUA)
	FIND_PACKAGE(EQLua51 REQUIRED)
	SET(Boost_USE_STATIC_LIBS OFF)
	SET(Boost_USE_MULTITHREADED ON)
	SET(Boost_USE_STATIC_RUNTIME OFF)
	SET(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/boost")

	FIND_PACKAGE(Boost REQUIRED)
	INCLUDE_DIRECTORIES(SYSTEM "${LUA_INCLUDE_DIR}" "${Boost_INCLUDE_DIRS}")
	INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/libs/luabind")

	OPTION(EQEMU_SANITIZE_LUA_LIBS "Sanitize Lua Libraries (Remove OS and IO standard libraries from being able to run)." ON)
	IF(EQEMU_SANITIZE_LUA_LIBS)
		ADD_DEFINITIONS(-DSANITIZE_LUA_LIBS)
	ENDIF(EQEMU_SANITIZE_LUA_LIBS)
ENDIF(EQEMU_BUILD_LUA)

INCLUDE_DIRECTORIES(SYSTEM "${ZLIB_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES(SYSTEM "${MySQL_INCLUDE_DIR}")
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/common/glm")
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/libs/cereal")
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/libs/libuv/include" )
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/libs/libuv/src")
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/libs/format")
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/libs/recast/detour/include")
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/libs/recast/recast/include")

IF(EQEMU_BUILD_SERVER OR EQEMU_BUILD_LOGIN OR EQEMU_BUILD_TESTS OR EQEMU_BUILD_HC)
	ADD_SUBDIRECTORY(common)
	ADD_SUBDIRECTORY(libs)
ENDIF(EQEMU_BUILD_SERVER OR EQEMU_BUILD_LOGIN OR EQEMU_BUILD_TESTS OR EQEMU_BUILD_HC)
IF(EQEMU_BUILD_SERVER)
	ADD_SUBDIRECTORY(shared_memory)
	ADD_SUBDIRECTORY(world)
	ADD_SUBDIRECTORY(zone)
	ADD_SUBDIRECTORY(ucs)
	ADD_SUBDIRECTORY(queryserv)
	ADD_SUBDIRECTORY(eqlaunch)
ENDIF(EQEMU_BUILD_SERVER)
IF(EQEMU_BUILD_LOGIN)
	ADD_SUBDIRECTORY(loginserver)
ENDIF(EQEMU_BUILD_LOGIN)

IF(EQEMU_BUILD_HC)
	ADD_SUBDIRECTORY(hc)
ENDIF(EQEMU_BUILD_HC)

IF(EQEMU_BUILD_TESTS)
	ADD_SUBDIRECTORY(tests)
ENDIF(EQEMU_BUILD_TESTS)

IF(EQEMU_BUILD_CLIENT_FILES)
	ADD_SUBDIRECTORY(client_files)
ENDIF(EQEMU_BUILD_CLIENT_FILES)
