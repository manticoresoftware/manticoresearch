# build UNIALGO at configure time
cmake_minimum_required ( VERSION 3.1 FATAL_ERROR )

set ( UNIALGO_LIBDIR "${MANTICORE_BINARY_DIR}/unialgo" )
set ( UNIALGO_SRC "${MANTICORE_BINARY_DIR}/unialgo-src" )
mark_as_advanced ( UNIALGO_SRC UNIALGO_LIBDIR )

include ( FetchContent )
# check whether we have local copy (to not disturb network)

set(UNI_ALGO_DISABLE_NORM ON CACHE BOOL "")
set(UNI_ALGO_DISABLE_PROP ON CACHE BOOL "")
set(UNI_ALGO_DISABLE_BREAK_WORD ON CACHE BOOL "")
set(UNI_ALGO_DISABLE_COLLATE  ON CACHE BOOL "")
set(UNI_ALGO_DISABLE_NFKC_NFKD  ON CACHE BOOL "")
set(UNI_ALGO_DISABLE_SHRINK_TO_FIT  ON CACHE BOOL "")
set(UNI_ALGO_DISABLE_SHRINK_TO_FIT  ON CACHE BOOL "")

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set ( UNIALGO_URL_GITHUB "https://github.com/manticoresoftware/uni-algo/archive/refs/tags/v0.7.2.tar.gz" )
message ( STATUS "Use UNIALGO from github ${UNIALGO_URL_GITHUB}" )
FetchContent_Declare ( unialgo
		SOURCE_DIR "${UNIALGO_SRC}"
		BINARY_DIR "${UNIALGO_LIBDIR}"
		URL ${UNIALGO_URL_GITHUB}
		GIT_TAG cmake-3.x-5.7
		GIT_SHALLOW TRUE
		)

FetchContent_GetProperties ( unialgo )
if ( NOT unialgo_POPULATED )
	FetchContent_Populate ( unialgo )
	add_subdirectory ( ${unialgo_SOURCE_DIR} ${unialgo_BINARY_DIR} )
endif ()
mark_as_advanced ( FETCHCONTENT_FULLY_DISCONNECTED FETCHCONTENT_QUIET FETCHCONTENT_UPDATES_DISCONNECTED
		FETCHCONTENT_SOURCE_DIR_UNIALGO FETCHCONTENT_UPDATES_DISCONNECTED_UNIALGO )
