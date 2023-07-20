# only cmake since 3.17 supports packaging of debuginfo
cmake_minimum_required ( VERSION 3.17 )

if (NOT installed)
	# start with short route - set all paths
	set ( CPACK_PACKAGING_INSTALL_PREFIX "/" )
	set ( CMAKE_INSTALL_PREFIX "${CPACK_PACKAGING_INSTALL_PREFIX}" CACHE PATH "prefix from distr build" FORCE )
	include ( GNUInstallDirs )
	SET ( FULL_SHARE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/manticore" )
	SET ( LOCALDATADIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/manticore" ) # will be used also in the app
	SET ( CMAKE_INSTALL_FULL_LOCALLIBDIR "${CMAKE_INSTALL_PREFIX}usr/local/lib" )
	set ( installed ON )
	diag ( CPACK_PACKAGING_INSTALL_PREFIX CMAKE_INSTALL_PREFIX )
endif ()

if (only_set_paths)
	return ()
endif ()

# set or append DEBLINE to deb-spec style DEBLIST
function ( seta DEBLIST DEBLINE )
	if (${DEBLIST})
		set ( ${DEBLIST} "${${DEBLIST}}, ${DEBLINE}" PARENT_SCOPE )
	else ()
		set ( ${DEBLIST} "${DEBLINE}" PARENT_SCOPE )
	endif ()
endfunction ()

# Common debian-specific build variables
set ( CPACK_GENERATOR DEB )

# Parse version dependencies from file and assign it to vars
include( builds/VersionDeps )
set ( DEP_BUDDY_VERSION "${BUDDY_VERNUM}-${BUDDY_VERDATE}-${BUDDY_VERHASH}" )
set ( DEP_BACKUP_VERSION "${BACKUP_VERNUM}-${BACKUP_VERDATE}-${BACKUP_VERHASH}" )

set ( CPACK_COMPONENTS_GROUPING IGNORE )
set ( CPACK_DEBIAN_FILE_NAME DEB-DEFAULT )
set ( CPACK_DEBIAN_DEBUGINFO_PACKAGE ON )

# dependencies will be auto calculated. FIXME! M.b. point them directly?
#set ( CPACK_DEBIAN_BIN_PACKAGE_DEPENDS "libc6 (>= 2.15), libexpat (>= 2.0.1), libgcc1 (>= 1:3.0), libstdc++6 (>= 5.2), zlib1g (>= 1:1.1.4), lsb-base (>= 4.1+Debian11ubuntu7)" )

if (NOT disable_shlideps)
	set ( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )
endif ()

set ( breaks "manticore (<= 4.2.1-220503-fffffffff)" )

set ( CPACK_DEBIAN_PACKAGE_SECTION "misc" )
set ( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )

set ( dirserver "${MANTICORE_BINARY_DIR}/config/server" )
set ( dircommon "${MANTICORE_BINARY_DIR}/config/common" )
set ( dircore "${MANTICORE_BINARY_DIR}/config/core" )

set ( CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON )

set ( CPACK_DEBIAN_SEARCHD_PACKAGE_NAME "manticore-server-core" )
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_REPLACES "manticore-bin, sphinxsearch, ${breaks}" )
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_CONTROL_EXTRA "${dircore}/postinst;${dircore}/postrm" )
seta ( CPACK_DEBIAN_SEARCHD_PACKAGE_DEPENDS "manticore-common (= ${CPACK_PACKAGE_VERSION})" )
seta ( CPACK_DEBIAN_SEARCHD_PACKAGE_SUGGESTS "manticore-server (= ${CPACK_PACKAGE_VERSION})" )
set ( CPACK_DEBIAN_SEARCHD_PACKAGE_BREAKS "${breaks}" )

set ( CPACK_DEBIAN_SERVER_PACKAGE_NAME "manticore-server" )
seta ( CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS "manticore-server-core (= ${CPACK_PACKAGE_VERSION})" )
set ( CPACK_DEBIAN_SERVER_PACKAGE_CONTROL_EXTRA "${dirserver}/conffiles;${dirserver}/postinst;${dirserver}/prerm;${dirserver}/postrm" )
set ( CPACK_DEBIAN_SERVER_PACKAGE_DEBUG OFF )
set ( CPACK_DEBIAN_SERVER_PACKAGE_REPLACES "${breaks}" )
set ( CPACK_DEBIAN_SERVER_PACKAGE_BREAKS "${breaks}" )

