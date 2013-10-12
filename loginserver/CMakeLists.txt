CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(eqlogin_sources
	Client.cpp
	ClientManager.cpp
	Config.cpp
	DatabaseMySQL.cpp
	DatabasePostgreSQL.cpp
	ErrorLog.cpp
	Main.cpp
	ServerManager.cpp
	WorldServer.cpp
)

IF(MSVC OR MINGW)
	ADD_DEFINITIONS(-DNOMINMAX)
	SET(eqlogin_sources ${eqlogin_sources} Encryption.cpp)
ENDIF(MSVC OR MINGW)

SET(eqlogin_headers
	Client.h
	ClientManager.h
	Config.h
	Database.h
	DatabaseMySQL.h
	DatabasePostgreSQL.h
	Encryption.h
	EQCryptoAPI.h
	ErrorLog.h
	LoginServer.h
	LoginStructures.h
	Options.h
	ServerManager.h
	WorldServer.h
)

IF(UNIX)
	SET(EQEMU_UNIX_ENC_LIBRARY_LOC "${CMAKE_SOURCE_DIR}/dependencies" CACHE PATH "Location of EQEmuAuthCrypto and cryptopp")
	LINK_DIRECTORIES(${EQEMU_UNIX_ENC_LIBRARY_LOC})
ENDIF(UNIX)

ADD_EXECUTABLE(loginserver ${eqlogin_sources} ${eqlogin_headers})

INSTALL(TARGETS loginserver RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(loginserver Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE})

IF(MSVC)
	SET_TARGET_PROPERTIES(loginserver PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(loginserver "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(loginserver "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(loginserver "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(loginserver "z")
	TARGET_LINK_LIBRARIES(loginserver "m")
	TARGET_LINK_LIBRARIES(loginserver "rt")
	TARGET_LINK_LIBRARIES(loginserver "pthread")
	TARGET_LINK_LIBRARIES(loginserver "EQEmuAuthCrypto")
	TARGET_LINK_LIBRARIES(loginserver "cryptopp")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
