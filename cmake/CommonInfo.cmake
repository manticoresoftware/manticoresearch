# Common informational variables for CPack

# internal vars for easier substitutions
set ( MANTICORESEARCH "Manticore Search" )
set ( INFO_BANNER "Lightweight SQL-first database designed for search applications that suits both big and small data collections." )
set ( THIS_PKG "${INFO_BANNER} This package" )

# common metadata
set ( CPACK_PACKAGE_CONTACT "Manticore Search Maintainers <build@manticoresearch.com>" )
set ( CMAKE_PROJECT_HOMEPAGE_URL "https://manticoresearch.com" )
set ( CPACK_PACKAGE_NAME "${PACKAGE_NAME}" )
set ( CPACK_PACKAGE_VENDOR "${COMPANY_NAME}" )
set ( CPACK_RESOURCE_FILE_LICENSE "${MANTICORE_BINARY_DIR}/COPYING.txt" )
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "${MANTICORESEARCH} - database for search" )
set ( CPACK_PACKAGE_DESCRIPTION "${INFO_BANNER}" )

# main. When no 'split', group 'main' used as one solid component and should be named
set ( CPACK_COMPONENT_MAIN_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}" )
set ( CPACK_COMPONENT_MAIN_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}" )

# converter tool
set ( CPACK_COMPONENT_CONVERTER_GROUP "converter" )
set ( CPACK_COMPONENT_GROUP_CONVERTER_DISPLAY_NAME "Converter" )
set ( CPACK_COMPONENT_CONVERTER_DISPLAY_NAME "CONVERTER application" )
set ( CPACK_COMPONENT_CONVERTER_SUMMARY "${MANTICORESEARCH} index converter" )
set ( CPACK_COMPONENT_CONVERTER_DESCRIPTION "${THIS_PKG} contains tool index_converter needed for converting indexes made prior version 3.0." )

# meta
set ( CPACK_COMPONENT_META_GROUP "main" )
set ( CPACK_COMPONENT_GROUP_META_DISPLAY_NAME "Virtual" )
set ( CPACK_COMPONENT_META_DISPLAY_NAME "${PACKAGE_NAME} meta-package" )
set ( CPACK_COMPONENT_META_DESCRIPTION "${THIS_PKG} is a group package that guarantee you have all needed to run ${MANTICORESEARCH}." )

# searchd daemon
set ( CPACK_COMPONENT_SERVER_GROUP "main" )
set ( CPACK_COMPONENT_GROUP_SERVER_DISPLAY_NAME "Server" )
set ( CPACK_COMPONENT_SERVER_DISPLAY_NAME "${PACKAGE_NAME} service" )
set ( CPACK_COMPONENT_SERVER_SUMMARY "${MANTICORESEARCH} server files" )
set ( CPACK_COMPONENT_SERVER_DESCRIPTION "${THIS_PKG} contains Manticore Search daemon." )

# runtime libraries (windows)
set ( CPACK_COMPONENT_RUNTIME_GROUP "main" )
set ( CPACK_COMPONENT_GROUP_RUNTIME_DISPLAY_NAME "Runtime" )
set ( CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "${PACKAGE_NAME} runtime" )
set ( CPACK_COMPONENT_RUNTIME_SUMMARY "${MANTICORESEARCH} runtime libraries" )
set ( CPACK_COMPONENT_RUNTIME_DESCRIPTION "${THIS_PKG} contains runtime libraries needed to run Manticore Search daemon." )

# manticore-systemd
set ( CPACK_COMPONENT_SYSTEMD_GROUP "main" )
set ( CPACK_COMPONENT_GROUP_SYSTEMD_DISPLAY_NAME "Systemd scripts" )
set ( CPACK_COMPONENT_SYSTEMD_DISPLAY_NAME "${PACKAGE_NAME} service" )
set ( CPACK_COMPONENT_SYSTEMD_SUMMARY "${MANTICORESEARCH} systemd service scripts" )
set ( CPACK_COMPONENT_SYSTEMD_DESCRIPTION "${THIS_PKG} contains systemd files for Manticore Search." )

# icu data library
set ( CPACK_COMPONENT_ICUDATA_GROUP "main" )
set ( CPACK_COMPONENT_ICUDATA_DISPLAY_NAME "ICU DAT file" )
set ( CPACK_COMPONENT_ICUDATA_SUMMARY "${MANTICORESEARCH} ICU files" )
set ( CPACK_COMPONENT_ICUDATA_DESCRIPTION "${THIS_PKG} contains the ICU DAT file. You need this package only if you use ICU morphology." )

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

# configurations
set ( CPACK_COMPONENT_CONFIG_GROUP "main" )
set ( CPACK_COMPONENT_CONFIG_DISPLAY_NAME "Configuration examples" )

# documentation files
set ( CPACK_COMPONENT_DOC_GROUP "main" )
set ( CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation" )
