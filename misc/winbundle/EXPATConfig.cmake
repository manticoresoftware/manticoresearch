if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
	message (FATAL_ERROR "CMake >= 2.6.0 required")
endif ()
cmake_policy (PUSH)
cmake_policy (VERSION 2.6...3.18)

# Compute the installation prefix relative to this file.
get_filename_component (_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
if (CMAKE_EXE_LINKER_FLAGS MATCHES "x64")
	set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/Expat-1.95.8-x64")
else()
	set (_IMPORT_PREFIX "${_IMPORT_PREFIX}/Expat-1.95.8")
endif ()

# Create imported target EXPAT::EXPAT
add_library (EXPAT::EXPAT SHARED IMPORTED)
set_target_properties (EXPAT::EXPAT PROPERTIES
		IMPORTED_LINK_INTERFACE_LANGUAGES "C"
		INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/Source/lib"
		IMPORTED_LOCATION "${_IMPORT_PREFIX}/Libs/libexpat.dll"
		IMPORTED_IMPLIB "${_IMPORT_PREFIX}/Libs/libexpat.lib"
		)

if (CMAKE_VERSION VERSION_LESS 3.1.0)
	message (FATAL_ERROR "This file relies on consumers using CMake 3.1.0 or greater.")
endif ()

# Cleanup temporary variables.
set (_IMPORT_PREFIX)
cmake_policy (POP)
