if (__init_cache_settings_included)
	return()
endif ()
set(__init_cache_settings_included YES)

if (DEFINED ENV{DIAGNOSTIC})
	set(DIAGNOSTIC "$ENV{DIAGNOSTIC}")
endif ()
if (DIAGNOSTIC)
	set (CMAKE_MESSAGE_LOG_LEVEL DEBUG)
endif()

if (DEFINED ENV{LIBS_BUNDLE})
	set(LIBS_BUNDLE "$ENV{LIBS_BUNDLE}")
endif ()

if (NOT LIBS_BUNDLE)
	get_filename_component(LIBS_BUNDLE "${MANTICORE_SOURCE_DIR}/../bundle" ABSOLUTE)
endif ()

if (NOT IS_ABSOLUTE ${LIBS_BUNDLE})
	set(LIBS_BUNDLE "${MANTICORE_BINARY_DIR}/${LIBS_BUNDLE}")
endif ()

SET(LIBS_BUNDLE "${LIBS_BUNDLE}" CACHE PATH "Choose the path to the dir which contains all helper libs like expat, mysql, etc." FORCE)

if (DEFINED ENV{CACHEB})
	set(CACHEB "$ENV{CACHEB}")
endif ()

if (NOT DEFINED CACHEB)
	get_filename_component(CACHEB "${LIBS_BUNDLE}/../cache" ABSOLUTE)
endif ()

if (NOT IS_ABSOLUTE ${CACHEB})
	set(CACHEB "${MANTICORE_BINARY_DIR}/${CACHEB}")
endif ()

if (DEFINED CACHEB)
	SET(CACHEB "${CACHEB}" CACHE PATH "Cache dir where unpacked sources and builds found.")
endif ()
