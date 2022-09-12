include ( update_bundle )

# How to perform version update (check-list):
#
# Say, you want to upgrade secondary headers to v.13
# --------- On columnar side ------------
# 1. Change value of constant LIB_VERSION in secondary/secondary.h to 13
# 2. Commit and publish the changes.
# 3. Wait until changes are mirrored to github. Ensure it is tagged as 'c16-s13' (mirroring script should do it)
# 4. If the tag wasn't appear, push it manually, as:
#	git tag c16-s13
#	git push origin c16-s13 # here you must write your alias of github (NOT gitlab) repo instead of 'origin'
# --------- On manticore side ------------
# 1. Fix the numbers NEED_COLUMNAR_API and NEED_SECONDARY_API according to your upgrade
# 2. Reconfigure build.
#
# Notice, with tagged revision in columnar repo you don't need to touch `columnar_src.txt` file anymore, however you
# still can do it for any specific requirements.

# Versions of API headers we are need to build with.
set ( NEED_COLUMNAR_API 16 )
set ( NEED_SECONDARY_API 5 )


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

set ( LIB_MANTICORE_COLUMNAR "lib_manticore_columnar.${EXTENSION}" )
set ( LIB_MANTICORE_SECONDARY "lib_manticore_secondary.${EXTENSION}" )

macro ( backup_paths )
	set ( _CMAKE_FIND_ROOT_PATH "${CMAKE_FIND_ROOT_PATH}" )
	set ( _CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" )
endmacro()

macro ( restore_paths )
	set ( CMAKE_FIND_ROOT_PATH "${_CMAKE_FIND_ROOT_PATH}" )
	set ( CMAKE_PREFIX_PATH "${_CMAKE_PREFIX_PATH}" )
endmacro ()

macro ( return_if_all_api_found )
	if (TARGET columnar::columnar_api)
		set ( _HAS_COLUMNAR ON )
	endif ()

	if (TARGET columnar::secondary_api)
		set ( _HAS_SECONDARY ON )
	endif ()

	if (_HAS_COLUMNAR AND _HAS_SECONDARY)
		include ( FeatureSummary )
		set_package_properties ( columnar PROPERTIES TYPE RUNTIME
				DESCRIPTION "column-oriented storage library, aiming to provide decent performance with low memory footprint at big data volume, and secondary index library"
				URL "https://github.com/manticoresoftware/columnar/"
				)
		trace ( columnar::columnar_api )
		trace ( columnar::secondary_api )

		# restore prev find paths to avoid polishing global scope
		restore_paths()
		return ()
	endif ()
endmacro ()

# Columnar might be already provided by inverted inclusion - i.e. when sources of manticore included as testing tool into columnar's sources
if (TARGET columnar::columnar_api)
	message ( STATUS "Columnar is already defined, skip." )
	return ()
endif ()

# expected version
set ( NEED_API_NUMERIC_VERSION "${NEED_COLUMNAR_API}.${NEED_SECONDARY_API}" )
set ( AUTO_TAG "c${NEED_COLUMNAR_API}-s${NEED_SECONDARY_API}" )

# set current path to modules in local usr
set ( COLUMNAR_BUILD "${MANTICORE_BINARY_DIR}/usr/${AUTO_TAG}" )

# store prev find paths to avoid polishing global scope
backup_paths()

append_prefix ( "${COLUMNAR_BUILD}" )

find_package ( columnar "${NEED_API_NUMERIC_VERSION}" EXACT COMPONENTS columnar_api secondary_api CONFIG )
return_if_all_api_found ()

# Not found. get columnar src, extract columnar_api.
if (DEFINED ENV{COLUMNAR_LOCATOR})
	set ( COLUMNAR_LOCATOR $ENV{COLUMNAR_LOCATOR} )
elseif (EXISTS "${MANTICORE_SOURCE_DIR}/local_columnar_src.txt")
	file ( STRINGS "${MANTICORE_SOURCE_DIR}/local_columnar_src.txt" COLUMNAR_LOCATOR LIMIT_COUNT 1 )
else ()
	file ( STRINGS "${MANTICORE_SOURCE_DIR}/columnar_src.txt" COLUMNAR_LOCATOR LIMIT_COUNT 1)
endif ()

string ( CONFIGURE "${COLUMNAR_LOCATOR}" COLUMNAR_LOCATOR ) # that is to expand possible inside variables

configure_file ( ${MANTICORE_SOURCE_DIR}/cmake/columnar-imported.cmake.in columnar-build/CMakeLists.txt )
execute_process ( COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/columnar-build )
execute_process ( COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/columnar-build )

find_package ( columnar ${NEED_API_NUMERIC_VERSION} EXACT REQUIRED COMPONENTS columnar_api secondary_api CONFIG )
return_if_all_api_found ()

# restore prev find paths to avoid polishing global scope
restore_paths()