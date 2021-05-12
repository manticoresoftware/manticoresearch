#=============================================================================
# Copyright 2019-2021, Manticore Software LTD (https://manticoresearch.com)
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
# On Windows - find system boost. If you have default installation, which cames to c:\local\boost_X_Y_Z - it will be found.
# If you've installed boost in custom location, as 'x:\boost_1_72_0' - consider to provide env BOOST_ROOT with this path.
# Most transparent way is to define new user or system env variable in computer's preferences named 'BOOST_ROOT' with the
# path to boost installation.
#
# On other systems (non-windows) we also find system boost. But since ancient ones has also ancient ver of boost, we build
# from sources and install. Build rules are found in dockers, see dist/build_dockers/SYS/boost_172.
#
# Caveats:
#
# Since Boost 1.72 has a bug which cause Boost::context being defined even if it is not available, and then reject further
# searching, we try to determine whether installed boost need Boost_USE_STATIC_RUNTIME to be set or not. Default installation
# doesn't need the flag, and we're moving to always use default. But unfortunately our current builds uses the flag, so
# it is viable to make a test to find if it is necessary. For such test we make aside project with boost without
# Boost_USE_STATIC_RUNTIME and try to configure it. Resulting code is used then as value for Boost_USE_STATIC_RUNTIME.
#
# On Windows Boost_USE_STATIC_RUNTIME was necessary because of the bug in cmake scripts searching the lib: they didn't set
# compiler definition BOOST_ALL_NO_LIB, and that caused linker to always automatically link with static runtime variant of the
# boost. Now it is fixed by manually assigning the property.
#
# After a while we will rebuild all dockers to use default boost. Then this file will be shrinked to couple of definitions
# (Boost_USE_MULTITHREADED and Boost_USE_STATIC_LIBS) and single 'find_package' call. However runaround line with setting
# BOOST_ALL_NO_LIB definition on windows is also will be left here.
#

FOREACH (policy CMP0054 CMP0074)
	IF (POLICY ${policy})
		CMAKE_POLICY(SET ${policy} NEW)
	ENDIF ()
ENDFOREACH ()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)

# diagnostic:
#set(Boost_DEBUG ON)
#set(Boost_DETAILED_FAILURE_MSG ON)

if (NOT TARGET Boost::context)
	configure_file("${MANTICORE_MODULE_PATH}/TestBoostContext.cmake" "${CMAKE_CURRENT_BINARY_DIR}/testboost/CMakeLists.txt" @ONLY)
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
			RESULT_VARIABLE Boost_USE_STATIC_RUNTIME WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/testboost"
			OUTPUT_QUIET ERROR_QUIET)
	if (Boost_USE_STATIC_RUNTIME)
		message(STATUS "Non-default boost found, consider to use boost built without 'runtime-link=static' option")
	endif ()
	find_package(Boost 1.61.0 REQUIRED COMPONENTS context)
endif()

if (WIN32)
	set_property(TARGET Boost::context PROPERTY INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB" )
endif()

return() # comment this line to have extra info about boost, for diag

include("CMakePrintHelpers")
cmake_print_properties(TARGETS Boost::context PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES
		INTERFACE_COMPILE_DEFINITIONS
		IMPORTED_CONFIGURATIONS
		IMPORTED_LOCATION_DEBUG
		IMPORTED_LOCATION_RELEASE
		IMPORTED_LOCATION_RELWITHDEBINFO
		IMPORTED_LOCATION_MINSIZEREL
		IMPORTED_LOCATION
		LOCATION
		)
cmake_print_variables (
		Boost_VERSION
		Boost_INCLUDE_DIRS
		Boost_LIBRARY_DIRS
		Boost_LIBRARIES
		Boost_LIB_DIAGNOSTIC_DEFINITIONS
		Boost_CONTEXT_FOUND
		Boost_CONTEXT_LIBRARY
		Boost_CONTEXT_LIBRARY_DEBUG
		Boost_CONTEXT_LIBRARY_RELEASE
		Boost_NO_BOOST_CMAKE
		)
