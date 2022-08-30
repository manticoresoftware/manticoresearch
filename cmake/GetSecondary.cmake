include ( update_bundle )

set ( SECONDARY_ABI 5 )
set ( SECONDARY_REQUIRED_VER ${SECONDARY_ABI} )

# Note: we don't build, neither link with columnar. Only thing we expect to get is a few interface headers, aka 'columnar_api'.
# Actual usage of columnar is solely defined by availability of the module named below. That module is build (or not built)
# separately outside the manticore.

# In order to debug columnar, you should, namely, debug columnar. Open columnar's source, provide MANTICORE_LOCATOR there and configure.
# It will be 'inverted' project with columnar as main code and manticore as helper aside.
# If you provide locator as 'SOURCE_DIR /path/to/manticore/sources', they will be used inplace, without any copying, and will be available
# to edit in IDE.

if (WIN32)
	set ( EXTENSION dll )
else()
	set ( EXTENSION so )
endif()

set ( LIB_MANTICORE_SECONDARY "lib_manticore_secondary.${EXTENSION}" )

macro ( return_if_secondary_api_found )
	if (TARGET columnar::secondary_api)
		include ( FeatureSummary )
		set_package_properties ( columnar PROPERTIES TYPE RUNTIME
				DESCRIPTION "column-oriented storage library, aiming to provide decent performance with low memory footprint at big data volume"
				URL "https://github.com/manticoresoftware/columnar/"
				)
		trace ( columnar::secondary_api )

		# restore prev find paths to avoid polishing global scope
		set ( CMAKE_FIND_ROOT_PATH "${_CMAKE_FIND_ROOT_PATH}" )
		set ( CMAKE_PREFIX_PATH "${_CMAKE_PREFIX_PATH}" )
		return ()
	endif ()
endmacro ()

# Columnar might be already provided by inverted inclusion - i.e. when sources of manticore included as testing tool into columnar's sources
if (TARGET columnar::secondary_api)
	message ( STATUS "Secondary is already defined, skip." )
	return ()
endif ()

# set current path to modules in local usr
set ( SECONDARY_BUILD "${MANTICORE_BINARY_DIR}/usr/${SECONDARY_ABI}" )

# store prev find paths to avoid polishing global scope
set ( _CMAKE_FIND_ROOT_PATH "${CMAKE_FIND_ROOT_PATH}" )
set ( _CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" )

append_prefix ( "${SECONDARY_BUILD}" )

find_package ( columnar ${SECONDARY_REQUIRED_VER} COMPONENTS secondary_api CONFIG )
return_if_secondary_api_found ()

# Not found. get columnar src, extract secondary_api.
if (DEFINED ENV{COLUMNAR_LOCATOR})
	set ( COLUMNAR_LOCATOR $ENV{COLUMNAR_LOCATOR} )
elseif (EXISTS "${MANTICORE_SOURCE_DIR}/local_columnar_src.txt")
	file ( READ "${MANTICORE_SOURCE_DIR}/local_columnar_src.txt" COLUMNAR_LOCATOR )
else ()
	file ( READ "${MANTICORE_SOURCE_DIR}/columnar_src.txt" COLUMNAR_LOCATOR )
endif ()

string ( CONFIGURE "${COLUMNAR_LOCATOR}" COLUMNAR_LOCATOR ) # that is to expand possible inside variables

configure_file ( ${MANTICORE_SOURCE_DIR}/cmake/secondary-imported.cmake.in secondary-build/CMakeLists.txt )
execute_process ( COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/secondary-build )
execute_process ( COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/secondary-build )

find_package ( columnar ${SECONDARY_REQUIRED_VER} REQUIRED COMPONENTS secondary_api CONFIG )
return_if_secondary_api_found ()

# restore prev find paths to avoid polishing global scope
set ( CMAKE_FIND_ROOT_PATH "${_CMAKE_FIND_ROOT_PATH}" )
set ( CMAKE_PREFIX_PATH "${_CMAKE_PREFIX_PATH}" )