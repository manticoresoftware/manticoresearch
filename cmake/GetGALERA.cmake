# Configure Galera build
cmake_minimum_required ( VERSION 3.11 FATAL_ERROR )

# Galera is unique dependency, since it never influents overall build, nor provides include headers, just
# standalone dynamic library.
#
# Getting galera pefroms following behaviour:
# - if prebuilds found in usual place (non-cleaned build) - just use them.
# - if prebuilds found in cache/ - use them.
# - if prebuilds found in bundle/<archive> - unpack into usual build folder and use them.
# in these only cases may build for mac/bsd, otherwise this platform is silently skipped.
# - if found sources in bundle - use them (CACHEB step may be just populated it).
# - otherwise use sources from github.
# target galera build to bundle/build, otherwise perform usual build folder.

set(HAVE_GALERA 0) # below will be overriden, if necessary

if (WIN32)
	return() # sorry, no galera for windows
endif ()

include(update_bundle)

set (GALERANAME "galera-cmake-3.x-5.7")
set (GALERA_BUNDLEZIP "${GALERANAME}.zip")

# fixme! Republish into public repo when migration done
set(GALERA_GITHUB "https://github.com/klirichek/galera/archive/cmake-3.x-5.7.zip")
#set(GALERA_GITHUB "https://github.com/manticoresoftware/galera/archive/cmake-3.x-5.7.zip")

function(get_galera_api_version OUTVAR HEADER)
	SET(APIVER "1")
	if (EXISTS "${HEADER}")
		message(STATUS "Parsing ${HEADER} for wsrep API version")
		FILE(READ "${HEADER}" _CONTENT)
		# replace lf into ';' (it makes list from the line)
		STRING(REGEX REPLACE "\n" ";" _CONTENT "${_CONTENT}")
		foreach (LINE ${_CONTENT})
			# match definitions like - #define NAME "VALUE"
			IF ("${LINE}" MATCHES "^#define[ \t]+WSREP_INTERFACE_VERSION[ \t]+\"(.*)\"")
				set(APIVER "${CMAKE_MATCH_1}")
			endif ()
		endforeach ()
	endif ()
	set("${OUTVAR}" "${APIVER}" PARENT_SCOPE)
endfunction()

