include ( update_bundle )

# MCL source is provided only through the daemon git submodule. Packaging builds
# use only API targets; local non-packaging builds can also build runtime modules
# from the submodule for local testing/debugging.

# Versions of API headers we are need to build with.
set ( NEED_COLUMNAR_API 27 )
set ( NEED_SECONDARY_API 20 )
set ( NEED_KNN_API 16 )

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

function ( import_mcl_runtime_target target file )
	if ( NOT EXISTS "${file}" )
		message ( FATAL_ERROR "MCL runtime artifact for ${target} is missing: ${file}" )
	endif ()

	add_library ( ${target} MODULE IMPORTED GLOBAL )
	set_target_properties ( ${target} PROPERTIES
		IMPORTED_LOCATION "${file}"
		IMPORTED_LOCATION_DEBUG "${file}"
		IMPORTED_LOCATION_RELEASE "${file}"
		IMPORTED_LOCATION_RELWITHDEBINFO "${file}"
		IMPORTED_LOCATION_MINSIZEREL "${file}"
	)
endfunction ()

if (TARGET columnar::columnar_api)
	message ( STATUS "Columnar API targets already defined, skip." )
	return ()
endif ()

backup_paths()

set ( MCL_SUBMODULE_DIR "${MANTICORE_SOURCE_DIR}/mcl" )
if ( NOT EXISTS "${MCL_SUBMODULE_DIR}/CMakeLists.txt" )
	message ( FATAL_ERROR "MCL submodule is required at ${MCL_SUBMODULE_DIR}. Initialize it with: git submodule update --init --recursive mcl" )
endif ()

set ( MCL_PACKAGING_BUILD OFF )
if ( PACK OR DISTR_BUILD )
	set ( MCL_PACKAGING_BUILD ON )
endif ()

set ( MCL_LOCAL_RUNTIME_DEFAULT OFF )
if ( NOT MCL_PACKAGING_BUILD )
	set ( MCL_LOCAL_RUNTIME_DEFAULT ON )
endif ()

option ( MCL_LOCAL_RUNTIME "Build MCL runtime modules from the local submodule for non-packaging developer builds" ${MCL_LOCAL_RUNTIME_DEFAULT} )
set ( MCL_RUNTIME_ARTIFACT_DIR "" CACHE PATH "Directory with prebuilt MCL runtime modules for CI tests" )

if ( MCL_LOCAL_RUNTIME AND MCL_PACKAGING_BUILD )
	message ( STATUS "MCL_LOCAL_RUNTIME is disabled for packaging builds; using MCL API targets only" )
	set ( MCL_LOCAL_RUNTIME OFF )
endif ()

if ( MCL_RUNTIME_ARTIFACT_DIR )
	message ( STATUS "Using MCL API from git submodule with runtime artifacts from: ${MCL_RUNTIME_ARTIFACT_DIR}" )
	set ( API_ONLY ON )
	add_subdirectory ( "${MCL_SUBMODULE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/mcl-api" EXCLUDE_FROM_ALL )
	unset ( API_ONLY )

	get_filename_component ( MCL_RUNTIME_ARTIFACT_DIR "${MCL_RUNTIME_ARTIFACT_DIR}" ABSOLUTE )
	set ( MCL_RUNTIME_COLUMNAR "${MCL_RUNTIME_ARTIFACT_DIR}/${LIB_MANTICORE_COLUMNAR}" )
	set ( MCL_RUNTIME_SECONDARY "${MCL_RUNTIME_ARTIFACT_DIR}/${LIB_MANTICORE_SECONDARY}" )
	set ( MCL_RUNTIME_KNN "${MCL_RUNTIME_ARTIFACT_DIR}/${LIB_MANTICORE_KNN}" )
	set ( MCL_RUNTIME_EMBEDDINGS "${MCL_RUNTIME_ARTIFACT_DIR}/${LIB_MANTICORE_KNN_EMBEDDINGS}" )

	import_mcl_runtime_target ( columnar_lib "${MCL_RUNTIME_COLUMNAR}" )
	import_mcl_runtime_target ( secondary_index "${MCL_RUNTIME_SECONDARY}" )
	import_mcl_runtime_target ( knn_lib "${MCL_RUNTIME_KNN}" )
	import_mcl_runtime_target ( embeddings "${MCL_RUNTIME_EMBEDDINGS}" )
	set ( MANTICORE_KNN_EMBEDDINGS_LIB "${MCL_RUNTIME_EMBEDDINGS}" CACHE INTERNAL "Path to manticoresearch text embeddings library" FORCE )

	function ( special_ubertest_addtest testN tst_name REQUIRES )
		if (NOT NON-RT IN_LIST REQUIRES AND NOT NON-COLUMNAR IN_LIST REQUIRES)
			add_ubertest ( "${testN}" "${tst_name}" "${REQUIRES}" "col" "COLUMNAR" "--rt --ignore-weights --columnar" )
		elseif (NOT NON-SECONDARY IN_LIST REQUIRES)
			add_ubertest ( "${testN}" "${tst_name}" "${REQUIRES}" "secondary" "SECONDARY" "" )
		endif ()
	endfunction ()

	function ( special_ubertest_properties test )
		set_property ( TEST "${test}" APPEND PROPERTY ENVIRONMENT "LIB_MANTICORE_COLUMNAR=$<TARGET_FILE:columnar_lib>" )
		set_property ( TEST "${test}" APPEND PROPERTY ENVIRONMENT "LIB_MANTICORE_SECONDARY=$<TARGET_FILE:secondary_index>" )
		set_property ( TEST "${test}" APPEND PROPERTY ENVIRONMENT "LIB_MANTICORE_KNN=$<TARGET_FILE:knn_lib>" )
		set_property ( TEST "${test}" APPEND PROPERTY ENVIRONMENT "LIB_MANTICORE_KNN_EMBEDDINGS=${MANTICORE_KNN_EMBEDDINGS_LIB}" )
	endfunction ()
elseif ( MCL_LOCAL_RUNTIME )
	message ( STATUS "Using MCL from git submodule with local runtime build: ${MCL_SUBMODULE_DIR}" )
	if ( NOT DEFINED BUILD_EMBEDDINGS_LOCALLY AND NOT MANTICORE_KNN_EMBEDDINGS_LIB )
		set ( BUILD_EMBEDDINGS_LOCALLY ON CACHE BOOL "Build MCL embeddings library locally" )
	endif ()
	add_subdirectory ( "${MCL_SUBMODULE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/mcl-runtime" )
else ()
	message ( STATUS "Using MCL API from git submodule: ${MCL_SUBMODULE_DIR}" )
	set ( API_ONLY ON )
	add_subdirectory ( "${MCL_SUBMODULE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/mcl-api" EXCLUDE_FROM_ALL )
	unset ( API_ONLY )
endif ()

return_if_all_api_found ()

message ( FATAL_ERROR "MCL submodule was found at ${MCL_SUBMODULE_DIR}, but API targets were not exported" )
