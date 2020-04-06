#=============================================================================
# Copyright 2019-2020, Manticore Software LTD (http://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# This file need to get boost context library.
# usually that is trivial, but on some ancient systems needs massage
#
# We perform following steps, each next performed if previous failed.
#
# 1. Check system boost, need at least 1.61.0.
# 2. Check if previous build in boost-context-build exists (and use, if so)
# 3. Check if exists bundle/build/boost-context-<arch>, like boost-context-linux-x86_64
# 4. Check if exists bundle/boost-<arch>.tar.gz (built full boost artefact)
# 5. Check if exists bundle/boost-context-<arch>.tar.gz (built boost context artefact)
# all the steps above assume you have pre-built boost or boost-context
# If nothing prebuilt, try to fetch the sources from:
# 6. Check if exists bundle/boost-context-src.tar.gz, unpack, build
# 7. Check if exists bundle/boost-src.tar.gz, unpack, build
# 8. use github https://github.com/boostorg/boost.git, tag boost-1.71.0
# After sources fetched, it will be built in config time.
#
# env WRITEB tells that we MAY write to bundle (and so, refresh the files there).
# that is os env, i.e. need to be called like WRITEB=1 cmake ...
#
# If bundle update is enabled (WRITEB=1 env is set):
# 9b. pack sources into bundle/boost-context-src.tag.gz, excluding '.git' folder.
# That is done only if source was cloned from git (no need to re-pack from tarball).
# 10. build into bundle/build/boost-context-<arch>
# 11. pack build into bundle/boost-context-<arch>.tar.gz
#
# So, configuring with empty bundle 'from scratch', having WRITEB=1 will leave you
# artefacts:
#	bundle/boost-context-src.tar.gz			- original sources
#	bundle/boost-context-<arch>.tar.gz		- packet built binaries for your platform
#
# Following rebuilds will immediately reuse these artefacts without need to rebuild.
#

FOREACH (policy CMP0054)
	IF (POLICY ${policy})
		CMAKE_POLICY(SET ${policy} NEW)
	ENDIF ()
ENDFOREACH ()

# special to keep artifacts
include (update_bundle)

function ( FIND_CUSTOM_CONTEXT LEGEND BOOST_ROOT )
	message(STATUS "${LEGEND} ${BOOST_ROOT}")
	unset(Boost_CONTEXT_FOUND)
	unset(Boost_CONTEXT_LIBRARY CACHE)
	unset(Boost_FOUND)
	set(Boost_NO_SYSTEM_PATHS ON)
	find_package(Boost 1.61.0 COMPONENTS context QUIET)
	set(Boost_CONTEXT_FOUND "${Boost_CONTEXT_FOUND}" PARENT_SCOPE)
	set(Boost_VERSION "${Boost_VERSION}" PARENT_SCOPE)
endfunction()

function (FINALIZE_BOOST_CONTEXT LEGEND BOOSTROOT)
	list(APPEND EXTRA_LIBRARIES Boost::context)
	set(EXTRA_LIBRARIES "${EXTRA_LIBRARIES}" PARENT_SCOPE)
	diags("=== ${LEGEND} ${Boost_VERSION} ${Boost_CONTEXT_LIBRARY} ${BOOSTROOT}")
	if (BOOSTROOT)
		set (BOOST_ROOT "${BOOSTROOT}" PARENT_SCOPE)
	endif()
endfunction()

function(CONFIGURE_BOOST WORKDIR)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
			RESULT_VARIABLE bresult
			WORKING_DIRECTORY ${WORKDIR})
	if (bresult)
		message(FATAL_ERROR "CMake step for boost-context failed")
	endif ()
endfunction()

function(DOWNLOAD_BOOST WORKDIR)
	message(STATUS "Download boost ${WORKDIR}")
	diag(bresult)
	execute_process(COMMAND ${CMAKE_COMMAND} --build . --target boost-context-download
			RESULT_VARIABLE bresult
			WORKING_DIRECTORY "${WORKDIR}")
	if (bresult)
		message(FATAL_ERROR "Download step for boost-context failed")
	endif ()
endfunction()