# add RPATH only to specific target
function(SET_SEARCHD_RPATH GALERA_LIBDIR)
	if (NOT WIN32)
		set(BINARYNAME searchd)
		file(RELATIVE_PATH relgalera "${MANTICORE_BINARY_DIR}/src" "${GALERA_LIBDIR}")
		set (ORIGIN "$ORIGIN")
		if (APPLE)
			set(ORIGIN "@loader_path")
		endif()
		set_target_properties(${BINARYNAME} PROPERTIES INSTALL_RPATH "${ORIGIN}")
		set_target_properties(${BINARYNAME} PROPERTIES BUILD_RPATH "${ORIGIN}/${relgalera}")
		# when building, don't use the install RPATH already
		# (but later on when installing)
		set_target_properties(${BINARYNAME} PROPERTIES BUILD_WITH_INSTALL_RPATH FALSE)
		# add the automatically determined parts of the RPATH
		# which point to directories outside the build tree to the install RPATH
		set_target_properties(${BINARYNAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
		diag(GALERA_LIBDIR)
		diag(${MANTICORE_BINARY_DIR}/src)
		diag(relgalera)
		diag(CMAKE_SYSTEM_NAME)
	endif ()
endfunction()

function(check_imported FOUND BINDIR)
	if (NOT EXISTS "${BINDIR}/galera-targets.cmake")
		return()
	endif()

	include("${BINDIR}/galera-targets.cmake")
	string(TOUPPER "${CMAKE_BUILD_TYPE}" UPB)
	get_target_property(LBB galera::galera_manticore LOCATION_${UPB})
	if (NOT EXISTS ${LBB})
		diags("not exists ${LBB}")
		return()
	endif ()

	get_filename_component(BUILDPATH ${LBB} PATH)
	get_filename_component(BUILDNAME ${LBB} NAME)

	diags ("GALERA_LIBRARY -> ${LBB}")

	set (GALERA_LIBRARY "${LBB}" PARENT_SCOPE)
	set (GALERA_LIBDIR "${BUILDPATH}" PARENT_SCOPE)
	set (GALERA_SONAME "${BUILDNAME}" PARENT_SCOPE)
	set (${FOUND} 1 PARENT_SCOPE)
endfunction()

function(galera_install)
	if (NOT HAVE_GALERA)
		return()
	endif()

	set (GALERA_PATH "${BINPREFIX}lib")
	if (GALERA_LIBRARY)
		# can't make 'true install' for imported target; only file installation available
		diags ("Install galera as usual file from imported target")
		install(FILES ${GALERA_LIBRARY} DESTINATION ${GALERA_PATH} COMPONENT applications NAMELINK_SKIP)
	else()
		diags("Install galera as usual file from imported target")
		install(TARGETS galera_manticore LIBRARY DESTINATION ${GALERA_PATH} COMPONENT applications NAMELINK_SKIP)
	endif()
endfunction()

# knowning version is necessary to further investigations
set(WSREP_PATH "${MANTICORE_SOURCE_DIR}/src/replication" CACHE STRING "")
get_galera_api_version(GALERA_SOVERSION "${WSREP_PATH}/wsrep_api.h")

get_buildd(GALERA_BUILD ${GALERANAME})

# first check 'lazy' case - build from previous run
diags("first check 'lazy' case - build from previous run ${MANTICORE_BINARY_DIR}/galera/${GALERA_SONAME}")
check_imported (HAVE_GALERA "${MANTICORE_BINARY_DIR}/galera")
if (HAVE_GALERA)
	diags("Use 'lazy' prebuilt galera from previous build ${GALERA_LIBDIR}")
	return() # we're done
endif ()

# check build in common cache
diags("check build in common cache ${GALERA_BUILD}/${GALERA_SONAME}")
check_imported(HAVE_GALERA "${GALERA_BUILD}")
if (HAVE_GALERA)
	diags("Use cached prebuilt galera from bundle ${GALERA_LIBDIR}")
	return() # we're done
endif ()

# packet build in the bundle, as bundle/galera-cmake-3.x-5.7-darwin-x86_64.tar.gz
get_platformed_named (GALERA_PLATFORM_BUILD "${GALERANAME}")
diags("packet build in the bundle ${LIBS_BUNDLE}/${GALERA_PLATFORM_BUILD}.tar.gz")
if (EXISTS "${LIBS_BUNDLE}/${GALERA_PLATFORM_BUILD}.tar.gz")
	set(GALERA_LIBDIR "${MANTICORE_BINARY_DIR}/galera")
	fetch_and_unpack(galera_lib "${LIBS_BUNDLE}/${GALERA_PLATFORM_BUILD}.tar.gz" "${GALERA_LIBDIR}")
	check_imported(HAVE_GALERA "${GALERA_LIBDIR}")
	if (HAVE_GALERA)
		diags("Use cached prebuilt galera from bundled archive ${GALERA_LIBDIR}")
		return() # we're done
	endif ()
endif ()

# finally set up build from sources
populate(GALERA_PLACE ${GALERANAME} "${LIBS_BUNDLE}/${GALERA_BUNDLEZIP}" ${GALERA_GITHUB})
get_srcpath(GALERA_SRC ${GALERANAME})

diags("check if src folder is empty")
if (NOT EXISTS "${GALERA_SRC}/README")
	diags("need to fetch sources from ${GALERA_PLACE} to ${GALERA_SRC}")
	fetch_and_unpack(galera ${GALERA_PLACE} ${GALERA_SRC})
endif ()

if (EXISTS "${GALERA_SRC}/README")
	set(GALERA_LIBDIR "${GALERA_BUILD}")
	add_subdirectory(${GALERA_SRC} ${GALERA_BUILD})
	set(HAVE_GALERA 1)
	return()
endif()

UNSET(GALERA_SOVERSION)
# return with HAVE_GALERA = 0