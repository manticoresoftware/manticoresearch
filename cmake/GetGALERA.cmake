# build Galera plugin at configure time
if ( WITH_REPLICATION )
	if ( WIN32 OR APPLE )
		message ( FATAL_ERROR "WITH_REPLICATION requested, but library unavailable on this platform" )
	else ()
		# get library from path provided
		if ( REPLICATION_LIB_PATH )
			find_library ( REPLICATION_LIB_FILE NAMES galera_smm HINTS "${REPLICATION_LIB_PATH}" )
		endif ( REPLICATION_LIB_PATH )

		# build library from source
		if ( NOT REPLICATION_LIB_FILE )
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
			find_library ( REPLICATION_LIB_FILE NAMES galera_smm HINTS "${CMAKE_SOURCE_DIR}/galera" )
		endif ( NOT REPLICATION_LIB_FILE )

		# setup library to package or use at box 
		if ( REPLICATION_LIB_FILE )
			add_library(EXTRA_SEARCHD_LIBRARIES SHARED IMPORTED)
			set_property(TARGET EXTRA_SEARCHD_LIBRARIES PROPERTY IMPORTED_LOCATION "${REPLICATION_LIB_FILE}")
			install(FILES "${REPLICATION_LIB_FILE}" DESTINATION lib COMPONENT APPLICATIONS)
			set ( REPLICATION_LIB libgalera_smm.so )
			GET_SONAME ( "${REPLICATION_LIB_FILE}" REPLICATION_LIB )
			
			message ( STATUS "Replication library will be loaded dynamically in runtime as (${REPLICATION_LIB})" )
		else ()
			message ( FATAL_ERROR "WITH_REPLICATION requested, but no library found in REPLICATION_LIB_PATH '${REPLICATION_LIB_PATH}'" )
		endif ( REPLICATION_LIB_FILE )
		
	endif ( WIN32 OR APPLE )
endif ( WITH_REPLICATION )

# add RPATH only to specific target
function( SET_RPATH BINARYNAME )
	if ( REPLICATION_LIB_FILE )
		# use, i.e. don't skip the full RPATH for the build tree
		set_target_properties (${BINARYNAME} PROPERTIES SKIP_BUILD_RPATH  FALSE)
		# when building, don't use the install RPATH already
		# (but later on when installing)
		set_target_properties(${BINARYNAME} PROPERTIES BUILD_WITH_INSTALL_RPATH FALSE)
		set_target_properties(${BINARYNAME} PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
		# add the automatically determined parts of the RPATH
		# which point to directories outside the build tree to the install RPATH
		set_target_properties(${BINARYNAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
	endif ( REPLICATION_LIB_FILE )
endfunction()