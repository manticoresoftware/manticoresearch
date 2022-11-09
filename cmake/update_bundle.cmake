if (__update_bundle_included)
	return ()
endif ()
set ( __update_bundle_included YES )

IF (POLICY CMP0135)
	CMAKE_POLICY ( SET CMP0135 NEW )
ENDIF ()

# env WRITEB (as bool) means that we can store downloaded stuff to our bundle (that's to refresh the bundle)
# env CACHEB may provide path to persistent folder where we will build heavy stuff (unpacked sources, builds)
include ( helpers )
diag ( DIAGNOSTIC )

SET ( SUFF "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" )
string ( TOLOWER "${SUFF}" SUFF )

# SUFF is line like 'darwin-x86_64' (system-arch)

# special cache folder where artefacts keep. Make it absolute also
if (DEFINED CACHEB)
	if (NOT EXISTS ${CACHEB})
		get_filename_component ( REL_BBUILD "${CACHEB}" REALPATH BASE_DIR "${MANTICORE_BINARY_DIR}" )
		file ( MAKE_DIRECTORY ${REL_BBUILD} )
	endif ()
#	get_filename_component(CACHEB "${CACHEB}" ABSOLUTE)
	diag ( CACHEB )
	set ( HAVE_BBUILD TRUE )
endif ()

# HAVE_BBUILD means we will build in aside folder (inside CACHEB) and then store the result for future.

# make libs_bundle absolute, if any
if (DEFINED LIBS_BUNDLE)
	get_filename_component ( LIBS_BUNDLE "${LIBS_BUNDLE}" ABSOLUTE )
endif ()

unset ( WRITEB )
set ( WRITEB "$ENV{WRITEB}" )
if (WRITEB)
	message ( STATUS "========================================================" )
	message ( STATUS "WRITEB is set, will modify bundle, will collect stuff..." )
	message ( STATUS "${LIBS_BUNDLE}" )
	message ( STATUS "========================================================" )
	file ( MAKE_DIRECTORY ${LIBS_BUNDLE} )
else ()
	message ( STATUS "WRITEB is not set, bundle will NOT be modified..." )
endif ()

diag ( WRITEB )
diag ( LIBS_BUNDLE )
diag ( CACHEB )
diag ( HAVE_BBUILD )

if (HAVE_BBUILD)
	set ( CACHE_BUILDS "${CACHEB}/${SUFF}" )
else ()
	set ( CACHE_BUILDS "${MANTICORE_BINARY_DIR}/cache" )
endif ()

# that is once populate cache to cmake prefix path
append_prefix ( "${CACHE_BUILDS}" )

# get path for build folder. In case with HAVE_BBUILD it will be suffixed with /arch/name flag.
function ( GET_BUILD RESULT NAME )
	if (NOT HAVE_BBUILD)
		set ( detail "local " )
	endif ()
	diags ( "${NAME} build will be set to ${detail}${CACHE_BUILDS}/${NAME}" )
	set ( ${RESULT} "${CACHE_BUILDS}/${NAME}" PARENT_SCOPE )
endfunction ()

# set PLACE to external url or to path in bundle.
# if WRITEB is active, download external url into bundle
function ( select_nearest_url PLACE NAME BUNDLE_URL REMOTE_URL )
	if (NOT EXISTS "${BUNDLE_URL}" AND WRITEB)
		diags ( "fetch ${REMOTE_URL} into ${BUNDLE_URL}..." )
		file ( DOWNLOAD ${REMOTE_URL} ${BUNDLE_URL} SHOW_PROGRESS )
		message ( STATUS "Absent ${NAME} put into ${BUNDLE_URL}" )
	endif ()

	if (EXISTS "${BUNDLE_URL}")
		set ( ${PLACE} "${BUNDLE_URL}" PARENT_SCOPE )
	else ()
		set ( ${PLACE} "${REMOTE_URL}" PARENT_SCOPE )
	endif ()

	diag ( NAME )
	diag ( BUNDLE_URL )
	diag ( REMOTE_URL )
endfunction ()

function ( fetch_and_check NAME URL MD5 OUTDIR )
	include ( FetchContent )
	FetchContent_Declare ( ${NAME} URL "${URL}" URL_MD5 ${MD5} )
	FetchContent_GetProperties ( ${NAME} )
	if (NOT ${NAME}_POPULATED)
		message ( STATUS "Populate ${NAME} from ${URL}" )
		FetchContent_Populate ( ${NAME} )
	endif ()

	string ( TOUPPER "${NAME}" UNAME )
	mark_as_advanced ( FETCHCONTENT_SOURCE_DIR_${UNAME} FETCHCONTENT_UPDATES_DISCONNECTED_${UNAME} )
	set ( ${OUTDIR} "${${NAME}_SOURCE_DIR}" PARENT_SCOPE )
endfunction ()

function ( external_build module MODULE_SRC_NAME MODULE_BUILD_NAME )
	set ( extra_args ${ARGN} )
	# Did we get any optional args?
	set (CMAKE_ARGS )
	list ( LENGTH extra_args num_extra_args )
	if (${num_extra_args} GREATER 0)
		list ( GET extra_args 0 optional_arg )
		set ( CMAKE_ARGS "${CMAKE_ARGS} -D${optional_arg}" )
	endif ()
	if ( CMAKE_ARGS )
		set (CMAKE_ARGS "CMAKE_ARGS ${CMAKE_ARGS}")
	endif()

	set ( MODULE_SRC "${${MODULE_SRC_NAME}}" )
	set ( MODULE_BUILD "${${MODULE_BUILD_NAME}}" )
	configure_file ( ${MANTICORE_SOURCE_DIR}/cmake/external-build.cmake.in ${module}-build/CMakeLists.txt @ONLY )
	execute_process ( COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${module}-build )
	execute_process ( COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${module}-build )
endfunction ()