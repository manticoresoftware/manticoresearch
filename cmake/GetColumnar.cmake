set(HAVE_COLUMNAR 0) # will be overridden later

include(update_bundle)

set (COLUMNARNAME "columnar")
set (COLUMNAR_BUNDLEZIP "${COLUMNARNAME}.zip")
set (COLUMNAR_GITHUB "https://github.com/manticoresoftware/columnar/archive/refs/heads/1.0.0.zip")

function(check_imported FOUND BINDIR)
	if (NOT EXISTS "${BINDIR}/columnar-targets.cmake")
		return()
	endif()

	include("${BINDIR}/columnar-targets.cmake")
	string(TOUPPER "${CMAKE_BUILD_TYPE}" UPB)
	get_target_property(LBB columnar LOCATION_${UPB})
	if (NOT EXISTS ${LBB})
		diags("not exists ${LBB}")
		return()
	endif ()

	get_filename_component(BUILDPATH ${LBB} PATH)
	get_filename_component(BUILDNAME ${LBB} NAME)

	diags ("COLUMNAR_LIBRARY -> ${LBB}")

	set (COLUMNAR_LIBRARY "${LBB}" PARENT_SCOPE)
	set (COLUMNAR_LIBDIR "${BUILDPATH}" PARENT_SCOPE)
	set (COLUMNAR_SONAME "${BUILDNAME}" PARENT_SCOPE)
	set (${FOUND} 1 PARENT_SCOPE)
endfunction()

function(columnar_install)
	if (NOT HAVE_COLUMNAR)
		return()
	endif()
    if ( APPLE )
        set ( COLUMNAR_PATH "${BINPREFIX}lib" )
    else()
        set ( COLUMNAR_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" )
    endif()
	if (COLUMNAR_LIBRARY)
		# can't make 'true install' for imported target; only file installation available
		diags ("Install columnar as usual file from imported target")
		install(PROGRAMS ${COLUMNAR_LIBRARY} DESTINATION "${COLUMNAR_PATH}" COMPONENT applications )
	else()
		diags("Install columnar as usual file from imported target")
		install(TARGETS columnar_manticore LIBRARY DESTINATION "${COLUMNAR_PATH}" COMPONENT applications NAMELINK_SKIP)
	endif()
endfunction()

get_buildd(COLUMNAR_BUILD ${COLUMNARNAME})

# first check 'lazy' case - build from previous run
diags("first check 'lazy' case - build from previous run ${MANTICORE_BINARY_DIR}/columnar/${COLUMNAR_SONAME}")
check_imported (HAVE_COLUMNAR "${MANTICORE_BINARY_DIR}/columnar")
if (HAVE_COLUMNAR)
	diags("Use 'lazy' prebuilt columnar from previous build ${COLUMNAR_LIBDIR}")
	return() # we're done
endif ()

# finally set up build from sources
populate(COLUMNAR_PLACE ${COLUMNARNAME} "${LIBS_BUNDLE}/${COLUMNAR_BUNDLEZIP}" ${COLUMNAR_GITHUB})
get_srcpath(COLUMNAR_SRC ${COLUMNARNAME})

diags("check if src folder is empty")
if (NOT EXISTS "${COLUMNAR_SRC}/CMakeLists.txt")
	diags("need to fetch sources from ${COLUMNAR_PLACE} to ${COLUMNAR_SRC}")
	fetch_and_unpack(columnar ${COLUMNAR_PLACE} ${COLUMNAR_SRC})
endif ()

if (EXISTS "${COLUMNAR_SRC}/CMakeLists.txt")
	if ( DEFINED DEBUG_COLUMNAR_DEST )
		if ( MSVC )
			set ( DEBUG_COLUMNAR_PATH "${DEBUG_COLUMNAR_DEST}/lib_manticore_columnar.dll" )
		else ()
			set ( DEBUG_COLUMNAR_PATH "${DEBUG_COLUMNAR_DEST}/lib_manticore_columnar.so" )
		endif ()
		message ( STATUS "DEBUG_COLUMNAR_PATH is set to ${DEBUG_COLUMNAR_PATH}" )
	endif ()

	set(COLUMNAR_LIBDIR "${COLUMNAR_BUILD}")
	add_subdirectory(${COLUMNAR_SRC} ${COLUMNAR_BUILD})
	include_directories ( ${COLUMNAR_SRC} )
	set(HAVE_COLUMNAR 1)
endif()
