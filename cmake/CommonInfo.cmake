# Common informational variables for CPack

# internal vars for easier substitutions
set ( MANTICORESEARCH "Manticore Search" )
set ( INFO_BANNER "Lightweight SQL-first database designed for search applications that suits both big and small data collections." )
set ( THIS_PKG "${INFO_BANNER} This package" )

# common metadata
set ( CPACK_PACKAGE_CONTACT "Manticore Search Maintainers <build@manticoresearch.com>" )
set ( CMAKE_PROJECT_HOMEPAGE_URL "https://manticoresearch.com" )
set ( CPACK_PACKAGE_VENDOR "${COMPANY_NAME}" )
set ( CPACK_RESOURCE_FILE_LICENSE "${MANTICORE_BINARY_DIR}/COPYING.txt" )
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "${MANTICORESEARCH} - database for search" )
set ( CPACK_PACKAGE_DESCRIPTION "${INFO_BANNER}" )

# converter tool - only citizen of group 'helpers'
set ( CPACK_COMPONENT_GROUP_HELPERS_DISPLAY_NAME "Converter" )
set ( CPACK_COMPONENT_CONVERTER_DISPLAY_NAME "CONVERTER application" )
set ( CPACK_COMPONENT_CONVERTER_SUMMARY "${MANTICORESEARCH} index converter" )
set ( CPACK_COMPONENT_CONVERTER_DESCRIPTION "${THIS_PKG} contains tool index_converter needed for converting indexes made prior version 3.0." )

# meta
set ( CPACK_COMPONENT_GROUP_META_DISPLAY_NAME "Metapackage" )
set ( CPACK_COMPONENT_META_DISPLAY_NAME "${PACKAGE_NAME} meta-package" )
set ( CPACK_COMPONENT_META_DESCRIPTION "${THIS_PKG} is a group package that guarantee you have all needed to run ${MANTICORESEARCH}." )

# common
set ( CPACK_COMPONENT_GROUP_COMMON_DISPLAY_NAME "Common stuff" )
set ( CPACK_COMPONENT_COMMON_DISPLAY_NAME "${PACKAGE_NAME} common files" )
set ( CPACK_COMPONENT_COMMON_DESCRIPTION "${THIS_PKG} contains config, stopwords and other common stuff for daemon and auxiliary tools" )

# searchd
set ( CPACK_COMPONENT_GROUP_SEARCHD_DISPLAY_NAME "Server" )
set ( CPACK_COMPONENT_SEARCHD_DISPLAY_NAME "${PACKAGE_NAME} service" )
set ( CPACK_COMPONENT_SEARCHD_SUMMARY "${MANTICORESEARCH} server files" )
set ( CPACK_COMPONENT_SEARCHD_DESCRIPTION "${THIS_PKG} contains Manticore Search daemon." )

# manticore-server
set ( CPACK_COMPONENT_GROUP_SERVER_DISPLAY_NAME "Systemd and other service scripts" )
set ( CPACK_COMPONENT_SERVER_DISPLAY_NAME "${PACKAGE_NAME} service" )
set ( CPACK_COMPONENT_SERVER_SUMMARY "${MANTICORESEARCH} systemd service scripts" )
set ( CPACK_COMPONENT_SERVER_DESCRIPTION "${THIS_PKG} contains startup and systemd files for Manticore Search." )

# icu data library
set ( CPACK_COMPONENT_ICUDATA_DISPLAY_NAME "ICU DAT file" )
set ( CPACK_COMPONENT_ICUDATA_SUMMARY "${MANTICORESEARCH} ICU files" )
set ( CPACK_COMPONENT_ICUDATA_DESCRIPTION "${THIS_PKG} contains the ICU DAT file. You need this package only if you use ICU morphology." )
set ( CPACK_ARCHIVE_ICUDATA_FILE_NAME "manticore-icudata-65l" )

# tools (indexes,indextool etc.)
set ( CPACK_COMPONENT_TOOLS_GROUP "main" )
set ( CPACK_COMPONENT_GROUP_TOOLS_DISPLAY_NAME "Tools" )
set ( CPACK_COMPONENT_TOOLS_DISPLAY_NAME "${PACKAGE_NAME} tools" )
set ( CPACK_COMPONENT_TOOLS_SUMMARY "${MANTICORESEARCH} utilities" )
set ( CPACK_COMPONENT_TOOLS_DESCRIPTION "${THIS_PKG} contains auxiliary tools." )

# development files
set ( CPACK_COMPONENT_DEVEL_GROUP "main" )
set ( CPACK_COMPONENT_DEVEL_DISPLAY_NAME "Development" )
set ( CPACK_COMPONENT_GROUP_DEVEL_DISPLAY_NAME "Development files" )
set ( CPACK_COMPONENT_DEVEL_SUMMARY "${MANTICORESEARCH} development headers" )
set ( CPACK_COMPONENT_DEVEL_DESCRIPTION "${THIS_PKG} contains headers for writing custom functions." )

# Deps versions parsing
file(STRINGS "${MANTICORE_SOURCE_DIR}/deps.txt" lines)
foreach(line ${lines})
  if(line STREQUAL "---")
    break()
  endif()
    message("Parsing deps.txt line: ${line}")
    string(REGEX MATCH "^([^ ]+) ([^ ]+) ([^ ]+) ([^ ]+)" _ ${line})
    string(TOUPPER ${CMAKE_MATCH_1} dep)
    set("${dep}_VERNUM" ${CMAKE_MATCH_2})
    set("${dep}_VERDATE" ${CMAKE_MATCH_3})
    set("${dep}_VERHASH" ${CMAKE_MATCH_4})
    message("${dep} version: ${CMAKE_MATCH_2}-${CMAKE_MATCH_3}-${CMAKE_MATCH_4}")
endforeach()