function(PACK_BOOST_SRC BOOST_SRC BOOST_NAME)
	message(STATUS "Pack boost sources ${BOOST_SRC} into ${BOOST_NAME}")
	set(TARNAME "${BOOST_NAME}-src")
	set(PACKDIR "${CACHEB}/${TARNAME}")
	file(REMOVE_RECURSE ${PACKDIR})
	file(COPY ${BOOST_SRC}/ DESTINATION ${PACKDIR} PATTERN ".git" EXCLUDE)
	execute_process(
			COMMAND "${CMAKE_COMMAND}" -E tar cfz "${LIBS_BUNDLE}/${TARNAME}.tar.gz" "${TARNAME}"
			WORKING_DIRECTORY "${CACHEB}")
endfunction()

function(PACK_BOOST_BUILD BOOST_BUILD)
	message(STATUS "Save built boost ${BOOST_BUILD}")
	get_filename_component(BUILDPATH ${BOOST_BUILD} PATH)
	get_filename_component(BUILDNAME ${BOOST_BUILD} NAME)
	execute_process(
			COMMAND "${CMAKE_COMMAND}" -E tar cfz "${LIBS_BUNDLE}/${BUILDNAME}.tar.gz" "${BUILDNAME}"
			WORKING_DIRECTORY "${BUILDPATH}")
endfunction()

function(BUILD_BOOST WORKDIR)
	# clear any CXX since b2 build use it's own introspection
	if (DEFINED ENV{CXX})
		set(STORE_CXX "$ENV{CXX}")
		unset(ENV{CXX})
	endif ()
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
			RESULT_VARIABLE bresult
			WORKING_DIRECTORY ${WORKDIR})
	if (bresult)
		message(FATAL_ERROR "Build step for boost-context failed")
	endif ()

	if (STORE_CXX)
		set(ENV{CXX} "${STORE_CXX}")
	endif ()
endfunction()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

#if (FALSE) # for debug purposes

find_package(Boost 1.61.0 COMPONENTS context)

#endif (FALSE) # for debug purposes

if ( Boost_CONTEXT_FOUND )
	finalize_boost_context("Found in system" FALSE)
	return()
endif()

# Check previously build and not yet cleaned
FIND_CUSTOM_CONTEXT("Try from previous run" "${MANTICORE_BINARY_DIR}/boost-context-build")
if (Boost_CONTEXT_FOUND)
	finalize_boost_context("Was build before" "${MANTICORE_BINARY_DIR}/boost-context-build")
	return()
endif ()


# Check in artifacts folder bundle/build for pre-build boost as boost-context-linux-x86_64
get_build(BOOST_BUILD boost)
FIND_CUSTOM_CONTEXT("Try pre-built full boost from artifacts" "${BOOST_BUILD}")
if (Boost_CONTEXT_FOUND)
	finalize_boost_context("Prebuilt from cache artifacts" "${BOOST_BUILD}")
	return()
endif ()

# Check in artifacts folder bundle/build for pre-build full boost as boost-linux-x86_64
get_build(BOOST_CONTEXT_BUILD boost-context)
FIND_CUSTOM_CONTEXT("Try pre-built boost from artifacts" "${BOOST_CONTEXT_BUILD}")
if (Boost_CONTEXT_FOUND)
	finalize_boost_context("Prebuilt from cache artifacts" "${BOOST_CONTEXT_BUILD}")
	return()
endif ()

# check if bundled pre-built bundle/boost-<arch>.tar.gz (that is full BOOST, not just boost.context)
get_build(BOOST_FULL_BUILD boost)
get_platformed_name(BOOST_FULL_BUILD_F "boost")
set(BOOST_FULL_BUNDLEZIP "${LIBS_BUNDLE}/${BOOST_FULL_BUILD_F}.tar.gz")
diag(BOOST_FULL_BUNDLEZIP)
diag(BOOST_FULL_BUILD_F)
if ( EXISTS ${BOOST_FULL_BUNDLEZIP} )
	fetch_and_unpack (boost-full ${BOOST_FULL_BUNDLEZIP} ${BOOST_FULL_BUILD})
	FIND_CUSTOM_CONTEXT("Packed prebuild full boost" "${BOOST_FULL_BUILD}")
	if (Boost_CONTEXT_FOUND)
		finalize_boost_context("Prebuilt full Boost from archive" "${BOOST_FULL_BUILD}")
		return()
	endif ()
endif()


