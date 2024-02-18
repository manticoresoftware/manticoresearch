cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

set ( CCTZ_REPO "https://github.com/manticoresoftware/cctz" )
set ( CCTZ_REV "1a861bb" )
set ( CCTZ_SRC_MD5 "8a8831e2595aef2e95f1a18498b3d600" )

set ( CCTZ_GITHUB "${CCTZ_REPO}/archive/${CCTZ_REV}.zip" )
set ( CCTZ_BUNDLE "${LIBS_BUNDLE}/cctz-${CCTZ_REV}.zip" )

include ( update_bundle )

# try to find quietly (will work most time
find_package ( cctz QUIET CONFIG )
return_if_target_found ( cctz::cctz "found ready (no need to build)" )

# not found. Populate and prepare sources
select_nearest_url ( CCTZ_PLACE cctz ${CCTZ_BUNDLE} ${CCTZ_GITHUB} )
fetch_and_check ( cctz ${CCTZ_PLACE} ${CCTZ_SRC_MD5} CCTZ_SRC )

# build external project
get_build ( CCTZ_BUILD cctz )
set ( CCTZ_BUILD_TESTING OFF CACHE BOOL "" FORCE )
set ( CCTZ_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE )
set ( CCTZ_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE )
external_build ( cctz CCTZ_SRC CCTZ_BUILD CCTZ_BUILD_TOOLS=0 )

# now it should find
find_package ( cctz REQUIRED CONFIG )
return_if_target_found ( cctz::cctz "was built and saved" )
