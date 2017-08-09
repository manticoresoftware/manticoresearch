# Download and unpack googletest at configure time

# allow to get googletest from bundle also

if ( LIBS_BUNDLE AND EXISTS "${LIBS_BUNDLE}/googletestmaster.zip" )
	set ( GTEST_URL "${LIBS_BUNDLE}/googletestmaster.zip" )
	message ( STATUS "Use google-tests from ${LIBS_BUNDLE}/googletestmaster.zip" )
else()
	set ( GTEST_URL "https://github.com/google/googletest/archive/master.zip" )
	message ( STATUS "Use google-tests from github https://github.com/google/googletest/archive/master.zip" )
	message ( STATUS "(you may download the file and place it as 'googletestmaster.zip' into ${LIBS_BUNDLE}/ " )
endif()

configure_file ( ${CMAKE_MODULE_PATH}/googletest-download.cmake.in
		${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt )
execute_process ( COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
		RESULT_VARIABLE gresult
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if ( gresult )
	message ( FATAL_ERROR "CMake step for googletest failed: ${gresult}" )
endif ()
execute_process ( COMMAND ${CMAKE_COMMAND} --build .
		RESULT_VARIABLE gresult
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if ( gresult )
	message ( FATAL_ERROR "Build step for googletest failed: ${gresult}" )
endif ()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set ( gtest_force_shared_crt ON CACHE BOOL "" FORCE )

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory ( ${CMAKE_BINARY_DIR}/googletest-src
		${CMAKE_BINARY_DIR}/googletest-build )

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if ( CMAKE_VERSION VERSION_LESS 2.8.11 )
	include_directories ( "${gtest_SOURCE_DIR}/include" )
endif ()

# we don't want google test/mock build artifacts at all.
set ( INSTALL_GTEST OFF CACHE BOOL "Install gtest" FORCE )
set ( INSTALL_GMOCK OFF CACHE BOOL "Install gmock" FORCE )

set ( HAVE_GOOGLETEST TRUE )