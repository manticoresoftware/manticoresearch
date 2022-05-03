# only cmake since 3.17 supports packaging of debuginfo
cmake_minimum_required ( VERSION 3.17 )

if (NOT installed)
	# start with short route - set all paths
	set ( CPACK_PACKAGING_INSTALL_PREFIX "/" )
	set ( CMAKE_INSTALL_PREFIX "${CPACK_PACKAGING_INSTALL_PREFIX}" CACHE PATH "prefix from distr build" FORCE )
	include ( GNUInstallDirs )
	SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )
	SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/manticore/data" ) # will be used also in the app
	set ( installed ON )
	diag ( CPACK_PACKAGING_INSTALL_PREFIX CMAKE_INSTALL_PREFIX )
endif ()

if (only_set_paths)
	return ()
endif ()

# Common debian-specific build variables
set ( CPACK_GENERATOR "DEB" )

set ( CPACK_DEBIAN_FILE_NAME DEB-DEFAULT )
set ( CPACK_DEBIAN_DEBUGINFO_PACKAGE ON )

# dependencies will be auto calculated. FIXME! M.b. point them directly?
#set ( CPACK_DEBIAN_BIN_PACKAGE_DEPENDS "libc6 (>= 2.15), libexpat (>= 2.0.1), libgcc1 (>= 1:3.0), libstdc++6 (>= 5.2), zlib1g (>= 1:1.1.4), lsb-base (>= 4.1+Debian11ubuntu7)" )

set ( CPACK_DEBIAN_PACKAGE_SHLIBDEPS "ON" )
set ( CPACK_DEBIAN_PACKAGE_SECTION "misc" )
set ( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )

set ( CPACK_DEBIAN_MAIN_PACKAGE_NAME "manticore" )
set ( CPACK_DEBIAN_MAIN_PACKAGE_CONTROL_EXTRA "${MANTICORE_BINARY_DIR}/conffiles;${MANTICORE_BINARY_DIR}/postinst;${MANTICORE_BINARY_DIR}/prerm;${MANTICORE_BINARY_DIR}/postrm" )
set ( CPACK_DEBIAN_MAIN_PACKAGE_REPLACES "manticore-bin, sphinxsearch" )

set ( CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION "ON" )

set ( CPACK_DEBIAN_SERVER_PACKAGE_REPLACES "manticore-bin, sphinxsearch" )
set ( CPACK_DEBIAN_SERVER_PACKAGE_NAME "manticore-server" )
set ( CPACK_DEBIAN_SERVER_FILE_NAME "DEB-DEFAULT" )
set ( CPACK_DEBIAN_SERVER_PACKAGE_CONTROL_EXTRA "${MANTICORE_BINARY_DIR}/conffiles;${MANTICORE_BINARY_DIR}/postinst;${MANTICORE_BINARY_DIR}/prerm;${MANTICORE_BINARY_DIR}/postrm" )

set ( CPACK_DEBIAN_META_PACKAGE_NAME "manticore-all" )
set ( CPACK_DEBIAN_META_PACKAGE_DEPENDS "manticore-server, manticore-tools, manticore-dev, manticore-icudata-65l" )
set ( CPACK_DEBIAN_META_PACKAGE_ARCHITECTURE all )
set ( CPACK_DEBIAN_META_FILE_NAME "DEB-DEFAULT" )
set ( CPACK_DEBIAN_META_PACKAGE_DEBUG "OFF" )
set ( CPACK_DEBIAN_META_PACKAGE_SECTION "metapackages" )

set ( CPACK_DEBIAN_ICUDATA_PACKAGE_NAME "manticore-icudata-65l" )
set ( CPACK_DEBIAN_ICUDATA_PACKAGE_ARCHITECTURE all )

set ( CPACK_DEBIAN_CONVERTER_PACKAGE_NAME "manticore-converter" )

set ( CPACK_DEBIAN_DEVEL_PACKAGE_NAME "manticore-dev" )
set ( CPACK_DEBIAN_DEVEL_PACKAGE_ARCHITECTURE all )

set ( CPACK_DEBIAN_TOOLS_PACKAGE_NAME "manticore-tools" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_CONFLICTS "sphinxsearch, manticore (<< 3.5.0-200722-1d34c491)" )


# pre-configuration. We're not yet configuring with final paths, but just construct incoming file
# at the beginning assume we have NOT yet set CMAKE_INSTALL_PREFIX, so any result of GNUInstallDirs is not actual

