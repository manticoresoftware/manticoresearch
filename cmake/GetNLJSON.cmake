cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )

set ( NLJSON_GITHUB "http://github.com/nlohmann/json/archive/refs/tags/v3.10.5.tar.gz" )
set ( NLJSON_BUNDLE "${LIBS_BUNDLE}/nljson-v3.10.5.tar.gz" )
set ( NLJSON_SRC_MD5 "5b946f7d892fa55eabec45e76a20286b" )

include ( update_bundle )

# determine destination folder where we expect pre-built nljson
find_package ( nlohmann_json QUIET CONFIG )
return_if_target_found ( nlohmann_json::nlohmann_json "found ready (no need to build)" )

# not found. Populate and prepare sources
select_nearest_url ( NLJSON_PLACE nlohmann_json ${NLJSON_BUNDLE} ${NLJSON_GITHUB} )
fetch_and_check ( nlohmann_json ${NLJSON_PLACE} ${NLJSON_SRC_MD5} NLJSON_SRC )

# build external project
get_build ( NLJSON_BUILD nlohmann_json )
external_build ( nlohmann_json NLJSON_SRC NLJSON_BUILD JSON_BuildTests=0 JSON_MultipleHeaders=1 JSON_GlobalUDLs=0 )

# now it should find
find_package ( nlohmann_json REQUIRED CONFIG )
return_if_target_found ( nlohmann_json::nlohmann_json "was built and saved" )
