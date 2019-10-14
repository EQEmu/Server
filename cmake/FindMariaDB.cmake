# - Find mariadbclient
#
# -*- cmake -*-
#
# Find the native MariaDB includes and library
#
#	MariaDB_INCLUDE_DIR	- where to find mysql.h, etc.
#	MariaDB_LIBRARIES		- List of libraries when using MariaDB.
#	MariaDB_FOUND			- True if MariaDB found.
#	The following can be used as a hint as to where to search:
#	MARIADB_ROOT

IF (MariaDB_INCLUDE_DIR AND MariaDB_LIBRARIES)
	# Already in cache, be silent
	SET(MariaDB_FIND_QUIETLY TRUE)
ENDIF (MariaDB_INCLUDE_DIR AND MariaDB_LIBRARIES)

# Include dir
IF(MARIADB_ROOT)
	FIND_PATH(MariaDB_INCLUDE_DIR
		NAMES mariadb_version.h
		PATHS ${MARIADB_ROOT}/include
		PATH_SUFFIXES mysql mariadb
		NO_DEFAULT_PATH
		NO_SYSTEM_ENVIRONMENT_PATH
	)
	FIND_PATH(MariaDB_INCLUDE_DIR
		NAMES mariadb_version.h
		PATH_SUFFIXES mysql mariadb
	)
ELSE(MARIADB_ROOT)
	FIND_PATH(MariaDB_INCLUDE_DIR
		NAMES mariadb_version.h
		PATH_SUFFIXES mysql mariadb
	)
ENDIF(MARIADB_ROOT)

# Library
SET(MariaDB_NAMES libmariadb)
IF(MARIADB_ROOT)
	FIND_LIBRARY(MariaDB_LIBRARY
		NAMES ${MariaDB_NAMES}
		PATHS ${MARIADB_ROOT}/lib
		PATH_SUFFIXES mysql mariadb
		NO_DEFAULT_PATH
		NO_SYSTEM_ENVIRONMENT_PATH
	)
	
	FIND_LIBRARY(MariaDB_LIBRARY
		NAMES ${MariaDB_NAMES}
		PATH_SUFFIXES mysql mariadb
	)
ELSE(MARIADB_ROOT)
	FIND_LIBRARY(MariaDB_LIBRARY
		NAMES ${MariaDB_NAMES} mariadbclient_r mariadbclient
		PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64
		PATH_SUFFIXES mysql mariadb
	)
ENDIF(MARIADB_ROOT)

IF (MariaDB_INCLUDE_DIR AND MariaDB_LIBRARY)
	SET(MariaDB_FOUND TRUE)
	SET(MariaDB_LIBRARIES ${MariaDB_LIBRARY})
ELSE (MariaDB_INCLUDE_DIR AND MariaDB_LIBRARY)
	SET(MariaDB_FOUND FALSE)
	SET(MariaDB_LIBRARIES)
ENDIF (MariaDB_INCLUDE_DIR AND MariaDB_LIBRARY)


# handle the QUIETLY and REQUIRED arguments and set MariaDB_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MariaDB DEFAULT_MSG MariaDB_LIBRARY MariaDB_INCLUDE_DIR)

IF(MariaDB_FOUND)
	SET( MariaDB_LIBRARY_RELEASE ${MariaDB_LIBRARY} )
	SET( MariaDB_LIBRARY_DEBUG ${MariaDB_LIBRARY} )
	SET( MariaDB_LIBRARIES ${MariaDB_LIBRARY_RELEASE} ${MariaDB_LIBRARY_DEBUG} )
ELSE(MariaDB_FOUND)
	SET( MariaDB_LIBRARIES )
ENDIF(MariaDB_FOUND)

MARK_AS_ADVANCED(
	MariaDB_LIBRARY_DEBUG
	MariaDB_LIBRARY_RELEASE
	MariaDB_INCLUDE_DIR
	)
