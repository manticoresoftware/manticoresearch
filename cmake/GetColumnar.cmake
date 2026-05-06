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
set ( NEED_COLUMNAR_API 27 )
set ( NEED_SECONDARY_API 20 )
set ( NEED_KNN_API 13 )

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
set ( LIB_MANTICORE_KNN "lib_manticore_knn.${EXTENSION}" )
set ( LIB_MANTICORE_KNN_EMBEDDINGS "lib_manticore_knn_embeddings.${EXTENSION}" )

macro ( backup_paths )
	set ( _CMAKE_FIND_ROOT_PATH "${CMAKE_FIND_ROOT_PATH}" )
	set ( _CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" )
endmacro()

macro ( restore_paths )
	set ( CMAKE_FIND_ROOT_PATH "${_CMAKE_FIND_ROOT_PATH}" )
	set ( CMAKE_PREFIX_PATH "${_CMAKE_PREFIX_PATH}" )
endmacro ()

macro ( return_if_all_api_found )
	if (TARGET columnar::columnar_api AND TARGET columnar::secondary_api AND TARGET columnar::knn_api)
		include ( FeatureSummary )
		set_package_properties ( columnar PROPERTIES TYPE RUNTIME
			DESCRIPTION "columnar, secondary, knn, and embeddings APIs from MCL"
			URL "https://github.com/manticoresoftware/columnar/"
		)
		restore_paths()
		return ()
	endif ()
endmacro ()

if (TARGET columnar::columnar_api)
	message ( STATUS "Columnar API targets already defined, skip." )
	return ()
endif ()

backup_paths()

set ( MCL_SUBMODULE_DIR "${MANTICORE_SOURCE_DIR}/mcl" )
set ( USE_MCL_SUBMODULE OFF )
if ( EXISTS "${MCL_SUBMODULE_DIR}/CMakeLists.txt" )
	set ( USE_MCL_SUBMODULE ON )
endif ()

if ( USE_MCL_SUBMODULE )
	message ( STATUS "Using MCL from git submodule: ${MCL_SUBMODULE_DIR}" )
	set ( API_ONLY ON )
	add_subdirectory ( "${MCL_SUBMODULE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/mcl-api" EXCLUDE_FROM_ALL )
	unset ( API_ONLY )
	return_if_all_api_found ()
	message ( FATAL_ERROR "MCL submodule was found at ${MCL_SUBMODULE_DIR}, but API targets were not exported" )
endif ()

message ( WARNING "MCL submodule is not available at ${MCL_SUBMODULE_DIR}; trying installed columnar package. MCL runtime support may be unavailable if headers are not installed system-wide." )
find_package ( columnar CONFIG QUIET COMPONENTS columnar_api secondary_api knn_api )
return_if_all_api_found ()

message ( FATAL_ERROR "Could not find MCL API targets. Initialize ./mcl submodule or install a compatible columnar package that exports columnar::columnar_api, columnar::secondary_api, and columnar::knn_api." )
