cmake_minimum_required ( VERSION 3.24 )

# Common debian-specific build variables
set ( CPACK_GENERATOR "DEB" )

set ( CPACK_DEBIAN_FILE_NAME DEB-DEFAULT )
set ( CPACK_DEBIAN_DEBUGINFO_PACKAGE ON )
set ( CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON )

if (NOT disable_shlideps)
	set ( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )
endif ()

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/GALERA, fixup to share/doc/galera-4
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/galera-4" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

#/usr/share/doc/galera-4/AUTHORS - no changes (AUTHORS)
#/usr/share/doc/galera-4/README.gz - no changes (README)
#/usr/share/doc/galera-4/changelog.Debian.gz - no changes (packages/debian/changelog)
#/usr/share/doc/galera-4/copyright - no changes (debian/copyright)
# see https://github.com/manticoresoftware/dev/issues/416 for details

install ( FILES AUTHORS README packages/debian/changelog debian/copyright DESTINATION ${CMAKE_INSTALL_DOCDIR} )

# set(CPACK_DEBIAN_PACKAGE_DEBUG ON)