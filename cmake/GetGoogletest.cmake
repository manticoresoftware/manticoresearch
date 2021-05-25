# Search prebuilt googletest. If nothing found - download, unpack, build and then search again
# allow to get googletest from bundle also (instead of download)

set(GTEST_GITHUB "https://github.com/google/googletest/archive/master.zip")
set(GTEST_ZIP "googletest-master.zip") # that is default filename if you download GTEST_GITHUB using browser
set(GTEST_URL "${LIBS_BUNDLE}/${GTEST_ZIP}")

if (TARGET GTest::gmock_main)
	return()
endif ()

include(update_bundle)

# set global cache path to cmake
get_cache(CACHE_BUILDS)
set(CMAKE_PREFIX_PATH "${CACHE_BUILDS}")

# check pre-built gtests
find_package(GTest QUIET CONFIG)
if (TARGET GTest::gmock_main)
	return()
endif()

# not found. Populate and build cache package for now and future usage.
MESSAGE(STATUS "prebuilt googletest wasn't found. Will build it right now...")
populate(GTEST_PLACE "gtests" ${GTEST_URL} ${GTEST_GITHUB})

# build as external project and install into cache
get_build(GTEST_BUILD gtest)
configure_file(${MANTICORE_SOURCE_DIR}/cmake/gtest-imported.cmake.in "${MANTICORE_BINARY_DIR}/gtestbuild/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/gtestbuild")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/gtestbuild")

# now it should find
find_package(GTest CONFIG)
diagp(GTest::gmock_main)