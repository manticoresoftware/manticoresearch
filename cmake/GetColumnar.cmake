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
