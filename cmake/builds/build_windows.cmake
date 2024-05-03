# ---------- windows ----------
# Above line is mandatory!
# rules to build default zip archive for windows
message ( STATUS "Will create ZIP archive, and NSIS installer for windows" )

if (NOT installed)
	# start with short route - set all paths
	set ( CPACK_NSIS_INSTALL_ROOT "c:" )
	set ( CPACK_PACKAGE_INSTALL_DIRECTORY manticore )
	set ( CMAKE_INSTALL_PREFIX "${CPACK_NSIS_INSTALL_ROOT}/${CPACK_PACKAGE_INSTALL_DIRECTORY}" )
	set ( SPLIT_SYMBOLS 1 )
	include ( GNUInstallDirs )
	SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}" )
	SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}" )
	SET ( CMAKE_INSTALL_FULL_LOCALLIBDIR "usr/local/lib" )
	set ( installed ON )
endif ()

if (only_set_paths)
	return ()
endif ()

set ( CPACK_GENERATOR "ZIP" )
set ( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-x64" )

set ( CPACK_ARCHIVE_COMPONENT_INSTALL ON )
set ( CPACK_ARCHIVE_HELPERS_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-converter" )

set ( CPACK_COMPONENT_ICUDATA_ARCHIVE_FILE "${CPACK_ARCHIVE_ICUDATA_FILE_NAME}.zip" )

# base where installer will download the archives
set ( DISTR_URL "https://repo.manticoresearch.com/repository/manticoresearch_windows" )

if (DEV_BUILD)
	set ( CPACK_DOWNLOAD_SITE "${DISTR_URL}/dev/x64/" )
else ()
	set ( CPACK_DOWNLOAD_SITE "${DISTR_URL}/release/x64/" )
endif ()

# where install archives for download are prepared. Intentionally overwrite files prepared by ZIP
set ( CPACK_UPLOAD_DIRECTORY "${MANTICORE_BINARY_DIR}" )

set ( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )

set ( CPACK_COMPONENT_DBGSYMBOLS_DISPLAY_NAME "Debug symbols" )
set ( CPACK_COMPONENT_DEVEL_DISABLED ON )
set ( CPACK_COMPONENT_DBGSYMBOLS_DISABLED ON )
set ( CPACK_COMPONENT_DBGSYMBOLS_DOWNLOADED ON )
set ( CPACK_COMPONENT_CONVERTER_DISABLED ON )
set ( CPACK_COMPONENT_CONVERTER_DOWNLOADED ON )
set ( CPACK_COMPONENT_ICUDATA_DISABLED ON )
set ( CPACK_COMPONENT_ICUDATA_DOWNLOADED ON )

# generate config file(s)
configure_config ( data )

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

install ( DIRECTORY api doc contrib DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common )
install ( FILES example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT tools )
install ( FILES "${MANTICORE_BINARY_DIR}/manticore.conf" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT common )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT searchd )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/run/manticore COMPONENT searchd )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR} COMPONENT common )
install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR} COMPONENT common )
if (WITH_ICU AND WITH_ICU_FORCE_STATIC)
	install_icudata ( ${CMAKE_INSTALL_DATADIR}/icu )
endif ()
