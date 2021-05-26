include(update_bundle)

set (COLUMNAR_ABI 8)
set (COLUMNAR_REQUIRED_VER 1.${COLUMNAR_ABI} )

# Path COLUMNAR_SRC has to be provided from outside. If it is defined, columnar sources will be added via 'add_subdirectory',
# as submodule from that folder. Note that path to columnar in that case you have to provide in runtime via env.

# Variable SKIP_COLUMNAR is set to install only columnar API and build manticore; NOT build columnar itself.

# Otherwise we first try perform find_package(), and if not success - download columnar sources, build them and install
# into temporary folder which then will be used in find_package() call to locate columnar_api

if (WIN32)
	set (LIB_MANTICORE_COLUMNAR "lib_manticore_columnar.dll")
else ()
	set (LIB_MANTICORE_COLUMNAR "lib_manticore_columnar.so")
endif ()

macro (return_if_columnar_found)
	if (TARGET columnar::columnar)
		get_target_property (COLUMNARLIB columnar::columnar LOCATION)
		get_filename_component (LIB_MANTICORE_COLUMNAR ${COLUMNARLIB} NAME)
		message (STATUS "Name is ${LIB_MANTICORE_COLUMNAR}")
		message (STATUS "Location is ${COLUMNARLIB}")
		trace (columnar::columnar)
	else()
		message (STATUS "Location of columnar is not known; you need to provide it manually in env if you want in-place testing possible")
	endif ()
	include (FeatureSummary)
	set_package_properties (columnar PROPERTIES TYPE RUNTIME
			DESCRIPTION "column-oriented storage library, aiming to provide decent performance with low memory footprint at big data volume"
			URL "https://github.com/manticoresoftware/columnar/"
			)
	if (TARGET columnar::columnar_api)
		trace (columnar::columnar_api)
		return ()
	endif ()
endmacro ()

# special CI path. Columnar set this with path to it's sources. We don't need to build it, and also target API assumed available
if (TARGET columnar::columnar_api)
	message (STATUS "Columnar is already defined, skip.")
	return ()
endif ()

if (COLUMNAR_SRC)
	set (SKIP_COLUMNAR FALSE)
	add_subdirectory ("${COLUMNAR_SRC}" "${MANTICORE_BINARY_DIR}/columnar-build")
	message (STATUS "Name is ${LIB_MANTICORE_COLUMNAR}, build as sub-project.")
	message (STATUS "Location is unknown, somewhere in subdir of columnar-build")
	trace_internal (columnar::columnar)
	include (FeatureSummary)
	set_package_properties (columnar PROPERTIES TYPE RUNTIME
			DESCRIPTION "column-oriented storage library, aiming to provide decent performance with low memory footprint at big data volume"
			URL "https://github.com/manticoresoftware/columnar/"
			)
	return()
endif()

# set current path to modules in local usr
set (COLUMNAR_BUILD "${MANTICORE_BINARY_DIR}/usr")
list (APPEND CMAKE_PREFIX_PATH "${COLUMNAR_BUILD}")
message (DEBUG "CMAKE_PREFIX_PATH became ${CMAKE_PREFIX_PATH}")

find_package (columnar ${COLUMNAR_REQUIRED_VER} CONFIG)
return_if_columnar_found ()

# Not found. get columnar src, build columnar_api.
set (SKIP_COLUMNAR TRUE) # that will cause columnar NOT to build, only build columnar_api

if (DEFINED ENV{COLUMNAR_LOCATOR})
	set (COLUMNAR_LOCATOR $ENV{COLUMNAR_LOCATOR})
elseif (EXISTS "${MANTICORE_SOURCE_DIR}/local_columnar_src.txt")
	file (READ "${MANTICORE_SOURCE_DIR}/local_columnar_src.txt" COLUMNAR_LOCATOR)
else ()
	file (READ "${MANTICORE_SOURCE_DIR}/columnar_src.txt" COLUMNAR_LOCATOR)
endif ()

string (CONFIGURE "${COLUMNAR_LOCATOR}" COLUMNAR_LOCATOR) # that is to expand possible inside variables

configure_file (${MANTICORE_SOURCE_DIR}/cmake/columnar-imported.cmake.in columnar-build/CMakeLists.txt)
execute_process (COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/columnar-build)
execute_process (COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/columnar-build)

find_package (columnar ${COLUMNAR_REQUIRED_VER} REQUIRED CONFIG)
return_if_columnar_found ()