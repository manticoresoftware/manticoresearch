# build Galera plugin at configure time
if ( WIN32 OR APPLE )
	message ( FATAL_ERROR "WITH_REPLICATION requested, but library unavailable on this platform" )
else ()
	if ( REPLICATION_LIB_PATH )
		find_library ( REPLICATION_LIB NAMES galera_smm HINTS "${REPLICATION_LIB_PATH}" )
	endif ( REPLICATION_LIB_PATH )

	if ( NOT REPLICATION_LIB )
		execute_process ( COMMAND git submodule init
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
		execute_process ( COMMAND git submodule update --remote
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
		execute_process ( COMMAND git submodule init
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/galera )
		execute_process ( COMMAND git submodule update --remote
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/galera )
		if ( CMAKE_BUILD_TYPE STREQUAL Debug )
			execute_process ( COMMAND scripts/build.sh --debug
					WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/galera
					RESULT_VARIABLE reply_result
					OUTPUT_FILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log )
		else ()
			execute_process ( COMMAND scripts/build.sh
					WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/galera
					RESULT_VARIABLE reply_result
					OUTPUT_FILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log )
		endif ( CMAKE_BUILD_TYPE STREQUAL Debug )
		
		if ( reply_result )
			message ( FATAL_ERROR "CMake step for build Galera replication failed: ${reply_result}" )
		endif ()
		find_library ( REPLICATION_LIB NAMES galera_smm HINTS "${CMAKE_SOURCE_DIR}/galera" )
	endif ( NOT REPLICATION_LIB )

	if ( REPLICATION_LIB )
		message ( STATUS "Replication library will be loaded dynamically in runtime as (${REPLICATION_LIB})" )
	else ()
		message ( FATAL_ERROR "WITH_REPLICATION requested, but no library found in REPLICATION_LIB_PATH '${REPLICATION_LIB_PATH}'" )
	endif ( REPLICATION_LIB )
endif ( WIN32 OR APPLE )