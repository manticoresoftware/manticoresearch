# ---------- homebrew ----------

message ( STATUS "Installing via Homebrew" )

if (NOT installed)
	# start with short route - set all paths

	# the following temporary replacement of CMAKE_INSTALL_PREFIX to HOMEBREW_PREFIX is needed mainly
	# for FULL_SHARE_DIR to become just /opt/homebrew/share/ instead of smth like /opt/homebrew/Cellar/manticoresearch/5.0.3-2023012600-6ffa0d1/share
	set ( CMAKE_INSTALL_PREFIX_ORIGINAL "${CMAKE_INSTALL_PREFIX}" )
	set ( CMAKE_INSTALL_PREFIX "$ENV{HOMEBREW_PREFIX}" )
	include ( GNUInstallDirs )
	set ( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX_ORIGINAL}" )

	set ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )


	# these guys came from homebrew formula
	set ( CMAKE_INSTALL_FULL_SYSCONFDIR ${_SYSCONFDIR} )
	set ( CMAKE_INSTALL_FULL_LOCALSTATEDIR ${_LOCALSTATEDIR} )
	set ( CMAKE_INSTALL_FULL_RUNSTATEDIR ${_RUNSTATEDIR} )
	# we have to use the external _LOCALSTATEDIR, otherwise the value may be incorrect (https://manticoresearch.slack.com/archives/C5EEXJG31/p1675084036163049)
	set ( LOCALDATADIR "${_LOCALSTATEDIR}/manticore/data" )
	set ( installed ON )
endif ()

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
