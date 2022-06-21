if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
	message (FATAL_ERROR "CMake >= 2.6.0 required")
endif ()
cmake_policy (PUSH)
cmake_policy (VERSION 2.6...3.18)

# Compute the installation prefix relative to this file.
get_filename_component (_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/pgsql-x64")
else()
	set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/pgsql-x32")
endif ()

# Create imported target PostgreSQL::PostgreSQL
add_library (PostgreSQL::PostgreSQL SHARED IMPORTED)
set_target_properties (PostgreSQL::PostgreSQL PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
		INTERFACE_LINK_LIBRARIES "PostgreSQL::SSL;PostgreSQL::Crypto;PostgreSQL::Iconv;PostgreSQL::Intl"
		IMPORTED_IMPLIB "${_IMPORT_PREFIX}/lib/libpq.lib"
		IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/libpq.dll"
		)

# Dependencies - have to be installed together
add_library (PostgreSQL::SSL INTERFACE IMPORTED)
set_target_properties (PostgreSQL::SSL PROPERTIES LOCATION "${_IMPORT_PREFIX}/bin/ssleay32.dll")

add_library (PostgreSQL::Crypto INTERFACE IMPORTED)
set_target_properties (PostgreSQL::Crypto PROPERTIES LOCATION "${_IMPORT_PREFIX}/bin/libeay32.dll")

add_library (PostgreSQL::Iconv INTERFACE IMPORTED)
add_library (PostgreSQL::Intl INTERFACE IMPORTED)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set_target_properties (PostgreSQL::Iconv PROPERTIES LOCATION "${_IMPORT_PREFIX}/bin/libiconv-2.dll")
	set_target_properties (PostgreSQL::Intl PROPERTIES LOCATION "${_IMPORT_PREFIX}/bin/libintl-8.dll")
else()
	set_target_properties (PostgreSQL::Iconv PROPERTIES LOCATION "${_IMPORT_PREFIX}/bin/libiconv.dll")
	set_target_properties (PostgreSQL::Intl PROPERTIES LOCATION "${_IMPORT_PREFIX}/bin/libintl.dll")
endif()

if (CMAKE_VERSION VERSION_LESS 3.1.0)
	message (FATAL_ERROR "This file relies on consumers using CMake 3.1.0 or greater.")
endif ()

# Cleanup temporary variables.
set (_IMPORT_PREFIX)
cmake_policy (POP)
