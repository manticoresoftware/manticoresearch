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
# This file is to find boost context library.
# That is trivial, but also provides kind of diagnostic, so located as aside to keep main project definitions shorter.
#
# On most systems defaultly installed boost should work. Or, you may provide 'BOOST_ROOT' env to point the place (for
# example, on system env, or via CI scripts).
#
# Since ancient builds also has ancient ver of boost, we build it from sources there.
# Build rules are found in dockers, see examples in dist/build_dockers/SYS/boost_1XX.
#

if (TARGET Boost::context)
	return()
endif ()

# that is to search using BOOST_ROOT
if (POLICY CMP0074)
	CMAKE_POLICY(SET CMP0074 NEW)
endif ()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)

# that line is because we have /MT instead of /MD in linker flags... (if we update - don't forget to remove it!)
if (MSVC)
	set(Boost_USE_STATIC_RUNTIME ON)
endif ()

# diagnostic:
#set(Boost_DEBUG ON)
#set(Boost_DETAILED_FAILURE_MSG ON)

find_package(Boost 1.61.0 REQUIRED COMPONENTS context)

return() # comment this line to have extra info about boost, for diag

include("CMakePrintHelpers")
cmake_print_properties(TARGETS Boost::context PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES
		INTERFACE_COMPILE_DEFINITIONS
		IMPORTED_CONFIGURATIONS
		INTERFACE_LINK_LIBRARIES
		IMPORTED_LOCATION_DEBUG
		IMPORTED_LOCATION_RELEASE
		IMPORTED_LOCATION_RELWITHDEBINFO
		IMPORTED_LOCATION_MINSIZEREL
		IMPORTED_LOCATION
		LOCATION
		)

cmake_print_properties(TARGETS Boost::disable_autolinking PROPERTIES
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
