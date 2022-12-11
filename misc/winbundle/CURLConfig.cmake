# wget https://curl.se/windows/dl-7.85.0_9/curl-7.85.0_9-win64-mingw.zip

# you might need to write correct version here...
set (CURL_VERSION_STRING "7.85.0")
set (_CURL_VER "${CURL_VERSION_STRING}_9")

if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
	message (FATAL_ERROR "CMake >= 2.6.0 required")
endif ()
cmake_policy (PUSH)
cmake_policy (VERSION 2.6...3.18)

# Compute the installation prefix relative to this file.
get_filename_component (_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
if (CMAKE_EXE_LINKER_FLAGS MATCHES "x64")
	set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/curl-${_CURL_VER}-win64-mingw")
else()
	message (FATAL_ERROR "Only x64 arch supported for CURL")
endif ()

# Create imported target ZSTD::ZSTD
add_library ( CURL::libcurl SHARED IMPORTED)
set_target_properties ( CURL::libcurl PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
		IMPORTED_IMPLIB "${_IMPORT_PREFIX}/lib/libcurl.dll.a"
		IMPORTED_LOCATION "${_IMPORT_PREFIX}/bin/libcurl-x64.dll"
		)

if (CMAKE_VERSION VERSION_LESS 3.1.0)
	message (FATAL_ERROR "This file relies on consumers using CMake 3.1.0 or greater.")
endif ()

# Cleanup temporary variables.
set (_IMPORT_PREFIX)
cmake_policy (POP)
