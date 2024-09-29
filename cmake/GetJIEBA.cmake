#=============================================================================
# Copyright 2017-2024, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
set ( JIEBA_REPO "https://github.com/manticoresoftware/cppjieba" )
set ( JIEBA_REV "c4eda211" )
set ( JIEBA_SRC_MD5 "28329fd68b17410210369fb43a2127a1" )

set ( JIEBA_GITHUB "${JIEBA_REPO}/archive/${JIEBA_REV}.zip" )
set ( JIEBA_BUNDLE "${LIBS_BUNDLE}/cppjieba-${XXH_REV}.zip" )

cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )
include ( update_bundle )

# determine destination folder where we expect pre-built nljson
find_package ( jieba QUIET CONFIG )
return_if_target_found ( jieba::jieba "found ready (no need to build)" )

# not found. Populate and prepare sources
select_nearest_url ( JIEBA_PLACE jieba ${JIEBA_BUNDLE} ${JIEBA_GITHUB} )
fetch_and_check ( jieba ${JIEBA_PLACE} ${JIEBA_SRC_MD5} JIEBA_SRC )

# build external project
get_build ( JIEBA_BUILD jieba )
external_build ( jieba JIEBA_SRC JIEBA_BUILD )

# now it should find
find_package ( jieba REQUIRED CONFIG )
return_if_target_found ( jieba::jieba "was built and saved" )