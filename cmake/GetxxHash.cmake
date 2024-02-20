cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

set ( XXH_REPO "https://github.com/manticoresoftware/xxHash" )
set ( XXH_REV "f045522" )
set ( XXH_SRC_MD5 "1ba80412b217c5cdfe51352ac5427531" )

set ( XXH_GITHUB "${XXH_REPO}/archive/${XXH_REV}.zip" )
set ( XXH_BUNDLE "${LIBS_BUNDLE}/xxHash-${XXH_REV}.zip" )

include ( update_bundle )

# determine destination folder where we expect pre-built xxhash
find_package ( xxHash QUIET CONFIG )
return_if_target_found ( xxHash::xxhash "found ready (no need to build)" )

# not found. Populate and prepare sources
select_nearest_url ( XXH_PLACE xxhash ${XXH_BUNDLE} ${XXH_GITHUB} )
fetch_and_check ( xxhash ${XXH_PLACE} ${XXH_SRC_MD5} XXH_SRC )

# build external project
get_build ( XXH_BUILD xxhash )
external_build ( xxhash XXH_SRC XXH_BUILD BUILD_SHARED_LIBS=0 XXHASH_BUILD_XXHSUM=0 )

# now it should find
find_package ( xxHash REQUIRED CONFIG )
return_if_target_found ( xxHash::xxhash "was built and saved" )
