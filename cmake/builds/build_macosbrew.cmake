# ---------- homebrew ----------

message ( STATUS "Installing via Homebrew" )

if (NOT installed)
	# start with short route - set all paths
	include ( GNUInstallDirs )
	SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )
	SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/manticore/data" )

	# these guys came from homebrew formula
	set ( CMAKE_INSTALL_FULL_SYSCONFDIR ${_SYSCONFDIR} )
	set ( CMAKE_INSTALL_FULL_LOCALSTATEDIR ${_LOCALSTATEDIR} )
	set ( CMAKE_INSTALL_FULL_RUNSTATEDIR ${_RUNSTATEDIR} )
	set ( installed ON )
endif ()

string(REGEX REPLACE "/Cellar/manticoresearch/[^/]+/" "/" FULL_SHARE_DIR ${FULL_SHARE_DIR})

if (only_set_paths)
	return ()
endif ()

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

# generate config files
configure_config ( manticore )

install ( FILES "${MANTICORE_BINARY_DIR}/manticore.conf" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT common )
install ( FILES COPYING INSTALL DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common )
install ( FILES example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT tools )
install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT common )

if (WITH_ICU AND WITH_ICU_FORCE_STATIC)
	install_icudata ( ${CMAKE_INSTALL_DATADIR}/manticore/icu )
endif ()

if (NOT NOAPI)
	install ( DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT searchd )
endif ()

# data and log dirs are created by brew formula
