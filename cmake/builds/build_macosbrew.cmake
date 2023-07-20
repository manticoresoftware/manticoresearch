# ---------- macos ----------

message ( STATUS "Installing via Homebrew" )

if (NOT installed)
	# start with short route - set all paths

	if ( CMAKE_SYSTEM_PROCESSOR STREQUAL arm64 )
		set ( HOMEBREW_PREFIX "/opt/homebrew")
	else()
		set ( HOMEBREW_PREFIX "/usr/local" )
	endif()

	# the following temporary replacement of CMAKE_INSTALL_PREFIX to HOMEBREW_PREFIX is needed mainly
	# for FULL_SHARE_DIR to become just /opt/homebrew/share/ instead of smth like /opt/homebrew/Cellar/manticoresearch/5.0.3-2023012600-6ffa0d1/share
	set ( CMAKE_INSTALL_PREFIX_ORIGINAL "${CMAKE_INSTALL_PREFIX}" )
	set ( CMAKE_INSTALL_PREFIX "${HOMEBREW_PREFIX}" )
	include ( GNUInstallDirs )
	#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
	set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
	GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )
	set ( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX_ORIGINAL}" )

	set ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )

	# by default, etc, var, var/run with /opt/homebrew became /etc/opt/homebrew, /var/opt/homebrew and /var/run/opt/homebrew, so need to be fixed
	set ( CMAKE_INSTALL_FULL_SYSCONFDIR "${HOMEBREW_PREFIX}/etc" )
	set ( CMAKE_INSTALL_FULL_LOCALSTATEDIR "${HOMEBREW_PREFIX}/var" )
	set ( CMAKE_INSTALL_FULL_RUNSTATEDIR "${HOMEBREW_PREFIX}/var/run" )

	set ( LOCALDATADIR "${HOMEBREW_PREFIX}/var/manticore" )
	set ( SPLIT_SYMBOLS 1 )

	# configure specific stuff
	set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ${CMAKE_SYSTEM_PROCESSOR}" )
	set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch ${CMAKE_SYSTEM_PROCESSOR}" )
	set ( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-osx${CMAKE_OSX_DEPLOYMENT_TARGET}-${CMAKE_SYSTEM_PROCESSOR}" )
	SET ( CMAKE_INSTALL_FULL_LOCALLIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}" )

	set ( installed ON )
endif ()

if (only_set_paths)
	return ()
endif ()

set ( CPACK_GENERATOR "TGZ" )
message ( STATUS "Will create ${CPACK_GENERATOR} with build for MacOS" )



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
