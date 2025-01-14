if (__init_cache_settings_included)
	return ()
endif ()
set ( __init_cache_settings_included YES )

if (DEFINED ENV{DIAGNOSTIC})
	set ( DIAGNOSTIC "$ENV{DIAGNOSTIC}" )
endif ()
if (DIAGNOSTIC)
	set ( CMAKE_MESSAGE_LOG_LEVEL DEBUG )
endif ()

if (DEFINED ENV{LIBS_BUNDLE})
	set ( LIBS_BUNDLE "$ENV{LIBS_BUNDLE}" )
endif ()

if (DEFINED ENV{WIN_BUNDLE})
	set ( WIN_BUNDLE "$ENV{WIN_BUNDLE}" )
endif ()

if (NOT LIBS_BUNDLE)
	get_filename_component ( LIBS_BUNDLE "${MANTICORE_SOURCE_DIR}/../bundle" ABSOLUTE )
endif ()

if (NOT IS_ABSOLUTE ${LIBS_BUNDLE})
	set ( LIBS_BUNDLE "${MANTICORE_BINARY_DIR}/${LIBS_BUNDLE}" )
endif ()

if (NOT WIN_BUNDLE)
	SET ( WIN_BUNDLE "${LIBS_BUNDLE}" )
endif ()

SET ( LIBS_BUNDLE "${LIBS_BUNDLE}" CACHE PATH "Choose the path to the dir which contains downloaded sources for libs like re2, icu, stemmer, etc." FORCE )
SET ( WIN_BUNDLE "${WIN_BUNDLE}" CACHE PATH "Choose the path to the dir which contains win builds of libs like expat, mysql, etc." FORCE )

if (DEFINED ENV{CACHEB})
	set ( CACHEB "$ENV{CACHEB}" )
endif ()

if (NOT DEFINED CACHEB)
	get_filename_component ( CACHEB "${LIBS_BUNDLE}/../cache" ABSOLUTE )
endif ()

if (NOT IS_ABSOLUTE ${CACHEB})
	set ( CACHEB "${MANTICORE_BINARY_DIR}/${CACHEB}" )
endif ()

if (DEFINED CACHEB)
	SET ( CACHEB "${CACHEB}" CACHE PATH "Cache dir where unpacked sources and builds found." )
endif ()

# this macro is need for cross-compiling. If we just add path to CMAKE_PREFIX_PATH, it will NOT work with active root path,
# if search strategy set to 'only'. So, we add path to the root path in this case instead.
macro ( PREPEND_PREFIX PATH )
	if (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE STREQUAL ONLY)
		diags ( "CMAKE_FIND_ROOT_PATH was ${CMAKE_FIND_ROOT_PATH}" )
		list ( REMOVE_ITEM CMAKE_FIND_ROOT_PATH "${PATH}" )
		list ( PREPEND CMAKE_FIND_ROOT_PATH "${PATH}" )
		diags ( "CMAKE_FIND_ROOT_PATH was renewed and became ${CMAKE_FIND_ROOT_PATH}" )
#	endif ()
	else ()
		diags ( "CMAKE_PREFIX_PATH was ${CMAKE_PREFIX_PATH}" )
		list ( REMOVE_ITEM CMAKE_PREFIX_PATH "${PATH}" )
		list ( PREPEND CMAKE_PREFIX_PATH "${PATH}" )
		diags ( "CMAKE_PREFIX_PATH was renewed and became ${CMAKE_PREFIX_PATH}" )
	endif ()
endmacro ()
