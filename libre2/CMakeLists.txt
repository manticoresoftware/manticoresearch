cmake_minimum_required ( VERSION 3.17 )

PROJECT ( RE2 CXX C )
#Parses Makefile and creates the own project based on it
#(actually we only use OFILES var from there)

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/MakefileOrig")
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Makefile" "${CMAKE_CURRENT_SOURCE_DIR}/MakefileOrig" COPYONLY)
endif ()

FILE ( STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/MakefileOrig" _CONTENT  )
FOREACH ( LINE ${_CONTENT} )
	IF ("${LINE}" MATCHES "^(INSTALL_HFILES|HFILES|OFILES)=")
		STRING(REPLACE ";" "" LINE "${LINE}")
		separate_arguments(SRCLIST UNIX_COMMAND "${LINE}")
		foreach (SRC ${SRCLIST})
			IF ("${SRC}" MATCHES "=")
				STRING(REPLACE "=" "" _NAME "${SRC}")
				set (${_NAME})
			else()
				LIST(APPEND ${_NAME} ${SRC})
			endif()
		endforeach()
		if ( _NAME STREQUAL "INSTALL_HFILES" )
			source_group ( "Install headers" FILES ${INSTALL_HFILES} )
		endif ()
		if ( _NAME STREQUAL "HFILES" )
			source_group ( "Headers" FILES ${HFILES} )
		endif ()
	endif ()
endforeach ()

if (OFILES )
	set ( SOURCES )
	foreach ( OBJ ${OFILES} )
		STRING( REPLACE "obj/" "" OBJ "${OBJ}")
		get_filename_component ( NAME ${OBJ} NAME_WE )
		get_filename_component ( DIR ${OBJ} DIRECTORY )
		LIST ( APPEND SOURCES "${DIR}/${NAME}.cc" )
	endforeach ()
else (OFILES )
	MESSAGE ( FATAL_ERROR "No OFILE definition found in Makefile" )
endif (OFILES )


# Set a default build type for single-configuration CMake generators if no build type is set.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE RelWithDebInfo)
	message (STATUS "Automatically set build type to RelWithDebInfo since no other provided")
endif ()

# Select flags.
if (MSVC)
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Zi /O2 /D NDEBUG")
else ()
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -O3 -g -DNDEBUG")
endif ()
#SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "") # reset to empty, global flags will be in game instead

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library ( re2 ${SOURCES} ${HFILES} ${INSTALL_HFILES} )
target_link_libraries(re2 PRIVATE Threads::Threads )
target_include_directories(re2 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>$<INSTALL_INTERFACE:include> )

target_compile_definitions(re2 PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:NOMINMAX>)
target_compile_options(re2 PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/Oi -wd4267>)
target_compile_options(re2 PRIVATE $<$<NOT:$<COMPILE_LANG_AND_ID:CXX,MSVC>>:-Wsign-compare -c>)
target_compile_options(re2 PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-class-memaccess -Wno-parentheses>)

set_target_properties (re2 PROPERTIES
		DEBUG_POSTFIX d
		PDB_NAME re2
		PDB_NAME_DEBUG re2d
		COMPILE_PDB_NAME re2
		COMPILE_PDB_NAME_DEBUG re2d
		)

# installation stuff
set(CMAKE_CMAKE_DIR "lib/cmake/re2")

# ensure ALL externally required headers are exported here
install (FILES ${INSTALL_HFILES} DESTINATION include/re2)
install (TARGETS re2 EXPORT Re2Export RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
install (EXPORT Re2Export FILE re2-targets.cmake DESTINATION "${CMAKE_CMAKE_DIR}" NAMESPACE re2::)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	install (FILES "$<TARGET_FILE_DIR:re2>/re2$<$<CONFIG:Debug>:d>.pdb" EXPORT Re2Export DESTINATION lib OPTIONAL)
endif ()

include(CMakePackageConfigHelpers)

file(WRITE "re2-config.cmake.in"
"\@PACKAGE_INIT@

if(NOT TARGET re2::re2)
    include(\"\${CMAKE_CURRENT_LIST_DIR}/re2-targets.cmake\")
    get_target_property ( configs re2::re2 IMPORTED_CONFIGURATIONS )
    foreach (config DEBUG RELEASE MINSIZEREL)
        if (NOT \${config} IN_LIST configs)
            set_property ( TARGET re2::re2 PROPERTY MAP_IMPORTED_CONFIG_\${config} RelWithDebInfo )
        endif ()
    endforeach ()
endif()
")

configure_package_config_file("re2-config.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/re2-config.cmake" INSTALL_DESTINATION "${CMAKE_CMAKE_DIR}")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/re2-config.cmake" DESTINATION "${CMAKE_CMAKE_DIR}")
