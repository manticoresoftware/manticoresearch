# only cmake since 3.17 supports packaging of debuginfo
cmake_minimum_required ( VERSION 3.17 )

set ( CMAKE_INSTALL_FULL_RUNSTATEDIR "/run")
set ( CMAKE_INSTALL_RUNSTATEDIR "run")


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

# Common rpm-specific build variables
set ( CPACK_GENERATOR RPM )

# Parse version dependencies from file and assign it to vars
include( builds/VersionDeps )
set ( DEP_BUDDY_VERSION "${BUDDY_VERNUM}_${BUDDY_VERDATE}.${BUDDY_VERHASH}" )
set ( DEP_BACKUP_VERSION "${BACKUP_VERNUM}_${BACKUP_VERDATE}.${BACKUP_VERHASH}" )

set ( CPACK_COMPONENTS_GROUPING IGNORE )
set ( CPACK_RPM_FILE_NAME RPM-DEFAULT )
if (RELEASE_DIST)
	set ( CPACK_RPM_PACKAGE_RELEASE 1${RELEASE_DIST} )
	set ( MYVER "${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}" )
else ()
	set ( CPACK_RPM_PACKAGE_RELEASE 1 )
	set ( CPACK_RPM_PACKAGE_RELEASE_DIST ON )
	set ( MYVER "${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}%{?dist}" )
endif ()
set ( CPACK_RPM_PACKAGE_GROUP "Applications/Internet" )
set ( CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )

set ( CPACK_RPM_SEARCHD_PACKAGE_NAME "manticore-server-core" )
set ( CPACK_RPM_SEARCHD_PACKAGE_REQUIRES "manticore-common = ${MYVER}" )
set ( CPACK_RPM_SEARCHD_INSTALL_WITH_EXEC ON )
set ( CPACK_RPM_SEARCHD_PACKAGE_OBSOLETES "sphinx" )

set ( CPACK_RPM_SERVER_PACKAGE_NAME "manticore-server" )
set ( CPACK_RPM_SERVER_PACKAGE_REQUIRES "manticore-server-core = ${MYVER}" )
set ( CPACK_RPM_SERVER_PACKAGE_DEBUG OFF )

set ( CPACK_RPM_TOOLS_PACKAGE_NAME "manticore-tools" )
set ( CPACK_RPM_TOOLS_PACKAGE_REQUIRES "manticore-common = ${MYVER}" )
set ( CPACK_RPM_TOOLS_PACKAGE_REQUIRES "manticore-backup >= ${DEP_BACKUP_VERSION}, manticore-backup < ${BACKUP_VERNUM_MAX}" )
set ( CPACK_RPM_TOOLS_PACKAGE_OBSOLETES "sphinx" )

set ( CPACK_RPM_DEVEL_PACKAGE_NAME "manticore-devel" )
set ( CPACK_RPM_DEVEL_PACKAGE_ARCHITECTURE noarch )

set ( CPACK_RPM_ICUDATA_PACKAGE_NAME "manticore-icudata" )
set ( CPACK_RPM_ICUDATA_FILE_NAME "manticore-icudata.rpm" )
set ( CPACK_RPM_ICUDATA_PACKAGE_ARCHITECTURE noarch )

set ( CPACK_RPM_COMMON_PACKAGE_NAME "manticore-common" )
set ( CPACK_RPM_COMMON_PACKAGE_ARCHITECTURE noarch )
set ( CPACK_RPM_COMMON_PACKAGE_CONFLICTS "sphinx" )

set ( CPACK_RPM_META_PACKAGE_NAME "manticore" )
set ( CPACK_RPM_META_PACKAGE_REQUIRES "manticore-server = ${MYVER}, manticore-tools = ${MYVER}, manticore-devel = ${MYVER}, manticore-icudata, manticore-buddy => ${DEP_BUDDY_VERSION}, manticore-buddy < ${BUDDY_VERNUM_MAX}" )
set ( CPACK_RPM_META_PACKAGE_CONFLICTS "sphinx" )
set ( CPACK_RPM_META_PACKAGE_OBSOLETES "manticore-all" )
set ( CPACK_RPM_META_BUILD_SOURCE_DIRS_PREFIX OFF )

set ( CPACK_RPM_CONVERTER_PACKAGE_NAME "manticore-converter" )
set ( CPACK_RPM_CONVERTER_FILE_NAME "RPM-DEFAULT" )

# other vars defined in CommonInfo
set ( CPACK_RPM_SEARCHD_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_SERVER_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_TOOLS_DEBUGINFO_PACKAGE ON )
set ( CPACK_RPM_DEVEL_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_ICUDATA_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_META_DEBUGINFO_PACKAGE OFF )
set ( CPACK_RPM_CONVERTER_DEBUGINFO_PACKAGE ON )

set ( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/;/tmp;/usr/share/man;/var;/var/lib" )
list ( APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/share/man/man1;/var/log" )
list ( APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/lib/systemd;/usr/lib/systemd/system-generators;/usr/lib/tmpfiles.d" )

#set ( CPACK_BUILD_SOURCE_DIRS OFF )
#string ( LENGTH "${CMAKE_SOURCE_DIR}" source_dir_len_ )
#if (source_dir_len_ LESS 75)
#	message ( STATUS "set src prefix to /tmp/m due to too long path" )
#	set ( CPACK_RPM_BUILD_SOURCE_DIRS_PREFIX "/tmp" )
#endif ()

#set ( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "/usr/include" )
set ( CPACK_RPM_COMMON_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/manticoresearch/manticore.conf"
		)
set ( CPACK_RPM_SEARCHD_USER_FILELIST
		"%config(noreplace) %{_sysconfdir}/logrotate.d/manticore"
		)

set ( CPACK_RPM_SPEC_MORE_DEFINE
		"%define _scripts ${MANTICORE_BINARY_DIR}
%define manticore_user manticore
%define manticore_group manticore" )

SET ( CPACK_RPM_PACKAGE_LICENSE "GNU General Public License v. 2 (GPL2)" )

set ( SCR "${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm" ) # a shortcut
set ( dirserver "${MANTICORE_BINARY_DIR}/config/server" )
set ( dircommon "${MANTICORE_BINARY_DIR}/config/common" )
set ( dircore "${MANTICORE_BINARY_DIR}/config/core" )

# server (service)
set ( CPACK_RPM_SERVER_BUILDREQUIRES "systemd-units" )
set ( CPACK_RPM_SERVER_POST_INSTALL_SCRIPT_FILE "${dirserver}/manticore.post" )

# server (core)
set ( CPACK_RPM_SEARCHD_POST_INSTALL_SCRIPT_FILE "${dircore}/manticore.post" )

# common
set ( CPACK_RPM_COMMON_POST_INSTALL_SCRIPT_FILE "${dircommon}/manticore.post" )

# now get system paths. These variables are used in configure substitutions below
set ( CMAKE_INSTALL_LIB "lib" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_LIB CMAKE_INSTALL_LIB LIBDIR )

# block below used to patch the minconf - add a slash at the end of 'binlog_path' section
configure_config ( lib/manticore )

configure_file ( ${SCR}/manticore.logrotate.in "${MANTICORE_BINARY_DIR}/manticore.logrotate" @ONLY )
configure_file ( ${SCR}/manticore-common.post.in "${dircommon}/manticore.post" @ONLY )
configure_file ( ${SCR}/manticore-core.post.in "${dircore}/manticore.post" @ONLY )
configure_file ( ${SCR}/manticore-server.post.in "${dirserver}/manticore.post" @ONLY )
configure_file ( ${SCR}/manticore.tmpfiles.in "${MANTICORE_BINARY_DIR}/manticore.tmpfiles" @ONLY )
configure_file ( ${SCR}/manticore.service.in "${dirserver}/manticore.service" @ONLY )

# installation

# stuff going to /etc
# CMAKE_INSTALL_SYSCONFDIR					etc 					/etc
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticoresearch COMPONENT common )
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.logrotate DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/logrotate.d COMPONENT searchd RENAME manticore )


# stuff going to /var
# CMAKE_INSTALL_LOCALSTATEDIR				var 					/var
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/manticore COMPONENT common )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_RUNSTATEDIR}/manticore COMPONENT server )
install ( DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore COMPONENT searchd )

# stuff that should go to /lib -> actually to /usr/lib
# CMAKE_INSTALL_LIBDIR						usr/lib64 				/usr/lib64
## note! file below is NOT manticore config, that is config of tmpfiles!
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.tmpfiles DESTINATION ${CMAKE_INSTALL_LIB}/tmpfiles.d COMPONENT server RENAME searchd.conf )

# stuff that should go to /lib -> actually to /usr/lib
install ( FILES "${dirserver}/manticore.service" DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system COMPONENT server )

# binaries go to /usr/bin (here is only new cluster, rest is in main file, installing targets)
# CMAKE_INSTALL_BINDIR						usr/bin 				/usr/bin
install ( PROGRAMS "dist/rpm/manticore_new_cluster" DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT server )


#fixup - CMAKE_INSTALL_DOCDIR is share/doc/MANTICORE, fixup to share/doc/manticore
set ( CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/manticore" )
GNUInstallDirs_get_absolute_install_dir ( CMAKE_INSTALL_FULL_DOCDIR CMAKE_INSTALL_DOCDIR DOCDIR )

# stuff going to /usr/share/man, /usr/share/doc
# CMAKE_INSTALL_MANDIR						usr/share/man 			/usr/share/man
# CMAKE_INSTALL_DOCDIR						usr/share/doc/manticore /usr/share/doc/manticore
install ( FILES doc/indexer.1 doc/indextool.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT tools )
install ( FILES doc/searchd.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT searchd )
install ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common RENAME manticore.conf.dist )
install ( FILES COPYING INSTALL DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT common )
install ( FILES example.sql DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT tools )

# stuff going to /usr/share/manticore
# CMAKE_INSTALL_DATAROOTDIR					usr/share 				/usr/share
# CMAKE_INSTALL_DATADIR						usr/share 				/usr/share
install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT common )

# stuff going to /usr/local/
install ( DIRECTORY DESTINATION /usr/local/lib/manticore COMPONENT common )

if (WITH_ICU AND WITH_ICU_FORCE_STATIC)
	install_icudata ( ${FULL_SHARE_DIR}/icu )
endif ()

if (NOT NOAPI)
	install ( DIRECTORY api DESTINATION ${CMAKE_INSTALL_DATADIR}/manticore COMPONENT searchd )
endif ()

get_cmake_property ( CPACK_COMPONENTS_ALL COMPONENTS )
list ( APPEND CPACK_COMPONENTS_ALL "meta" )

# uncomment this line to produce long (really long) verbose output of rpm building
#set ( CPACK_RPM_PACKAGE_DEBUG ON )
