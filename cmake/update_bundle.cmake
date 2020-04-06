if (__update_bundle_included)
	return()
endif ()
set(__update_bundle_included YES)

# env WRITEB (as bool) means that we can store downloaded stuff to our bundle (that's to refresh the bundle)
# env CACHE may provide path to persistent folder where we will build heavy stuff (unpacked sources, builds)

function(DIAG VARR)
	if (DIAGNOSTIC)
		message(STATUS "${VARR} -> ${${VARR}}")
	endif ()
endfunction()

function(DIAGS MSG)
	if (DIAGNOSTIC)
		message(STATUS "${MSG}")
	endif ()
endfunction()

diag (DIAGNOSTIC)

SET(SUFF "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
string(TOLOWER "${SUFF}" SUFF)
if (MSVC OR XCODE OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
	SET(SUFFD "${SUFF}")
else()
	SET(SUFFD "${CMAKE_BUILD_TYPE}-${SUFF}")
endif ()
string(TOLOWER "${SUFFD}" SUFFD)
diag (CMAKE_BUILD_TYPE)

# special cache folder where artefacts keep. Make it absolute also
if (DEFINED CACHEB)
	if (NOT EXISTS ${CACHEB})
		get_filename_component(REL_BBUILD "${CACHEB}" REALPATH BASE_DIR "${MANTICORE_BINARY_DIR}")
		file(MAKE_DIRECTORY ${REL_BBUILD})
	endif()
#	get_filename_component(CACHEB "${CACHEB}" ABSOLUTE)
	diag(CACHEB)
	set(HAVE_BBUILD TRUE)
endif()

# make libs_bundle absolute, if any
if (DEFINED LIBS_BUNDLE)
	get_filename_component(LIBS_BUNDLE "${LIBS_BUNDLE}" ABSOLUTE)
endif ()

unset(WRITEB)
set(WRITEB "$ENV{WRITEB}")
if (WRITEB)
	message(STATUS "========================================================")
	message(STATUS "WRITEB is set, will modify bundle, will collect stuff...")
	message(STATUS "${LIBS_BUNDLE}")
	message(STATUS "========================================================")
	file(MAKE_DIRECTORY ${LIBS_BUNDLE} )
else()
	message(STATUS "WRITEB is not set, bundle will NOT be modified...")
endif ()

diag (WRITEB)
diag (LIBS_BUNDLE)
diag (CACHEB)
diag (HAVE_BBUILD)

function(GET_PLATFORMED_NAME RESULT NAME)
	set (${RESULT} "${NAME}-${SUFF}" PARENT_SCOPE)
endfunction()

function(GET_PLATFORMED_NAMED RESULT NAME)
	set(${RESULT} "${NAME}-${SUFFD}" PARENT_SCOPE)
endfunction()

function(GET_BUILD RESULT NAME)
	if (HAVE_BBUILD)
		diags("${NAME} build will be set to ${CACHEB}/${NAME}-${SUFF}")
		set(${RESULT} "${CACHEB}/${NAME}-${SUFF}" PARENT_SCOPE)
	else ()
		diags("${NAME} build will be set to local ${MANTICORE_BINARY_DIR}/${NAME}")
		set(${RESULT} "${MANTICORE_BINARY_DIR}/${NAME}" PARENT_SCOPE)
	endif ()
endfunction()

function(GET_BUILDD RESULT NAME)
	if (HAVE_BBUILD)
		diags("${NAME} build will be set to ${CACHEB}/${NAME}-${SUFFD}")
		set(${RESULT} "${CACHEB}/${NAME}-${SUFFD}" PARENT_SCOPE)
	else ()
		diags("${NAME} build will be set to local ${MANTICORE_BINARY_DIR}/${NAME}")
		set(${RESULT} "${MANTICORE_BINARY_DIR}/${NAME}" PARENT_SCOPE)
	endif ()
endfunction()

# get name for source folder. It will give either local folder, or build in bundle
function(GET_SRCPATH RESULT NAME)
	if (HAVE_BBUILD)
		diags("${NAME} src will be set to ${CACHEB}/${NAME}-src")
		set(${RESULT} "${CACHEB}/${NAME}-src" PARENT_SCOPE)
	else ()
		diags("${NAME} src will be set to local ${MANTICORE_BINARY_DIR}/${NAME}-src")
		set(${RESULT} "${MANTICORE_BINARY_DIR}/${NAME}-src" PARENT_SCOPE)
	endif ()
endfunction()

# set PLACE to external url or to path in bundle.
# if WRITEB is active, download external url into bundle
function ( POPULATE PLACE NAME BUNDLE_URL REMOTE_URL )
	if (NOT EXISTS "${BUNDLE_URL}" AND WRITEB )
		diags("fetch ${REMOTE_URL} into ${BUNDLE_URL}..." )
		file(DOWNLOAD ${REMOTE_URL} ${BUNDLE_URL} SHOW_PROGRESS)
		message(STATUS "Absent ${NAME} put into ${BUNDLE_URL}")
	endif()

	if (EXISTS "${BUNDLE_URL}")
		set (${PLACE} "${BUNDLE_URL}" PARENT_SCOPE)
	else()
		set (${PLACE} "${REMOTE_URL}" PARENT_SCOPE)
	endif()

	diag (NAME)
	diag (BUNDLE_URL)
	diag (REMOTE_URL )
endfunction()

# fetches given PLACE (from bundle tarball or remote) to TARGET_SRC folder.
function(FETCH_AND_UNPACK NAME PLACE TARGET_SRC)
	include(FetchContent)
	if (EXISTS ${PLACE})
		file(SHA1 "${PLACE}" SHA1SUM)
		set(SHA1SUM "SHA1=${SHA1SUM}")
		FetchContent_Declare(${NAME} SOURCE_DIR "${TARGET_SRC}" URL "${PLACE}" URL_HASH ${SHA1SUM})
	else ()
		FetchContent_Declare(${NAME} SOURCE_DIR "${TARGET_SRC}" URL "${PLACE}")
	endif ()

	FetchContent_GetProperties(${NAME})
	if (NOT ${NAME}_POPULATED)
		message(STATUS "Populate ${NAME} from ${PLACE}")
		FetchContent_Populate(${NAME})
	endif ()

	string(TOUPPER "${NAME}" UNAME)
	mark_as_advanced (FETCHCONTENT_SOURCE_DIR_${UNAME} FETCHCONTENT_UPDATES_DISCONNECTED_${UNAME} )
endfunction()

# default if custom is not defined
function (FIND_LIB_BUILD_DEFAULT LIB SRCDIR BINDIR)
	string(TOLOWER "${LIB}" SLIB)

	diags("FIND_LIB_BUILD_DEFAULT ${LIB} ${SRCDIR} ${BINDIR}")
	if (NOT EXISTS "${BINDIR}/${SLIB}-targets.cmake")
		diags("not found ${SLIB}-targets.cmake")
		return()
	endif()

	include("${BINDIR}/${SLIB}-targets.cmake")

	get_target_property(INC ${SLIB}::${SLIB} INTERFACE_INCLUDE_DIRECTORIES)
	IF (NOT EXISTS ${INC})
		diags("not exists ${INC}")
		return()
	endif ()

	# for multi config - check existance of built RelWithDebInfo and Debug libraries
	# (if both present - link as imported, otherwise fall back to sources)
	if (NOT DEFINED CMAKE_BUILD_TYPE)
		diags("not defined cmake_build_type, will try RelWithDebInfo and Debug")
		get_target_property(LBR ${SLIB}::${SLIB} IMPORTED_LOCATION_RELWITHDEBINFO)
		if (NOT EXISTS ${LBR})
			diags("not exists rel-wiht-deb-info ${LBR}")
			return()
		endif ()

		get_target_property(LBD ${SLIB}::${SLIB} IMPORTED_LOCATION_DEBUG)
		if (NOT EXISTS ${LBD})
			diags("not exists debug ${LBD}")
			return()
		endif ()
	else()

		# for single config - just check existance of single library
		string(TOUPPER "${CMAKE_BUILD_TYPE}" UPB)
		get_target_property(LBB ${SLIB}::${SLIB} IMPORTED_LOCATION_${UPB})
		if (NOT EXISTS ${LBB})
			diags("not exists ${LBB}")
			return()
		endif()
	endif()

	set(${LIB}_FOUND TRUE PARENT_SCOPE)
	set(${LIB}_LIBRARIES "${SLIB}::${SLIB}" PARENT_SCOPE)
	diag(CMAKE_BUILD_TYPE)
endfunction()



# call function FIND_${LIB}_BUILD by name
macro(FIND_LIB_BUILD LIB LIB_SRC LIB_BUILD)
	if (COMMAND find_${LIB}_build)
		set(__invoke_temp_file "${CMAKE_CURRENT_BINARY_DIR}/__invoke_temp.cmake")
		file(WRITE "${__invoke_temp_file}" "find_${LIB}_build(${LIB_SRC} ${LIB_BUILD})")
		include (${__invoke_temp_file})
	else()
		FIND_LIB_BUILD_DEFAULT(${LIB} ${LIB_SRC} ${LIB_BUILD})
	endif()
endmacro()

# call function PREPARE_${LIB} by name
function(prepare_lib LIB LIB_SRC)
	if (COMMAND prepare_${LIB})
		set(__invoke_prepare_file "${CMAKE_CURRENT_BINARY_DIR}/__prepare_temp.cmake")
		file(WRITE "${__invoke_prepare_file}" "prepare_${LIB}(${LIB_SRC})")
		include(${__invoke_prepare_file})
	endif()
endfunction()

# call function PREPARE_${LIB} by name
macro(check_lib_src LIB IS_FOUND HINT)
	if (COMMAND check_${LIB}_src)
		set(__invoke_check_file "${CMAKE_CURRENT_BINARY_DIR}/__check_temp.cmake")
		file(WRITE "${__invoke_check_file}" "check_${LIB}_src(${IS_FOUND} ${HINT})")
		include(${__invoke_check_file})
	endif()
endmacro()

# check if all found and return. That is intentially macro to write in caller's scope
macro(RETURN_IF_FOUND LIB LEGEND)
	if (${LIB}_FOUND)
		set(${LIB}_INCLUDE_DIRS "${${LIB}_INCLUDE_DIRS}" PARENT_SCOPE)
		set(${LIB}_LIBRARIES "${${LIB}_LIBRARIES}" PARENT_SCOPE)
		set(${LIB}_FOUND TRUE PARENT_SCOPE)
		diags("${LIB} ${LEGEND} ${${LIB}_INCLUDE_DIRS} ${${LIB}_LIBRARIES}")
		return()
	endif ()
endmacro()

macro(CUSTOM_FIND_LIB LIB)
	if (COMMAND custom_find_${LIB})
		set(__invoke_find_file "${CMAKE_CURRENT_BINARY_DIR}/__find_temp.cmake")
		file(WRITE "${__invoke_find_file}" "custom_find_${LIB}()")
		include(${__invoke_find_file})
	else()
		find_package (${LIB} QUIET)
	endif ()
endmacro()

# main ring that rules them all
# uses up to 4 callbacks (all are optional)
#   custom_find_LIB() - to be used instead of find_package (LIB)
#   check_LIB_src(RESULT) - to check if we have simple embedded sources, and point LIB_SRC and LIB_BUILD into it
#   prepare_LIB(SRC) - to finalize legacy unpacked sources (patch, add cmakelist, etc.)
#   find_LIB_build(SRC BIN) - to check if lib in SRC and BIN is suitable, and set LIB_INCLUDE_DIRSC,
#		LIB_LIBRARIES, and LIB_FOUND, if so.
function(PROVIDE LIB LIB_GITHUB LIB_TARBALL)
	string(TOLOWER "${LIB}" SLIB)

	# if it is allowed to use system library - try to use it
	if ( NOT WITH_${LIB}_FORCE_STATIC )
		custom_find_lib ( ${LIB} )
		return_if_found(${LIB} "found as default (sys or other) dir")
	endif()

	# get source tarball, if WRITEB is in action
	populate (LIB_PLACE ${LIB} "${LIBS_BUNDLE}/${LIB_TARBALL}" "${LIB_GITHUB}")
	get_buildd(LIB_BUILD ${SLIB})
	get_srcpath(LIB_SRC ${SLIB})

	# check for cached lib
	if (HAVE_BBUILD)
		find_lib_build (${LIB} ${LIB_SRC} ${LIB_BUILD})
		return_if_found(${LIB} "found in cache")
	endif()

	# check in-tree sources
	check_lib_src(${LIB} IN_SRC EMBEDDED)
	if (IN_SRC)
		set(LIB_SRC ${${LIB}_SRC})
		set(LIB_BUILD ${${LIB}_BUILD})
	else()
		# check if already populated
		check_lib_src(${LIB} HAVE_SRC ${LIB_SRC})
		if (HAVE_SRC)
			diags ("${LIB} found ready source ${LIB_SRC}, will use it")
		else()
			diags("${LIB} wasn't found in-source, will fetch it")
			# provide sources
			fetch_and_unpack(${LIB} ${LIB_PLACE} ${LIB_SRC})
			prepare_lib(${LIB} ${LIB_SRC})
		endif()
	endif()

	set(${LIB}_SRC "${LIB_SRC}" PARENT_SCOPE)
	set(${LIB}_BUILD "${LIB_BUILD}" PARENT_SCOPE)
	set(${LIB}_FROMSOURCES TRUE PARENT_SCOPE)
endfunction()


