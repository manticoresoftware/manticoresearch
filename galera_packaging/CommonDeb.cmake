cmake_minimum_required ( VERSION 3.24 )

# Common debian-specific build variables
set ( CPACK_GENERATOR "DEB" )

set ( CPACK_DEBIAN_FILE_NAME DEB-DEFAULT )
set ( CPACK_DEBIAN_DEBUGINFO_PACKAGE ON )
set ( CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON )

if (NOT disable_shlideps)
	set ( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )
endif ()

set ( CPACK_PACKAGING_INSTALL_PREFIX "/" )
set ( CMAKE_INSTALL_PREFIX "${CPACK_PACKAGING_INSTALL_PREFIX}" CACHE PATH "prefix from distr build" FORCE )
include ( GNUInstallDirs )

install ( TARGETS galera LIBRARY DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/modules )

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/GALERA, fixup to share/doc/manticore-galera
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore-galera" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

# see https://github.com/manticoresoftware/dev/issues/416#issuecomment-1807560203 for details

install ( FILES AUTHORS README packages/debian/changelog debian/copyright DESTINATION ${CMAKE_INSTALL_DOCDIR} )

# set(CPACK_DEBIAN_PACKAGE_DEBUG ON)
