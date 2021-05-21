# Search prebuilt googletest. If nothing found - download, unpack, build and then search again
# allow to get googletest from bundle also (instead of download)

set(GTEST_GITHUB "https://github.com/google/googletest/archive/master.zip")
set(GTEST_ZIP "googletest-master.zip") # that is default filename if you download GTEST_GITHUB using browser
set(GTEST_URL "${LIBS_BUNDLE}/${GTEST_ZIP}")

if (TARGET GTest::gmock_main)
	return()
endif ()

include(update_bundle)

# check pre-built gtests
get_build(GTEST_BUILD gtest)
find_package(GTest CONFIG PATHS "${GTEST_BUILD}")
#diagp (GTest::gmock_main)

if (TARGET GTest::gmock_main)
	return()
endif()

# not found. Populate and build cache package for now and future usage.
MESSAGE(STATUS "prebuilt googletest wasn't found. Will build it right now...")
get_srcpath(GTEST_SRC googletest)
populate(GTEST_PLACE "gtests" ${GTEST_URL} ${GTEST_GITHUB})
if (NOT EXISTS "${GTEST_SRC}/README.md")
	fetch_and_unpack("gtests" ${GTEST_PLACE} ${GTEST_SRC})
endif()

# build as external project and install into cache
set(GTEST_EXTERNAL_DIR "${CMAKE_BINARY_DIR}/googletest-external")
configure_file(${MANTICORE_SOURCE_DIR}/cmake/gtest-imported.cmake.in ${GTEST_EXTERNAL_DIR}/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${GTEST_EXTERNAL_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${GTEST_EXTERNAL_DIR})

# now it should find
find_package(GTest CONFIG PATHS "${GTEST_BUILD}")
diagp(GTest::gmock_main)