set ( CPACK_DEBIAN_TOOLS_PACKAGE_NAME "manticore-tools" )
seta ( CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS "manticore-common (= ${CPACK_PACKAGE_VERSION})" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_RECOMMENDS "manticore-backup (>= ${DEP_BACKUP_VERSION} ), manticore-backup(<< ${BACKUP_VERNUM_MAX} )" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_CONFLICTS "sphinxsearch, manticore (<< 3.5.0-200722-1d34c491)" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_REPLACES "${breaks}" )
set ( CPACK_DEBIAN_TOOLS_PACKAGE_BREAKS "${breaks}" )

set ( CPACK_DEBIAN_DEVEL_PACKAGE_NAME "manticore-dev" )
set ( CPACK_DEBIAN_DEVEL_PACKAGE_ARCHITECTURE all )

set ( CPACK_DEBIAN_ICUDATA_PACKAGE_NAME "manticore-icudata-65l" )
set ( CPACK_DEBIAN_ICUDATA_FILE_NAME "manticore-icudata-65l.deb" )
set ( CPACK_DEBIAN_ICUDATA_PACKAGE_ARCHITECTURE all )
set ( CPACK_DEBIAN_ICUDATA_PACKAGE_REPLACES "${breaks}" )
set ( CPACK_DEBIAN_ICUDATA_PACKAGE_BREAKS "${breaks}" )

set ( CPACK_DEBIAN_COMMON_PACKAGE_NAME "manticore-common" )
set ( CPACK_DEBIAN_COMMON_PACKAGE_CONTROL_EXTRA "${dircommon}/conffiles;${dircommon}/postinst;${dircommon}/postrm" )
set ( CPACK_DEBIAN_COMMON_PACKAGE_SUGGESTS "manticore-icudata-65l" )
set ( CPACK_DEBIAN_COMMON_PACKAGE_REPLACES "${breaks}" )
set ( CPACK_DEBIAN_COMMON_PACKAGE_BREAKS "${breaks}" )
set ( CPACK_DEBIAN_COMMON_PACKAGE_ARCHITECTURE all )

set ( CPACK_DEBIAN_META_PACKAGE_NAME "manticore" )
seta ( CPACK_DEBIAN_META_PACKAGE_DEPENDS "manticore-server (= ${CPACK_PACKAGE_VERSION}), manticore-tools (= ${CPACK_PACKAGE_VERSION}), manticore-dev (= ${CPACK_PACKAGE_VERSION}), manticore-icudata-65l, manticore-buddy (>= ${DEP_BUDDY_VERSION}), manticore-buddy (<< ${BUDDY_VERNUM_MAX})" )
#set ( CPACK_DEBIAN_META_PACKAGE_ARCHITECTURE all )
set ( CPACK_DEBIAN_META_PACKAGE_DEBUG OFF )
set ( CPACK_DEBIAN_META_PACKAGE_SECTION metapackages )

set ( CPACK_DEBIAN_CONVERTER_PACKAGE_NAME "manticore-converter" )

# pre-configuration. We're not yet configuring with final paths, but just construct incoming file
# at the beginning assume we have NOT yet set CMAKE_INSTALL_PREFIX, so any result of GNUInstallDirs is not actual

# make template postinst.in
FILE ( READ dist/deb/postinst-server.head POSTINST_HEAD )
FILE ( READ dist/deb/postinst.${flavour}.tail POSTINST_TAIL )
configure_file ( "dist/deb/postinst.in.in" "${dirserver}/postinst.in" )

configure_file ( "dist/deb/postinst-common.in" "${dircommon}/postinst.in" COPYONLY )
configure_file ( "dist/deb/postinst-core.in" "${dircore}/postinst.in" COPYONLY )

# copy template prerm
configure_file ( "dist/deb/prerm.${flavour}.in" "${dirserver}/prerm.in" COPYONLY )

# copy template postrm
configure_file ( "dist/deb/postrm-common.in" "${dircommon}/postrm.in" COPYONLY )
configure_file ( "dist/deb/postrm-core.in" "${dircore}/postrm.in" COPYONLY )
configure_file ( "dist/deb/postrm-server.in" "${dirserver}/postrm.in" COPYONLY )

