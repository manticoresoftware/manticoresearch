cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

set ( XXH_REPO "https://github.com/manticoresoftware/xxHash" )
set ( XXH_REV "cb21910" )
set ( XXH_SRC_MD5 "b82466bd7156e407d1416d6c1c5d5821" )

set ( XXH_GITHUB "${XXH_REPO}/archive/${XXH_REV}.zip" )
set ( XXH_BUNDLE "${LIBS_BUNDLE}/xxHash-${XXH_REV}.zip" )

include ( update_bundle )

# determine destination folder where we expect pre-built xxhash
find_package ( xxHash QUIET CONFIG )
return_if_target_found ( xxHash::xxHash "found ready (no need to build)" )

# not found. Populate and prepare sources
select_nearest_url ( XXH_PLACE xxHash ${XXH_BUNDLE} ${XXH_GITHUB} )
fetch_and_check ( xxHash ${XXH_PLACE} ${XXH_SRC_MD5} XXH_SRC )

# build external project
get_build ( XXH_BUILD xxHash )
external_build ( xxHash XXH_SRC XXH_BUILD )

# now it should find
find_package ( xxHash REQUIRED CONFIG )
return_if_target_found ( xxHash::xxHash "was built and saved" )
