# Configure Galera build
cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

set ( GALERA_REPO "https://github.com/manticoresoftware/galera" )
set ( GALERA_REV "a79a074" )
set ( GALERA_SRC_MD5 "720cbbed3916ce217285349140f1fca6" )
set ( GALERA_VERSION "25.3.35" )

set ( WSREP_REPO "https://github.com/percona/wsrep-API" )
set ( WSREP_REV "2c211e1" ) # or "percona-3.x-5.7-v31"
set ( WSREP_SRC_MD5 "da31c60185c0fb4a35cf5e092db82059" ) # or "cc77ccec1a8144a6d9009ac603400406" when use "percona..."

set ( GALERA_GITHUB "${GALERA_REPO}/archive/${GALERA_REV}.zip" )
set ( GALERA_BUNDLE "${LIBS_BUNDLE}/galera-${GALERA_REV}.zip" )

set ( WSREP_GITHUB "${WSREP_REPO}/archive/${WSREP_REV}.zip" )
set ( WSREP_BUNDLE "${LIBS_BUNDLE}/wsrep-${WSREP_REV}.zip" )

if (DEFINED WITH_GALERA AND NOT WITH_GALERA) # already defined and required NOT to be used
	return ()
endif ()

# here WITH_GALERA is not defined, or explicitly required.
set ( WITH_GALERA ON CACHE BOOL "Build Galera replication library" FORCE )
add_feature_info ( Galera WITH_GALERA "table replication" )

# that will read galera location from the target and install it to final destination
include ( printers ) # for diag
function ( install_galera DEST COMPONENT )
	if (TARGET galera::galera)
		get_target_property ( GALERA_MODULE galera::galera LOCATION )
		diag ( GALERA_MODULE )
		install ( PROGRAMS ${GALERA_MODULE} DESTINATION "${DEST}" COMPONENT "${COMPONENT}" )
	endif()
endfunction ()

function ( cache_galera_module_name )
	get_target_property ( GALERA_MODULE galera::galera LOCATION )
	get_filename_component ( _name ${GALERA_MODULE} NAME )
	SET ( GALERA_SONAME "${_name}" CACHE STRING "Filename of galera replication library" )
endfunction ()

# that will find prebuilt, if any
include ( update_bundle ) # that would actualize CMAKE_PREFIX_PATH with our cache
find_package ( galera QUIET )
set_package_properties ( galera PROPERTIES TYPE RUNTIME
		DESCRIPTION "provides support for replication"
		URL "${GALERA_REPO}"
		)
if (TARGET galera::galera)
	diagst ( galera::galera "library found (no need to build)" )
	return ()
endif ()

# not found. Populate and build cache package for now and future usage.
select_nearest_url ( GALERA_PLACE "galera" ${GALERA_BUNDLE} ${GALERA_GITHUB} )
select_nearest_url ( WSREP_PLACE "wsrep" ${WSREP_BUNDLE} ${WSREP_GITHUB} ) # WSREP_PATH provides path to galera-imported for build
get_build ( GALERA_BUILD galera )
configure_file ( ${MANTICORE_SOURCE_DIR}/cmake/galera-imported.cmake.in galera-build/CMakeLists.txt @ONLY ) # consumes GALERA_PLACE, GALERA_SRC_MD5, WSREP_PATH, GALERA_BUILD, GALERA_REV
execute_process ( COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/galera-build )
execute_process ( COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/galera-build )

find_package ( galera REQUIRED )
diagst ( galera::galera "library built and saved" )
