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
set ( CPACK_COMPONENT_CONVERTER_DESCRIPTION "${THIS_PKG} includes the tool \"index_converter\" necessary for converting indexes created before version 3.0." )

# meta
set ( CPACK_COMPONENT_GROUP_META_DISPLAY_NAME "Metapackage" )
set ( CPACK_COMPONENT_META_DISPLAY_NAME "${PACKAGE_NAME} meta-package" )
set ( CPACK_COMPONENT_META_DESCRIPTION "${THIS_PKG} is a group package that refers to everything needed to run ${MANTICORESEARCH}." )

# common
set ( CPACK_COMPONENT_GROUP_COMMON_DISPLAY_NAME "Common stuff" )
set ( CPACK_COMPONENT_COMMON_DISPLAY_NAME "${PACKAGE_NAME} common files" )
set ( CPACK_COMPONENT_COMMON_DESCRIPTION "${THIS_PKG} includes config files, stopwords, and other necessary resources for the daemon and auxiliary tools." )

# searchd
set ( CPACK_COMPONENT_GROUP_SEARCHD_DISPLAY_NAME "Server" )
set ( CPACK_COMPONENT_SEARCHD_DISPLAY_NAME "${PACKAGE_NAME} service" )
set ( CPACK_COMPONENT_SEARCHD_SUMMARY "${MANTICORESEARCH} server files" )
set ( CPACK_COMPONENT_SEARCHD_DESCRIPTION "${THIS_PKG} includes the Manticore Search daemon." )

# manticore-server
set ( CPACK_COMPONENT_GROUP_SERVER_DISPLAY_NAME "Systemd and other service scripts" )
set ( CPACK_COMPONENT_SERVER_DISPLAY_NAME "${PACKAGE_NAME} service" )
set ( CPACK_COMPONENT_SERVER_SUMMARY "${MANTICORESEARCH} systemd service scripts" )
set ( CPACK_COMPONENT_SERVER_DESCRIPTION "${THIS_PKG} includes startup and systemd files for Manticore Search." )

# icu data library
set ( CPACK_COMPONENT_ICUDATA_DISPLAY_NAME "ICU DAT file" )
set ( CPACK_COMPONENT_ICUDATA_SUMMARY "${MANTICORESEARCH} ICU files" )
set ( CPACK_COMPONENT_ICUDATA_DESCRIPTION "${THIS_PKG} includes the ICU DAT file, which you need when using ICU morphology." )
set ( CPACK_ARCHIVE_ICUDATA_FILE_NAME "manticore-icudata-65l" )

# tools (indexes,indextool etc.)
set ( CPACK_COMPONENT_TOOLS_GROUP "main" )
set ( CPACK_COMPONENT_GROUP_TOOLS_DISPLAY_NAME "Tools" )
set ( CPACK_COMPONENT_TOOLS_DISPLAY_NAME "${PACKAGE_NAME} tools" )
set ( CPACK_COMPONENT_TOOLS_SUMMARY "${MANTICORESEARCH} utilities" )
set ( CPACK_COMPONENT_TOOLS_DESCRIPTION "${THIS_PKG} includes auxiliary tools." )

# development files
set ( CPACK_COMPONENT_DEVEL_GROUP "main" )
set ( CPACK_COMPONENT_DEVEL_DISPLAY_NAME "Development" )
set ( CPACK_COMPONENT_GROUP_DEVEL_DISPLAY_NAME "Development files" )
set ( CPACK_COMPONENT_DEVEL_SUMMARY "${MANTICORESEARCH} development headers" )
set ( CPACK_COMPONENT_DEVEL_DESCRIPTION "${THIS_PKG} includes headers for writing custom functions." )
