if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
	message (FATAL_ERROR "CMake >= 2.6.0 required")
endif ()
cmake_policy (PUSH)
cmake_policy (VERSION 2.6...3.18)

# Compute the installation prefix relative to this file.
get_filename_component (_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
if (CMAKE_EXE_LINKER_FLAGS MATCHES "x64")
	set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/mysql-x64")
else()
	set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/mysql")
endif ()

if (MYSQL_USE_STATIC_LIBS)

	# Create imported target Mysql::Mysql as static
	add_library (Mysql::Mysql STATIC IMPORTED)
	set_target_properties (Mysql::Mysql PROPERTIES
			IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opt/mysqlclient.lib"
			IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/debug/mysqlclient.lib"
			)
else()

	# Create imported target Mysql::Mysql as shared
	add_library (Mysql::Mysql SHARED IMPORTED)
	set_target_properties (Mysql::Mysql PROPERTIES
			IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/opt/libmysql.lib"
			IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/debug/libmysql.lib"
			IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/opt/libmysql.dll"
			IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/debug/libmysql.dll"
			)
endif()

set_target_properties (Mysql::Mysql PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
		IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
		IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
		IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
		MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
		MAP_IMPORTED_CONFIG_MINSIZEREL Release
		)

if (CMAKE_VERSION VERSION_LESS 3.1.0)
	message (FATAL_ERROR "This file relies on consumers using CMake 3.1.0 or greater.")
endif ()

# Cleanup temporary variables.
set (_IMPORT_PREFIX)
cmake_policy (POP)