# manually configure (require known prefix/gnu install dirs)
configure_config ( lib/manticore )

# configure conffiles
configure_file ( "dist/deb/conffiles-common.in" "${dircommon}/conffiles" @ONLY )
configure_file ( "dist/deb/conffiles-server.in" "${dirserver}/conffiles" @ONLY )

configure_file ( "dist/deb/manticore.default.in" "${dirserver}/manticore" @ONLY )
configure_file ( "dist/deb/manticore.init.in" "${dirserver}/manticore.init" @ONLY )
configure_file ( "dist/deb/manticore.service.in" "${dirserver}/manticore.service" @ONLY )

configure_file ( "dist/deb/README.Debian.in" "${MANTICORE_BINARY_DIR}/README.Debian" @ONLY )
configure_file ( "dist/deb/manticore.logrotate.in" "${MANTICORE_BINARY_DIR}/manticore.logrotate" @ONLY )

# configure common scripts
configure_file ( "${dircommon}/postinst.in" "${dircommon}/postinst" @ONLY )
configure_file ( "${dircommon}/postrm.in" "${dircommon}/postrm" @ONLY )

# configure core scripts
configure_file ( "${dircore}/postinst.in" "${dircore}/postinst" @ONLY )
configure_file ( "${dircore}/postrm.in" "${dircore}/postrm" @ONLY )

# configure systemd scripts
configure_file ( "${dirserver}/postinst.in" "${dirserver}/postinst" @ONLY )
configure_file ( "${dirserver}/prerm.in" "${dirserver}/prerm" @ONLY )
configure_file ( "${dirserver}/postrm.in" "${dirserver}/postrm" @ONLY )

# installation

# stuff going to /etc
# CMAKE_INSTALL_SYSCONFDIR					etc 					/etc
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT common )
install ( FILES "${dirserver}/manticore" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/default COMPONENT server )
install ( FILES "${dirserver}/manticore.init" DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/init.d
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ COMPONENT server RENAME manticore )
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.logrotate DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/logrotate.d COMPONENT searchd RENAME manticore )

# stuff going to /var
# CMAKE_INSTALL_LOCALSTATEDIR				var 					/var
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore COMPONENT common )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT searchd )

# stuff that should go to /lib -> actually to /usr/lib
# CMAKE_INSTALL_LIBDIR						usr/lib64 				/usr/lib64
install ( FILES "${dirserver}/manticore.service" DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system COMPONENT server )

# binaries go to /usr/bin (here is only new cluster, rest is in main file, installing targets)
# CMAKE_INSTALL_BINDIR						usr/bin 				/usr/bin
install ( PROGRAMS "dist/deb/manticore_new_cluster" DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT server )

#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

# stuff going to /usr/share/man, /usr/share/doc
# CMAKE_INSTALL_MANDIR						usr/share/man 			/usr/share/man
# CMAKE_INSTALL_DOCDIR						usr/share/doc/manticore /usr/share/doc/manticore
install ( FILES doc/searchd.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT searchd )
install ( FILES doc/indexer.1 doc/indextool.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT tools )
install ( FILES "${MANTICORE_BINARY_DIR}/README.Debian" DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common )
install ( FILES COPYING INSTALL DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common )
install ( FILES example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT tools )

# stuff going to /usr/share/manticore
# CMAKE_INSTALL_DATAROOTDIR					usr/share 				/usr/share
# CMAKE_INSTALL_DATADIR						usr/share 				/usr/share
install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT common )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore/modules COMPONENT common )

# stuff going to /usr/local/
install ( DIRECTORY DESTINATION /usr/local/lib/manticore COMPONENT common )

if (WITH_ICU AND WITH_ICU_FORCE_STATIC)
	install_icudata ( ${CMAKE_INSTALL_DATADIR}/manticore/icu )
endif ()

if (NOT NOAPI)
	install ( DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT searchd )
endif ()

get_cmake_property ( CPACK_COMPONENTS_ALL COMPONENTS )
list ( APPEND CPACK_COMPONENTS_ALL "meta" )

# set(CPACK_DEBIAN_PACKAGE_DEBUG ON)
