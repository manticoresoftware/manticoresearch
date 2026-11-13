# Search prebuilt googletest. If nothing found - download, unpack, build and then search again
# allow to get googletest from bundle also (instead of download)

# Pin the fallback to v1.16.0; main requires VS 2022, unlike our Windows cross toolchain.
set ( GTEST_GITHUB "https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip" )
set ( GTEST_ZIP "googletest-1.16.0.zip" )
set ( GTEST_URL "${LIBS_BUNDLE}/${GTEST_ZIP}" )

if (TARGET GTest::gmock_main)
	return ()
endif ()

include ( update_bundle )

# check pre-built gtests
find_package ( GTest QUIET CONFIG )
if (TARGET GTest::gmock_main)
	return ()
endif ()

# not found. Populate and build cache package for now and future usage.
MESSAGE ( STATUS "prebuilt googletest wasn't found. Will build it right now..." )
select_nearest_url ( GTEST_PLACE "gtests" ${GTEST_URL} ${GTEST_GITHUB} )

# build as external project and install into cache
get_build ( GTEST_BUILD gtest )
configure_file ( ${MANTICORE_SOURCE_DIR}/cmake/gtest-imported.cmake.in "${MANTICORE_BINARY_DIR}/gtestbuild/CMakeLists.txt" )
execute_process ( COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/gtestbuild" )
execute_process ( COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/gtestbuild" )

# now it should find
find_package ( GTest CONFIG )
trace ( GTest::gmock_main )

