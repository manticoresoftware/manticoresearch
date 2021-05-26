cmake_minimum_required(VERSION 3.17)

PROJECT(stemmer C)
FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/mkinc.mak" _CONTENT)
FOREACH (LINE ${_CONTENT})
	IF ("${LINE}" MATCHES "^(snowball_sources|snowball_headers)=")
		STRING(REPLACE ";" "" LINE "${LINE}")
		separate_arguments(SRCLIST UNIX_COMMAND "${LINE}")
		foreach (SRC ${SRCLIST})
			IF ("${SRC}" MATCHES "=")
				STRING(REPLACE "=" "" _NAME "${SRC}")
				set(${_NAME})
			else ()
				LIST(APPEND ${_NAME} ${SRC})
			endif ()
		endforeach ()
	endif ()
endforeach ()

# Set a default build type for single-configuration
# CMake generators if no build type is set.
IF (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)


# all the sources parsed. Now just add the lib
add_library(stemmer ${snowball_sources} ${snowball_headers})
target_include_directories(stemmer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>$<INSTALL_INTERFACE:include/stemmer>)

set_target_properties (stemmer PROPERTIES
		DEBUG_POSTFIX d
		PDB_NAME stemmer
		PDB_NAME_DEBUG stemmerd
		COMPILE_PDB_NAME stemmer
		COMPILE_PDB_NAME_DEBUG stemmerd
		)

# installation stuff
set(CMAKE_CMAKE_DIR "lib/cmake/stemmer")

# ensure ALL externally required headers are exported here
install (DIRECTORY include/ DESTINATION include/stemmer)
install (TARGETS stemmer EXPORT stemmerexport RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
install (EXPORT stemmerexport FILE stemmer-targets.cmake DESTINATION "${CMAKE_CMAKE_DIR}" NAMESPACE stemmer::)
if (CMAKE_C_COMPILER_ID MATCHES "MSVC")
	install (FILES "$<TARGET_FILE_DIR:stemmer>/stemmer$<$<CONFIG:Debug>:d>.pdb" DESTINATION lib)
endif ()

include(CMakePackageConfigHelpers)
file(WRITE "stemmer-config.cmake.in"
		"\@PACKAGE_INIT@

if(NOT TARGET stemmer::stemmer)
    include(\"\${CMAKE_CURRENT_LIST_DIR}/stemmer-targets.cmake\")
    get_target_property ( configs stemmer::stemmer IMPORTED_CONFIGURATIONS )
    foreach (config DEBUG RELEASE MINSIZEREL)
        if (NOT \${config} IN_LIST configs)
            set_property ( TARGET stemmer::stemmer PROPERTY MAP_IMPORTED_CONFIG_\${config} RelWithDebInfo )
        endif ()
    endforeach ()
endif()
")

configure_package_config_file("stemmer-config.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/stemmer-config.cmake" INSTALL_DESTINATION "${CMAKE_CMAKE_DIR}")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/stemmer-config.cmake" DESTINATION "${CMAKE_CMAKE_DIR}")