# check if bundled pre-built bundle/boost-context-<arch>.tar.gz (that is just boost.context)
get_build(BOOST_CONTEXT_BUILD boost-context)
get_platformed_name(BOOST_CONTEXT_BUILD_F "boost-context")
set(BOOST_CONTEXT_BUNDLEZIP "${LIBS_BUNDLE}/${BOOST_CONTEXT_BUILD_F}.tar.gz")
diag(BOOST_CONTEXT_BUNDLEZIP)
diag(BOOST_CONTEXT_BUILD_F)
if (EXISTS ${BOOST_CONTEXT_BUNDLEZIP})
	fetch_and_unpack(boost-context ${BOOST_CONTEXT_BUNDLEZIP} ${BOOST_CONTEXT_BUILD})
	FIND_CUSTOM_CONTEXT("Packed prebuild context" ${BOOST_CONTEXT_BUILD})
	if (Boost_CONTEXT_FOUND)
		finalize_boost_context("Prebuilt Boost context from archive" "${BOOST_CONTEXT_BUILD}")
		return()
	endif ()
endif ()

# All easy (pre-built) ways are over. Will try to configure the build from sources now

# check specific boost-context sources from boost-context-src.tar.gz in bundle
set(BOOST_CONTEXT_SRC "${CMAKE_BINARY_DIR}/boost-context-src")

# try to unpack bundle from boost-context-src.tar.gz (boost context only) or boost-src.tar.gz (full boost) sources
if (NOT EXISTS "${BOOST_CONTEXT_SRC}/README.md")
	if (EXISTS "${LIBS_BUNDLE}/boost-context-src.tar.gz")
		fetch_and_unpack(boost-context-src "${LIBS_BUNDLE}/boost-context-src.tar.gz" "${BOOST_CONTEXT_SRC}")
	elseif (EXISTS ${LIBS_BUNDLE}/boost-src.tar.gz)
		set(BOOST_CONTEXT_SRC "${CMAKE_BINARY_DIR}/boost-src")
		get_build(BOOST_CONTEXT_BUILD boost)
		fetch_and_unpack(boost-full-src "${LIBS_BUNDLE}/boost-src.tar.gz" "${BOOST_CONTEXT_SRC}")
	endif()
endif()

set(BOOST_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/boost-context-download")

if (WIN32)
	set (WIN_SUFF "-win")
endif()

# on-board sources available, continue preparation for just-build
if (EXISTS "${BOOST_CONTEXT_SRC}/README.md")
	configure_file("${MANTICORE_MODULE_PATH}/boost-context${WIN_SUFF}-local.cmake.in"
			${BOOST_DOWNLOAD_DIR}/CMakeLists.txt @ONLY)
	configure_boost("${BOOST_DOWNLOAD_DIR}")
	build_boost("${BOOST_DOWNLOAD_DIR}")
	FIND_CUSTOM_CONTEXT("From sources" "${BOOST_CONTEXT_BUILD}")
	if (Boost_CONTEXT_FOUND)
		finalize_boost_context("Prebuilt from tarball or source dir" "${BOOST_CONTEXT_BUILD}")
		return()
	endif ()
endif()


file(REMOVE_RECURSE ${BOOST_CONTEXT_SRC})

# no on-board sources, will fetch from git
get_build(BOOST_CONTEXT_BUILD boost)

configure_file(${MANTICORE_MODULE_PATH}/boost${WIN_SUFF}-git.cmake.in
		${BOOST_DOWNLOAD_DIR}/CMakeLists.txt @ONLY)
configure_boost(${BOOST_DOWNLOAD_DIR})
download_boost("${BOOST_DOWNLOAD_DIR}")
if (WRITEB)
	pack_boost_src(${BOOST_CONTEXT_SRC} boost)
endif()
build_boost("${BOOST_DOWNLOAD_DIR}")
FIND_CUSTOM_CONTEXT("From fresh github sources" "${BOOST_CONTEXT_BUILD}")
if (Boost_CONTEXT_FOUND)
	finalize_boost_context("Prebuilt from github sources" "${BOOST_CONTEXT_BUILD}")
	if (WRITEB)
		pack_boost_build(${BOOST_CONTEXT_BUILD})
	endif()
	return()
endif ()

message (FATAL_ERROR "Boost context is not found")