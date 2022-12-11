# build NLJSON plugin at configure time
cmake_minimum_required ( VERSION 3.1 FATAL_ERROR )

set ( NLJSON_LIBDIR "${MANTICORE_BINARY_DIR}/nljson" )
set ( NLJSON_SRC "${MANTICORE_BINARY_DIR}/nljson-src" )
mark_as_advanced ( NLJSON_SRC NLJSON_LIBDIR )

include ( FetchContent )
# check whether we have local copy (to not disturb network)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_MultipleHeaders ON CACHE BOOL "")
set(JSON_GlobalUDLs OFF CACHE BOOL "")
set ( NLJSON_URL_GITHUB "http://github.com/nlohmann/json/archive/refs/tags/v3.10.5.zip" )
message ( STATUS "Use nljson from github ${NLJSON_URL_GITHUB}" )
FetchContent_Declare ( nljson
		SOURCE_DIR "${NLJSON_SRC}"
		BINARY_DIR "${NLJSON_LIBDIR}"
		URL ${NLJSON_URL_GITHUB}
		GIT_TAG cmake-3.x-5.7
		GIT_SHALLOW TRUE
		)

FetchContent_GetProperties ( nljson )
if ( NOT nljson_POPULATED )
	FetchContent_Populate ( nljson )
	add_subdirectory ( ${nljson_SOURCE_DIR} ${nljson_BINARY_DIR} )
endif ()
mark_as_advanced ( FETCHCONTENT_FULLY_DISCONNECTED FETCHCONTENT_QUIET FETCHCONTENT_UPDATES_DISCONNECTED
		FETCHCONTENT_SOURCE_DIR_NLJSON FETCHCONTENT_UPDATES_DISCONNECTED_NLJSON )