# make template postinst.in
FILE ( READ dist/deb/postinst.common.head POSTINST_HEAD )
FILE ( READ dist/deb/postinst.${flavour}.tail POSTINST_TAIL )
configure_file ( "dist/deb/postinst.in.in" "${MANTICORE_BINARY_DIR}/postinst.in" )

# copy template prerm
configure_file ( "dist/deb/prerm.${flavour}.in" "${MANTICORE_BINARY_DIR}/prerm.in" COPYONLY )

# copy template postrm
configure_file ( "dist/deb/postrm.in" "${MANTICORE_BINARY_DIR}/postrm.in" COPYONLY )


# manually configure (require known prefix/gnu install dirs)
configure_config ( lib/manticore )

# configure source files
configure_file ( "dist/deb/conffiles.in" "${MANTICORE_BINARY_DIR}/conffiles" @ONLY )
configure_file ( "dist/deb/manticore.default.in" "${MANTICORE_BINARY_DIR}/manticore" @ONLY )
configure_file ( "dist/deb/manticore.init.in" "${MANTICORE_BINARY_DIR}/manticore.init" @ONLY )
configure_file ( "dist/deb/README.Debian.in" "${MANTICORE_BINARY_DIR}/README.Debian" @ONLY )
configure_file ( "dist/deb/manticore.service.in" "${MANTICORE_BINARY_DIR}/manticore.service" @ONLY )
configure_file ( "dist/deb/manticore.generator.in" "${MANTICORE_BINARY_DIR}/manticore-generator" @ONLY )

# configure initially preconfigured scripts
configure_file ( "${MANTICORE_BINARY_DIR}/postinst.in" "${MANTICORE_BINARY_DIR}/postinst" @ONLY )
configure_file ( "${MANTICORE_BINARY_DIR}/prerm.in" "${MANTICORE_BINARY_DIR}/prerm" @ONLY )
configure_file ( "${MANTICORE_BINARY_DIR}/postrm.in" "${MANTICORE_BINARY_DIR}/postrm" @ONLY )

# installation

# stuff going to /etc
# CMAKE_INSTALL_SYSCONFDIR					etc 					/etc
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT server )
install ( FILES "${MANTICORE_BINARY_DIR}/manticore" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/default COMPONENT server )
install ( FILES "${MANTICORE_BINARY_DIR}/manticore.init" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/init.d
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ COMPONENT server RENAME manticore )

# stuff going to /var
# CMAKE_INSTALL_LOCALSTATEDIR				var 					/var
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore/data COMPONENT server )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT server )

# stuff that should go to /lib -> actually to /usr/lib
# CMAKE_INSTALL_LIBDIR						usr/lib64 				/usr/lib64
install ( FILES "${MANTICORE_BINARY_DIR}/manticore.service" DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system COMPONENT server )
install ( FILES "${MANTICORE_BINARY_DIR}/manticore-generator" DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system-generators
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ COMPONENT server )

# binaries go to /usr/bin (here is only new cluster, rest is in main file, installing targets)
# CMAKE_INSTALL_BINDIR						usr/bin 				/usr/bin
install ( PROGRAMS "dist/deb/manticore_new_cluster" DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT server )

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

# stuff going to /usr/share/man, /usr/share/doc
# CMAKE_INSTALL_MANDIR						usr/share/man 			/usr/share/man
# CMAKE_INSTALL_DOCDIR						usr/share/doc/manticore /usr/share/doc/manticore
install ( FILES doc/searchd.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT server )
install ( FILES doc/indexer.1 doc/indextool.1 doc/spelldump.1 doc/wordbreaker.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT tools )
install ( FILES "${MANTICORE_BINARY_DIR}/README.Debian" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT server )

# stuff going to /usr/share/manticore
# CMAKE_INSTALL_DATAROOTDIR					usr/share 				/usr/share
# CMAKE_INSTALL_DATADIR						usr/share 				/usr/share
install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server )
install ( FILES INSTALL DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/modules COMPONENT server )

if (WITH_ICU)
	install_icudata ( ${CMAKE_INSTALL_DATADIR}/manticore/icu )
endif ()

if (NOT NOAPI)
	install ( DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT server )
endif ()

if (SPLIT)
	get_cmake_property ( CPACK_COMPONENTS_ALL COMPONENTS )
	list ( APPEND CPACK_COMPONENTS_ALL "meta" )
endif ()
# set(CPACK_DEBIAN_PACKAGE_DEBUG ON)