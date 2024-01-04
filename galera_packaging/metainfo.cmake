# Galera packaging metadata
# Whole vendor-specific metadata lives in this file, and in 'description.txt'

set ( CPACK_PACKAGE_NAME "manticore-galera" )
set ( CPACK_PACKAGE_CONTACT "Manticore Search Maintainers <build@manticoresearch.com>" )
set ( CPACK_PACKAGE_VENDOR ${CPACK_PACKAGE_CONTACT} )
set ( CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/manticoresoftware/galera" )
set ( CPACK_RESOURCE_FILE_LICENSE "${GALERA_SOURCE_DIR}/LICENSE" )
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Synchronous multi-master wsrep provider (replication engine) (Manticore's fork)" )
set ( CPACK_PACKAGE_DESCRIPTION_FILE "${GALERA_CMAKE_DIR}/description.txt")

set ( CPACK_DEBIAN_PACKAGE_SECTION "database" )

set ( CPACK_RPM_PACKAGE_GROUP "Applications/Internet" )
set ( CPACK_RPM_PACKAGE_LICENSE "GPLv2" )
