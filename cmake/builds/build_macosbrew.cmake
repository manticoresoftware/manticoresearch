# ---------- macosbrew ----------

message ( STATUS "Installing for MacOS via Brew" )

set ( SPLIT_SYMBOLS 1 )

# configure specific stuff
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64" )
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64" )

# now get system paths
include (GNUInstallDirs)

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set (CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore")
GNUInstallDirs_get_absolute_install_dir (CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR)

# generate config files
configure_config (manticore)

install (FILES "${MANTICORE_BINARY_DIR}/manticore.conf" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT server)
install (FILES COPYING INSTALL example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT server)
install (DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server)

SET (FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore")
if (WITH_ICU)
	install_icudata (${CMAKE_INSTALL_DATADIR}/manticore/icu)
endif ()

if (NOT NOAPI)
        install (DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server)
endif ()

SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/manticore/data")

set (installed ON)
# data and log dirs are created by brew formula
