#=============================================================================
# Copyright 2017-2025, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
set ( JIEBA_REPO "https://github.com/manticoresoftware/cppjieba" )
set ( JIEBA_REV "0fa3cb07" )
set ( JIEBA_SRC_MD5 "19b78cf80eef0c6289a6163674bfa869" )

set ( JIEBA_GITHUB "${JIEBA_REPO}/archive/${JIEBA_REV}.zip" )
set ( JIEBA_BUNDLE "${LIBS_BUNDLE}/cppjieba-${JIEBA_REV}.zip" )

cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )
include ( update_bundle )

# helpers
function ( install_jiebadicts DEST )
	if (NOT TARGET jieba::jiebadict)
		return ()
	endif ()
	get_target_property ( JIEBA_DICTS jieba::jiebadict INTERFACE_SOURCES )
	diag ( JIEBA_DICTS )
	install ( DIRECTORY ${JIEBA_DICTS}/ DESTINATION "${DEST}" COMPONENT jiebadicts )
endfunction ()

# determine destination folder where we expect ready jieba and dict
find_package ( jieba QUIET CONFIG )
if (TARGET jieba::jieba AND TARGET jieba::jiebadict)
	diagst ( jieba::jieba "found ready (no need to build)" )
	return ()
endif ()

# not found. Populate and prepare sources
select_nearest_url ( JIEBA_PLACE jieba ${JIEBA_BUNDLE} ${JIEBA_GITHUB} )
fetch_and_check ( jieba ${JIEBA_PLACE} ${JIEBA_SRC_MD5} JIEBA_SRC )

# build external project
get_build ( JIEBA_BUILD jieba )
external_build ( jieba JIEBA_SRC JIEBA_BUILD NO_BUILD=1 )

# now it should find
find_package ( jieba REQUIRED CONFIG )
return_if_target_found ( jieba::jieba "was built and saved" )