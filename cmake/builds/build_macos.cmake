# ---------- macos ----------
# Above line is mandatory!
# rules to build tgz archive for Mac OS X

if (NOT installed)
	# start with short route - set all paths
	set ( CPACK_PACKAGING_INSTALL_PREFIX /usr/local )
	set ( CMAKE_INSTALL_PREFIX "${CPACK_PACKAGING_INSTALL_PREFIX}" CACHE PATH "prefix from distr build" FORCE )
	include ( GNUInstallDirs )
	SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )
	SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/manticore/data" )
	set ( SPLIT_SYMBOLS 1 )
	# configure specific stuff
	set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ${CMAKE_SYSTEM_PROCESSOR}" )
	set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch ${CMAKE_SYSTEM_PROCESSOR}" )
	set ( CPACK_SUFFIX "-osx${CMAKE_OSX_DEPLOYMENT_TARGET}-${CMAKE_SYSTEM_PROCESSOR}" )
	set ( installed ON )
endif ()

if (only_set_paths)
	return ()
endif ()

set ( CPACK_GENERATOR "TGZ" )
message ( STATUS "Will create ${CPACK_GENERATOR} with build for MacOS X" )

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

# generate config files
configure_config ( manticore )

# install specific stuff
install ( DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT common )
install ( DIRECTORY doc manual contrib DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common )
install ( FILES "${MANTICORE_BINARY_DIR}/manticore.conf" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticore COMPONENT common )

install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore/data COMPONENT common )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT searchd )

install ( FILES COPYING INSTALL DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common )
install ( FILES example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT tools )

install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT common )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/modules COMPONENT common )

if (WITH_ICU)
	install_icudata ( ${CMAKE_INSTALL_DATADIR}/manticore/icu )
endif ()
