# Search prebuilt googlebench. If nothing found - download, unpack, build and then search again
# allow to get googlebench from bundle also (instead of download)

set(GBENCH_GITHUB "https://github.com/google/benchmark/archive/main.zip")
set(GBENCH_ZIP "benchmark-main.zip") # that is default filename if you download GBENCH_GITHUB using browser
set(GBENCH_URL "${LIBS_BUNDLE}/${GBENCH_ZIP}")

if (TARGET benchmark::benchmark_main )
	return()
endif ()

macro(fixup_benches_and_return_if_found)
	if (TARGET benchmark::benchmark)
		set_target_properties(benchmark::benchmark PROPERTIES
				MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
				MAP_IMPORTED_CONFIG_MINSIZEREL Release
				MAP_IMPORTED_CONFIG_DEBUG Release
				)
	endif ()

	if (TARGET benchmark::benchmark_main)
		set_target_properties(benchmark::benchmark_main PROPERTIES
				MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
				MAP_IMPORTED_CONFIG_MINSIZEREL Release
				MAP_IMPORTED_CONFIG_DEBUG Release
				)
	endif ()

	if (TARGET benchmark::benchmark)
		return()
	endif ()
endmacro()

include(update_bundle)

# check pre-built gbenches
find_package(benchmark QUIET CONFIG)
fixup_benches_and_return_if_found()

# not found. Populate and build cache package for now and future usage.
MESSAGE(STATUS "prebuilt googlebench wasn't found. Will build it right now...")
select_nearest_url(GBENCH_PLACE "gbench" ${GBENCH_URL} ${GBENCH_GITHUB})

# build as external project and install into cache
get_build(GBENCH_BUILD benchmark)
configure_file(${MANTICORE_SOURCE_DIR}/cmake/gbench-imported.cmake.in "${MANTICORE_BINARY_DIR}/gbench-build/CMakeLists.txt")
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/gbench-build")
execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/gbench-build")

# now it should find
find_package(benchmark CONFIG)

fixup_benches_and_return_if_found()
