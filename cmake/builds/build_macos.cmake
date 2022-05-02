# ---------- macos ----------
# Above line is mandatory!
# rules to build tgz archive for Mac OS X

set (CPACK_GENERATOR "TGZ")
set (CPACK_PACKAGING_INSTALL_PREFIX /usr/local)

message (STATUS "Will create TGZ with build for Mac Os X")
set (SPLIT_SYMBOLS 1)

# configure specific stuff
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")

find_program (SWVERSPROG sw_vers)
if (SWVERSPROG)
	# use dpkg to fix the package file name
	execute_process (
			COMMAND ${SWVERSPROG} -productVersion
			OUTPUT_VARIABLE MACOSVER
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	mark_as_advanced (SWVERSPROG MACOSVER)
endif (SWVERSPROG)

if (NOT MACOSVER)
	set (MACOSVER "10.12")
endif ()

set (CPACK_SUFFIX "-osx${MACOSVER}-x86_64")

# now get system paths
set (CMAKE_INSTALL_PREFIX "${CPACK_PACKAGING_INSTALL_PREFIX}" CACHE PATH "prefix from distr build" FORCE)
include (GNUInstallDirs)

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set (CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore")
GNUInstallDirs_get_absolute_install_dir (CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR)

# generate config files
configure_config (manticore)

# install specific stuff
install (DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server)
install (DIRECTORY doc manual contrib DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT server)
install (FILES "${MANTICORE_BINARY_DIR}/manticore.conf" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticore COMPONENT server)

install (DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore/data COMPONENT server)
install (DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT server)

install (FILES COPYING INSTALL example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT server)

install (DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server)
install (FILES INSTALL DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server)
install (DIRECTORY DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/modules COMPONENT server)

SET (FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore")
if (WITH_ICU)
	install_icudata (${CMAKE_INSTALL_DATADIR}/manticore/icu)
endif ()

# package specific
SET (LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/manticore/data")

mark_as_advanced (CMAKE_OSX_ARCHITECTURES CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_SYSROOT)

set (installed ON)