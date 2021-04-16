# Common informational variables for CPack

# internal vars for easier substitutions
set(MANTICORESEARCH "Manticore Search")
set(INFO_BANNER "${MANTICORESEARCH} is a powerful free open source search engine
with a focus on low latency and high throughput full-text search
and high volume stream filtering.")
set(THIS_PKG "${INFO_BANNER} This package")

# common metadata
set(CPACK_PACKAGE_CONTACT "${PACKAGE_NAME} package builds <build@manticoresearch.com>")
set(CMAKE_PROJECT_HOMEPAGE_URL "https://manticoresearch.com")
set(CPACK_PACKAGE_NAME "${PACKAGE_NAME}")
set(CPACK_PACKAGE_VENDOR "${COMPANY_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${MANTICORE_BINARY_DIR}/COPYING.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${MANTICORESEARCH} - database for search")
set(CPACK_PACKAGE_DESCRIPTION "${INFO_BANNER}")

# main. This is used when create 'simple package', without split
set(CPACK_COMPONENT_MAIN_DESCRIPTION "${INFO_BANNER}")

# meta
set(CPACK_COMPONENT_META_GROUP "main")
set(CPACK_COMPONENT_GROUP_META_DISPLAY_NAME "Virtual")
set(CPACK_COMPONENT_META_DISPLAY_NAME "${PACKAGE_NAME} meta-package")
set(CPACK_COMPONENT_META_DESCRIPTION "${THIS_PKG} is a group package that guarantee you have all is needed
to run ${MANTICORESEARCH}.")

# searchd daemon
set(CPACK_COMPONENT_APPLICATIONS_GROUP "main")
set(CPACK_COMPONENT_GROUP_APLLICATIONS_DISPLAY_NAME "Server")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "${PACKAGE_NAME} service")
set(CPACK_COMPONENT_APPLICATIONS_SUMMARY "${MANTICORESEARCH} server files")
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "${THIS_PKG} contains the service daemon.")

# converter tool
set(CPACK_COMPONENT_CONVERTER_GROUP "converter")
set(CPACK_COMPONENT_GROUP_CONVERTER_DISPLAY_NAME "Converter")
set(CPACK_COMPONENT_CONVERTER_DISPLAY_NAME "CONVERTER application")
set(CPACK_COMPONENT_CONVERTER_SUMMARY "${MANTICORESEARCH} index converter")
set(CPACK_COMPONENT_CONVERTER_DESCRIPTION "${THIS_PKG} contains the tool for converting indexes made prior version 3.0.")

# icu data library
set(CPACK_COMPONENT_ICUDATA_GROUP "main")
set(CPACK_COMPONENT_ICUDATA_DISPLAY_NAME "ICU DAT file")
set(CPACK_COMPONENT_ICUDATA_SUMMARY "${MANTICORESEARCH} ICU files")
set(CPACK_COMPONENT_ICUDATA_DESCRIPTION "${THIS_PKG} contains the ICU DAT file. You need this package only if you use the ICU morphology.")

# applications (indexes,indextool etc.)
set(CPACK_COMPONENT_TOOLS_GROUP "main")
set(CPACK_COMPONENT_GROUP_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "${PACKAGE_NAME} tools")
set(CPACK_COMPONENT_TOOLS_SUMMARY "${MANTICORESEARCH} utilities")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "${THIS_PKG} contains auxiliary tools.")

# development files
set(CPACK_COMPONENT_DEVEL_GROUP "main")
set(CPACK_COMPONENT_DEVEL_DISPLAY_NAME "Development")
set(CPACK_COMPONENT_GROUP_DEVEL_DISPLAY_NAME "Development files")
set(CPACK_COMPONENT_DEVEL_SUMMARY "${MANTICORESEARCH} development headers")
set(CPACK_COMPONENT_DEVEL_DESCRIPTION "${THIS_PKG} contains the headers for writing custom functions.")

# configurations
set(CPACK_COMPONENT_CONFIG_GROUP "main")
set(CPACK_COMPONENT_CONFIG_DISPLAY_NAME "Configuration examples")

# documentation files
set(CPACK_COMPONENT_DOC_GROUP "main")
set(CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation")
