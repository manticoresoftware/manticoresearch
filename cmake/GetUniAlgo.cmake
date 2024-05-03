cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

set ( UNIALGO_GITHUB "https://github.com/manticoresoftware/uni-algo/archive/refs/tags/v0.7.2.tar.gz" )
set ( UNIALGO_BUNDLE "${LIBS_BUNDLE}/unialgo-v0.7.2.tar.gz" )
set ( UNIALGO_SRC_MD5 "11f64b34000f3b98fa806d01eeeda70b" )

include ( update_bundle )

# determine destination folder where we expect pre-built uni-algo
find_package ( uni-algo QUIET CONFIG )
return_if_target_found ( uni-algo::uni-algo "found ready (no need to build)" )

# not found. Populate and prepare sources
select_nearest_url ( UNIALGO_PLACE uni-algo ${UNIALGO_BUNDLE} ${UNIALGO_GITHUB} )
fetch_and_check ( uni-algo ${UNIALGO_PLACE} ${UNIALGO_SRC_MD5} UNIALGO_SRC )

# build external project
get_build ( UNIALGO_BUILD uni-algo )
external_build ( uni-algo UNIALGO_SRC UNIALGO_BUILD UNI_ALGO_DISABLE_PROP=1 UNI_ALGO_DISABLE_BREAK_WORD=1 UNI_ALGO_DISABLE_COLLATE=1 UNI_ALGO_DISABLE_NFKC_NFKD=1 UNI_ALGO_DISABLE_SHRINK_TO_FIT=1 )

# now it should find
find_package ( uni-algo REQUIRED CONFIG )
return_if_target_found ( uni-algo::uni-algo "was built and saved" )
