# Common informational variables for CPack

# internal vars for easier substitutions
set ( COMMON_MANTICORE_INFO "Manticore Search - database for search. Lightweight SQL-first database designed for search applications that suitable for both large and small data collections. This package" )

# common metadata
set ( CPACK_PACKAGE_CONTACT "Manticore Search Maintainers <build@manticoresearch.com>" )
set ( CMAKE_PROJECT_HOMEPAGE_URL "https://manticoresearch.com" )
set ( CPACK_PACKAGE_VENDOR "${COMPANY_NAME}" )
set ( CPACK_RESOURCE_FILE_LICENSE "${MANTICORE_BINARY_DIR}/COPYING.txt" )
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Manticore Search - database for search" )

# converter tool - only citizen of group 'helpers'
set ( CPACK_COMPONENT_GROUP_HELPERS_DISPLAY_NAME "Converter" )
set ( CPACK_COMPONENT_CONVERTER_DISPLAY_NAME "CONVERTER application" )
set ( CPACK_COMPONENT_CONVERTER_SUMMARY "Manticore Search index converter" )
set ( CPACK_COMPONENT_CONVERTER_DESCRIPTION "${COMMON_MANTICORE_INFO} includes 'index_converter' needed for converting tables created in version < 3.0" )

# meta
set ( CPACK_COMPONENT_GROUP_META_DISPLAY_NAME "Metapackage" )
set ( CPACK_COMPONENT_META_DISPLAY_NAME "${PACKAGE_NAME} meta-package" )
set ( CPACK_COMPONENT_META_SUMMARY "Manticore Search group package" )
set ( CPACK_COMPONENT_META_DESCRIPTION "${COMMON_MANTICORE_INFO} is a group package that refers to everything needed to run Manticore Search." )

# common
set ( CPACK_COMPONENT_GROUP_COMMON_DISPLAY_NAME "Common stuff" )
set ( CPACK_COMPONENT_COMMON_DISPLAY_NAME "${PACKAGE_NAME} common files" )
set ( CPACK_COMPONENT_COMMON_SUMMARY "Manticore Search common files" )
set ( CPACK_COMPONENT_COMMON_DESCRIPTION "${COMMON_MANTICORE_INFO} includes config files, stopwords, and other necessary resources for the daemon and auxiliary tools." )

# searchd
set ( CPACK_COMPONENT_GROUP_SEARCHD_DISPLAY_NAME "Server" )
set ( CPACK_COMPONENT_SEARCHD_DISPLAY_NAME "${PACKAGE_NAME} service" )
set ( CPACK_COMPONENT_SEARCHD_SUMMARY "Manticore Search server" )
set ( CPACK_COMPONENT_SEARCHD_DESCRIPTION "${COMMON_MANTICORE_INFO} includes the Manticore Search server core files." )

# manticore-server
set ( CPACK_COMPONENT_GROUP_SERVER_DISPLAY_NAME "Systemd and other service scripts" )
set ( CPACK_COMPONENT_SERVER_DISPLAY_NAME "${PACKAGE_NAME} service" )
set ( CPACK_COMPONENT_SERVER_SUMMARY "Manticore Search systemd scripts" )
set ( CPACK_COMPONENT_SERVER_DESCRIPTION "${COMMON_MANTICORE_INFO} includes startup and systemd files for Manticore Search." )

# icu data library
set ( CPACK_COMPONENT_ICUDATA_DISPLAY_NAME "ICU DAT file" )
set ( CPACK_COMPONENT_ICUDATA_DESCRIPTION "Manticore Search ICU data files required for ICU morphology." )
set ( CPACK_COMPONENT_ICUDATA_SUMMARY "ICU data files for Manticore Search" )
set ( CPACK_ARCHIVE_ICUDATA_FILE_NAME "manticore-icudata-65l" )

# tools (indexes,indextool etc.)
set ( CPACK_COMPONENT_TOOLS_GROUP "main" )
set ( CPACK_COMPONENT_GROUP_TOOLS_DISPLAY_NAME "Tools" )
set ( CPACK_COMPONENT_TOOLS_DISPLAY_NAME "${PACKAGE_NAME} tools" )
set ( CPACK_COMPONENT_TOOLS_SUMMARY "Manticore Search tools" )
set ( CPACK_COMPONENT_TOOLS_DESCRIPTION "${COMMON_MANTICORE_INFO} includes auxiliary tools." )

# development files
set ( CPACK_COMPONENT_DEVEL_GROUP "main" )
set ( CPACK_COMPONENT_DEVEL_DISPLAY_NAME "Development" )
set ( CPACK_COMPONENT_GROUP_DEVEL_DISPLAY_NAME "Development files" )
set ( CPACK_COMPONENT_DEVEL_SUMMARY "Manticore Search development headers" )
set ( CPACK_COMPONENT_DEVEL_DESCRIPTION "${COMMON_MANTICORE_INFO} includes headers for writing custom functions." )
