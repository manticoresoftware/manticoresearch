# Configure Galera build
cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

set ( GALERA_REPO "https://github.com/manticoresoftware/galera" )
set ( GALERA_REV "582dc17" )
set ( GALERA_SRC_MD5 "5bf00736bfaa6da45468ec70b5cbeafe" )

set ( GALERA_GITHUB "${GALERA_REPO}/archive/${GALERA_REV}.zip" )
set ( GALERA_BUNDLE "${LIBS_BUNDLE}/galera-${GALERA_REV}.zip" )

if (DEFINED WITH_GALERA AND NOT WITH_GALERA) # already defined and required NOT to be used
	return ()
endif ()

# here WITH_GALERA is not defined, or explicitly required.
set ( WITH_GALERA ON CACHE BOOL "Build and use Galera replication library" FORCE )
add_feature_info ( Galera WITH_GALERA "replication of indexes" )

# that will read galera location from the target and install it to final destination
include ( printers ) # for diag
function ( install_galera DEST )
	get_target_property ( GALERA_MODULE galera::galera LOCATION )
	diag ( GALERA_MODULE )
	install ( PROGRAMS ${GALERA_MODULE} DESTINATION "${DEST}" COMPONENT searchd )
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
	diagst ( galera::galera "library found ready (no need to build)" )
	return ()
endif ()

# not found. Populate and build cache package for now and future usage.
select_nearest_url ( GALERA_PLACE "galera" ${GALERA_BUNDLE} ${GALERA_GITHUB} )
set ( WSREP_PATH "${MANTICORE_SOURCE_DIR}/src/replication" ) # WSREP_PATH provides path to galera-imported for build
get_build ( GALERA_BUILD galera )
configure_file ( ${MANTICORE_SOURCE_DIR}/cmake/galera-imported.cmake.in galera-build/CMakeLists.txt ) # consumes GALERA_PLACE, GALERA_SRC_MD5, WSREP_PATH, GALERA_BUILD, GALERA_REV
execute_process ( COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/galera-build )
execute_process ( COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/galera-build )

find_package ( galera REQUIRED )
diagst ( galera::galera "library was built and saved" )