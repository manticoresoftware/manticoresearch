cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

if (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL Windows)
	message (STATUS "will NOT do cross-compile OpenSSL for windows")
	return()
endif()

if (NOT DEFINED ENV{VCPKG_ROOT})
	message ( STATUS "VCPKG_ROOT is not defined, bail" )
	return()
endif()

# set path despite the fact if we have vcpkg or not (we may have ready prepared data from previous run)
set ( _VCARCH "arm64" )
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
	set ( _VCARCH "x64" )
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL x64)
	set ( _VCARCH "x64" )
endif ()
set ( _TARGET "linux" )
if (${CMAKE_SYSTEM_NAME} STREQUAL windows)
	set ( _TARGET "windows" )
elseif (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
	set ( _TARGET "osx" )
elseif (${CMAKE_SYSTEM_NAME} STREQUAL FreeBSD)
	set ( _TARGET "freebsd" )
endif ()
set ( _VCACHE "$ENV{VCPKG_ROOT}/installed/${_VCARCH}-${_TARGET}" )
prepend_prefix ( ${_VCACHE} )

find_program ( VCPKG vcpkg HINTS $ENV{VCPKG_ROOT} )
if (NOT VCPKG)
	message ( STATUS "vcpkg is not found, bail" )
	return ()
endif ()

set (vcpkg_params "") # will cause default system triplet to be used
if (CMAKE_CROSSCOMPILING)
	set (vcpkg_params "--triplet=${_VCARCH}-${_TARGET}")
endif()

set ( ENV{MANTICORE_SOURCE_DIR} "${MANTICORE_SOURCE_DIR}" )
set ( ENV{VCPKG_DEFAULT_BINARY_CACHE} "${CACHEB}/vcpkg" )
set ( ENV{VCPKG_OVERLAY_TRIPLETS} "${MANTICORE_SOURCE_DIR}/dist/build_dockers/cross/triplets" )

message ( STATUS "VCPKG_DEFAULT_BINARY_CACHE is $ENV{VCPKG_DEFAULT_BINARY_CACHE}" )
message ( STATUS "VCPKG_OVERLAY_TRIPLETS is $ENV{VCPKG_OVERLAY_TRIPLETS}" )
if (NOT EXISTS ENV{VCPKG_DEFAULT_BINARY_CACHE})
	file ( MAKE_DIRECTORY "$ENV{VCPKG_DEFAULT_BINARY_CACHE}" )
endif ()

# oneliner to achieve same in bash (from the root of the sources):
# export MANTICORE_SOURCE_DIR=$(pwd); export VCPKG_DEFAULT_BINARY_CACHE=$MANTICORE_SOURCE_DIR/cache/vcpkg; export VCPKG_OVERLAY_TRIPLETS=$MANTICORE_SOURCE_DIR/dist/build_dockers/cross/triplets

message ( STATUS "executing vcpkg install openssl..." )
execute_process (
		COMMAND "${VCPKG}" install ${vcpkg_params} openssl
		RESULT_VARIABLE OPENSSL_INSTALL_RESULT
)
message (STATUS "exitcode is ${OPENSSL_INSTALL_RESULT}")