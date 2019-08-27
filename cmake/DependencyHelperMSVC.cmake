OPTION(EQEMU_FETCH_MSVC_DEPENDENCIES "Automatically fetch vcpkg dependencies for MSCV" ON)
MARK_AS_ADVANCED(EQEMU_FETCH_MSVC_DEPENDENCIES)

SET(EQEMU_MSVC_DEPENDENCIES_VCPKG_X86 "https://github.com/EQEmu/Server/releases/download/v1.2/vcpkg-export-x86.zip" CACHE STRING "The location that windows x86 dependencies will be pulled from.")
SET(EQEMU_MSVC_DEPENDENCIES_VCPKG_X64 "https://github.com/EQEmu/Server/releases/download/v1.2/vcpkg-export-x64.zip" CACHE STRING "The location that windows x64 dependencies will be pulled from.")

MARK_AS_ADVANCED(EQEMU_WINDOWS_DEPENDENCIES_X86)
MARK_AS_ADVANCED(EQEMU_WINDOWS_DEPENDENCIES_X64)

IF(EQEMU_FETCH_MSVC_DEPENDENCIES AND NOT DEFINED VCPKG_TARGET_TRIPLET)
	MESSAGE(STATUS "No existing vcpkg found for MSVC")
	MESSAGE(STATUS "Downloading existing dependencies from releases...")
	
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(DEP_URL ${EQEMU_MSVC_DEPENDENCIES_VCPKG_X64})
	ELSE()
		SET(DEP_URL ${EQEMU_MSVC_DEPENDENCIES_VCPKG_X86})
	ENDIF()
	
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/vcpkg/vcpkg-export-20180828-145455)
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/vcpkg/vcpkg-export-20180828-145854)
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/vcpkg)
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/vcpkg)
	
	FILE(DOWNLOAD ${DEP_URL} ${PROJECT_SOURCE_DIR}/vcpkg/vcpkg.zip 
		SHOW_PROGRESS
		STATUS DOWNLOAD_STATUS)
		
	LIST(GET DOWNLOAD_STATUS 0 STATUS_CODE)
	IF(NOT STATUS_CODE EQUAL 0)
		MESSAGE(FATAL_ERROR "Was unable to download dependencies from ${DEP_URL}")
	ENDIF()
	
	MESSAGE(STATUS "Extracting files")
	EXECUTE_PROCESS(
		COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/vcpkg/vcpkg.zip
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/vcpkg
	)
	
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		INCLUDE("${PROJECT_SOURCE_DIR}/vcpkg/vcpkg-export-20180828-145455/scripts/buildsystems/vcpkg.cmake")
	ELSE()
		INCLUDE("${PROJECT_SOURCE_DIR}/vcpkg/vcpkg-export-20180828-145854/scripts/buildsystems/vcpkg.cmake")
	ENDIF()
	
	#TODO: Perl, will be more complicated
ENDIF()
