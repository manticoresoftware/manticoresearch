# build Galera plugin at configure time
cmake_minimum_required ( VERSION 3.11 FATAL_ERROR )

if ( UNIX AND NOT APPLE )

	set ( GALERA_LIBDIR "${MANTICORE_BINARY_DIR}/galera" )
	set ( GALERA_SRC "${MANTICORE_BINARY_DIR}/galera-src" )
	mark_as_advanced ( GALERA_SRC GALERA_LIBDIR )
	set ( GALERAZIP "galera-cmake-3.x-5.7.zip" )

	include ( FetchContent )
	# check whether we have local copy (to not disturb network)
	if ( LIBS_BUNDLE AND EXISTS "${LIBS_BUNDLE}/${GALERAZIP}" )
		set ( GALERA_URL "${LIBS_BUNDLE}/${GALERAZIP}" )
		file ( SHA1 "${LIBS_BUNDLE}/${GALERAZIP}" SHA1GALERA )
		set ( GALERA_URL_HASH "SHA1=${SHA1GALERA}" )
		message ( STATUS "Use galera from ${LIBS_BUNDLE}/${GALERAZIP}" )
		FetchContent_Declare ( galera
				SOURCE_DIR "${GALERA_SRC}"
				BINARY_DIR "${GALERA_LIBDIR}"
				URL "${GALERA_URL}"
				URL_HASH ${GALERA_URL_HASH}
				)
	else ()
		set ( GALERA_URL_GITHUB "https://github.com/manticoresoftware/galera/archive/cmake-3.x-5.7.zip" )
		message ( STATUS "Use galera from github ${GALERA_URL_GITHUB}" )
		message ( STATUS "(you can download the file and save it as '${GALERAZIP}' into ${LIBS_BUNDLE}/) " )
		FetchContent_Declare ( galera
				SOURCE_DIR "${GALERA_SRC}"
				BINARY_DIR "${GALERA_LIBDIR}"
				URL ${GALERA_URL_GITHUB}
#				GIT_REPOSITORY git@github.com:manticoresoftware/galera.git
#				GIT_REPOSITORY https://github.com/manticoresoftware/galera.git
				GIT_TAG cmake-3.x-5.7
				GIT_SHALLOW TRUE
				)
	endif ()

	FetchContent_GetProperties ( galera )
	if ( NOT galera_POPULATED )
		FetchContent_Populate ( galera )
		set ( WSREP_PATH "${MANTICORE_SOURCE_DIR}/src/replication" CACHE STRING "" )
		add_subdirectory ( ${galera_SOURCE_DIR} ${galera_BINARY_DIR} )
		get_galera_api_version ( "${WSREP_PATH}/wsrep_api.h" GALERA_SOVERSION )
	endif ()
	mark_as_advanced ( FETCHCONTENT_FULLY_DISCONNECTED FETCHCONTENT_QUIET FETCHCONTENT_UPDATES_DISCONNECTED
			FETCHCONTENT_SOURCE_DIR_GALERA FETCHCONTENT_UPDATES_DISCONNECTED_GALERA )
endif()

# add RPATH only to specific target
function( SET_SEARCHD_RPATH )
	if ( UNIX AND NOT APPLE )
		set ( BINARYNAME searchd )
		file ( RELATIVE_PATH relgalera "${MANTICORE_BINARY_DIR}/src" "${GALERA_LIBDIR}"  )
		# use, i.e. don't skip the full RPATH for the build tree
		set_target_properties ( ${BINARYNAME} PROPERTIES SKIP_BUILD_RPATH FALSE)
		# when building, don't use the install RPATH already
		# (but later on when installing)
		set_target_properties( ${BINARYNAME} PROPERTIES BUILD_WITH_INSTALL_RPATH FALSE )
		set_target_properties( ${BINARYNAME} PROPERTIES INSTALL_RPATH "$ORIGIN" )
		set_target_properties( ${BINARYNAME} PROPERTIES BUILD_RPATH "$ORIGIN/${relgalera}" )
		# add the automatically determined parts of the RPATH
		# which point to directories outside the build tree to the install RPATH
  		set_target_properties(${BINARYNAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
	endif (  )
endfunction()