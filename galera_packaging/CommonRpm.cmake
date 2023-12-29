cmake_minimum_required ( VERSION 3.24 )

# Common rpm-specific build variables
set ( CPACK_GENERATOR "RPM" )

set ( CPACK_RPM_FILE_NAME RPM-DEFAULT )
set ( CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
set ( CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )
set ( CPACK_RPM_INSTALL_WITH_EXEC ON )
set ( CPACK_RPM_DEBUGINFO_PACKAGE ON )

if (RELEASE_DIST)
	set ( CPACK_RPM_PACKAGE_RELEASE 1${RELEASE_DIST} )
	set ( MYVER "${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}" )
else ()
	set ( CPACK_RPM_PACKAGE_RELEASE 1 )
	set ( CPACK_RPM_PACKAGE_RELEASE_DIST ON )
	set ( MYVER "${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}%{?dist}" )
endif ()

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/GALERA, fixup to share/doc/galera
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/galera" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

# /usr/share/doc/galera/AUTHORS - no changes (AUTHORS)
# /usr/share/doc/galera/COPYING - no changes (COPYING)
# /usr/share/doc/galera/LICENSE.asio - no changes (`asio/LICENSE_1_0.txt)
# /usr/share/doc/galera/README - no changes (README)
# see https://github.com/manticoresoftware/dev/issues/416 for details

install ( FILES AUTHORS COPYING README DESTINATION ${CMAKE_INSTALL_DOCDIR} )
install ( FILES asio/LICENSE_1_0.txt DESTINATION ${CMAKE_INSTALL_DOCDIR} RENAME "LICENSE.asio" )